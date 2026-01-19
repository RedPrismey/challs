# to create an ascii table where table[char_ascii_code] = b64_value_of_char

charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
table = [0xFF for _ in range(0x100)]

for i in range(len(charset)):
    ascii_code = ord(charset[i])
    table[ascii_code] = i

table[ord("=")] = 64

for i in table:
    print(hex(i), end=",")
