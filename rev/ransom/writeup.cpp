#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/param_build.h>
#include <openssl/rsa.h>
#include <vector>

void unransom(std::string filename, EVP_PKEY *pkey);
bool decrypt(EVP_PKEY *pkey, const unsigned char *cipher, size_t cipher_len,
             unsigned char **plain, size_t *plain_len);
BIGNUM *generate_random_prime(int bits);
bool generate_keys(BIGNUM **n, BIGNUM **e, BIGNUM **d);
EVP_PKEY *create_rsa_pkey(BIGNUM *n, BIGNUM *e, BIGNUM *d);
void print_bignum(BIGNUM *bn);
std::vector<unsigned char> read_file(const std::string &filename);

int main() {
  BIGNUM *n = nullptr, *e = nullptr, *d = nullptr;
  EVP_PKEY *pkey = nullptr;
  unsigned char *cipher = nullptr, *plain_out = nullptr;

  generate_keys(&n, &e, &d);

  pkey = create_rsa_pkey(n, e, d);

  unransom("flag.txt", pkey);

  return 0;
}

void unransom(std::string filename, EVP_PKEY *pkey) {
  std::vector<unsigned char> fileCiphertext = read_file(filename);
  if (fileCiphertext.empty())
    return;

  unsigned char *decrypted = nullptr;
  size_t decrypted_len = 0;
  if (!decrypt(pkey, fileCiphertext.data(), fileCiphertext.size(), &decrypted,
               &decrypted_len))
    return;

  std::string decryptedStr(reinterpret_cast<char *>(decrypted), decrypted_len);

  std::cout << "Decrypted content: " << decryptedStr << std::endl;

  OPENSSL_free(decrypted);
}

bool decrypt(EVP_PKEY *pkey, const unsigned char *cipher, size_t cipher_len,
             unsigned char **plain, size_t *plain_len) {
  bool ret = false;
  EVP_PKEY_CTX *ctx = nullptr;
  ctx = EVP_PKEY_CTX_new(pkey, NULL);
  if (!ctx)
    goto cleanup;

  if (EVP_PKEY_decrypt_init(ctx) <= 0)
    goto cleanup;
  if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
    goto cleanup;

  if (EVP_PKEY_decrypt(ctx, NULL, plain_len, cipher, cipher_len) <= 0)
    goto cleanup;

  *plain = (unsigned char *)OPENSSL_malloc(*plain_len);
  if (!*plain)
    goto cleanup;

  if (EVP_PKEY_decrypt(ctx, *plain, plain_len, cipher, cipher_len) <= 0) {
    OPENSSL_free(*plain);
    *plain = nullptr;
    goto cleanup;
  }

  ret = true;
cleanup:
  if (ctx)
    EVP_PKEY_CTX_free(ctx);
  return ret;
}

bool generate_keys(BIGNUM **n, BIGNUM **e, BIGNUM **d) {
  bool ret = false;
  BIGNUM *p = nullptr, *q = nullptr, *r = nullptr;
  BN_CTX *ctx = nullptr;
  int srand_seed = 29461696;

  auto time_point_from_minutes =
      std::chrono::system_clock::from_time_t(srand_seed * 60);
  std::time_t time_t_from_point =
      std::chrono::system_clock::to_time_t(time_point_from_minutes);
  std::tm *tm_from_time = std::localtime(&time_t_from_point);
  std::cout << "date d'execution : "
            << std::put_time(tm_from_time, "%Y-%m-%d %H:%M:%S") << std::endl;

  std::cout << "srand_seed : " << srand_seed << std::endl;

  *n = BN_new();
  *e = BN_new();
  *d = BN_new();

  srand(srand_seed);

  ctx = BN_CTX_new();
  if (!ctx)
    goto cleanup;

  BN_set_word(*e, 65537);
  p = generate_random_prime(128);
  q = generate_random_prime(128);
  if (!p || !q)
    goto cleanup;
  std::cout << "q: ";
  print_bignum(q);

  /*---[n = p * q]---*/
  BN_mul(*n, p, q, ctx);
  std::cout << "n: ";
  print_bignum(*n);

  /*---[Euler totient (r)]---*/
  r = BN_new();
  if (!r)
    goto cleanup;

  BN_sub_word(p, 1);
  BN_sub_word(q, 1);
  BN_mul(r, p, q, ctx);

  BN_mod_inverse(*d, *e, r, ctx);

  ret = true;

  /*---[free]---*/
cleanup:
  if (ctx)
    BN_CTX_free(ctx);
  if (r)
    BN_free(r);
  if (p)
    BN_free(p);
  if (q)
    BN_free(q);

  return ret;
}

BIGNUM *generate_random_prime(int bytes) {
  bool is_prime = false;

  BIGNUM *candidate = BN_new();
  if (candidate == nullptr) {
    return nullptr;
  }

  BN_CTX *ctx = BN_CTX_new();
  if (ctx == nullptr) {
    return nullptr;
  }

  unsigned char *buffer = new unsigned char[bytes];

  while (!is_prime) {
    for (int i = 0; i < bytes; ++i) {
      buffer[i] = static_cast<unsigned char>(rand() % 256);
    }

    BN_bin2bn(buffer, bytes, candidate);

    BN_set_bit(candidate, 0);
    BN_set_bit(candidate, 1023);

    is_prime = BN_check_prime(candidate, ctx, nullptr);
  }

  delete[] buffer;
  return candidate;
}

EVP_PKEY *create_rsa_pkey(BIGNUM *n, BIGNUM *e, BIGNUM *d) {
  EVP_PKEY *pkey = nullptr;
  EVP_PKEY_CTX *pctx = nullptr;
  OSSL_PARAM *params = nullptr;
  OSSL_PARAM_BLD *bld = nullptr;

  bld = OSSL_PARAM_BLD_new();
  if (!bld)
    goto cleanup;
  if (!OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_N, n) ||
      !OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_E, e) ||
      !OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_D, d))
    goto cleanup;

  params = OSSL_PARAM_BLD_to_param(bld);
  if (!params)
    goto cleanup;

  pctx = EVP_PKEY_CTX_new_from_name(NULL, "RSA", NULL);
  if (!pctx)
    goto cleanup;

  if (EVP_PKEY_fromdata_init(pctx) <= 0)
    goto cleanup;

  EVP_PKEY_fromdata(pctx, &pkey, EVP_PKEY_KEYPAIR, params);

cleanup:
  if (bld)
    OSSL_PARAM_BLD_free(bld);
  if (params)
    OPENSSL_free(params);
  if (pctx)
    EVP_PKEY_CTX_free(pctx);
  return pkey;
}

std::vector<unsigned char> read_file(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file)
    return {};
  return std::vector<unsigned char>((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
}

void print_bignum(BIGNUM *bn) {
  char *hex_str = BN_bn2hex(bn);
  std::cout << hex_str << std::endl;
  OPENSSL_free(hex_str);
}
