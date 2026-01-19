user = "Administrator"
flag = "N0_d3c0mp1l3r"

table = []

for i in range(len(user)):
    table.append(ord(flag[i]) - ord(user[i]))

print(table)  # 13, -52, -14, -5, -59, -6, -67, -7, -2, -48, -8, -60, 0

for i in range(len(user)):
    print(chr(ord(user[i]) + table[i]), end="")

print()

for i in table:
    print(i, end=", ")
