expected = "6w\tw)\x06w6\x1b\x1ct6\x1bu7\x1b*ts\x1bw*\a\x16=4sut\n"

out = ""

for i in expected:
    out += chr(ord(i) ^ 0x44)

print(out)
