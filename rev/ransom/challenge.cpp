#include <chrono>
#include <fstream>
#include <iostream>
#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/param_build.h>
#include <openssl/rsa.h>
#include <vector>

void ransom(const std::string &filename, EVP_PKEY *pkey);
bool encrypt(EVP_PKEY *pkey, const unsigned char *plain, size_t plain_len,
             unsigned char **cipher, size_t *cipher_len);
BIGNUM *generate_random_prime(int bits);
bool generate_keys(BIGNUM **n, BIGNUM **e, BIGNUM **d);
EVP_PKEY *create_rsa_pkey(BIGNUM *n, BIGNUM *e, BIGNUM *d);
std::vector<unsigned char> readFile(const std::string &filename);
bool writeFile(const std::string &filename,
               const std::vector<unsigned char> &data);

int main() {
  BIGNUM *n = nullptr, *e = nullptr, *d = nullptr;
  EVP_PKEY *pkey = nullptr;

  std::vector<unsigned char> fileContent = readFile("flag.txt");
  if (fileContent.empty())
    return 0;

  generate_keys(&n, &e, &d);

  pkey = create_rsa_pkey(n, e, d);

  ransom("flag.txt", pkey);

  return 0;
}

void ransom(const std::string &filename, EVP_PKEY *pkey) {
  std::vector<unsigned char> fileContent = readFile(filename);
  if (fileContent.empty())
    return;

  unsigned char *cipher = nullptr;
  size_t cipher_len = 0;
  if (!encrypt(pkey, fileContent.data(), fileContent.size(), &cipher,
               &cipher_len))
    return;

  std::vector<unsigned char> cipherVec(cipher, cipher + cipher_len);
  if (!writeFile(filename, cipherVec)) {
    OPENSSL_free(cipher);
    return;
  }

  OPENSSL_free(cipher);
}

bool encrypt(EVP_PKEY *pkey, const unsigned char *plain, size_t plain_len,
             unsigned char **cipher, size_t *cipher_len) {
  bool ret = false;
  EVP_PKEY_CTX *ctx = nullptr;
  ctx = EVP_PKEY_CTX_new(pkey, NULL);
  if (!ctx)
    goto cleanup;

  if (EVP_PKEY_encrypt_init(ctx) <= 0)
    goto cleanup;
  if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
    goto cleanup;

  if (EVP_PKEY_encrypt(ctx, NULL, cipher_len, plain, plain_len) <= 0)
    goto cleanup;

  *cipher = (unsigned char *)OPENSSL_malloc(*cipher_len);
  if (!*cipher)
    goto cleanup;

  if (EVP_PKEY_encrypt(ctx, *cipher, cipher_len, plain, plain_len) <= 0) {
    OPENSSL_free(*cipher);
    *cipher = nullptr;
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

  *n = BN_new();
  *e = BN_new();
  *d = BN_new();

  auto now = std::chrono::system_clock::now();
  auto now_seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
  auto epoch = now_seconds.time_since_epoch();
  auto minutes = std::chrono::duration_cast<std::chrono::minutes>(epoch);

  srand(static_cast<int>(minutes.count()));

  ctx = BN_CTX_new();
  if (!ctx)
    goto cleanup;

  BN_set_word(*e, 65537);
  p = generate_random_prime(128);
  q = generate_random_prime(128);
  if (!p || !q)
    goto cleanup;

  /*---[n = p * q]---*/
  BN_mul(*n, p, q, ctx);

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
      unsigned char a = static_cast<unsigned char>(rand() % 256);
      buffer[i] = a;
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

std::vector<unsigned char> readFile(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file)
    return {};
  return std::vector<unsigned char>((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
}

bool writeFile(const std::string &filename,
               const std::vector<unsigned char> &data) {
  std::ofstream file(filename, std::ios::binary | std::ios::trunc);
  if (!file)
    return false;
  file.write(reinterpret_cast<const char *>(data.data()), data.size());
  return file.good();
}
