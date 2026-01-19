g++ challenge.cpp -o challenge -lcrypto -s -Os -Wall -Wextra -Wshadow -fvisibility=hidden
strip -R .comment -R .note.gnu.build-id -R .note.gnu.property -R .note.ABI-tag -R gnu.version challenge # remove a lot of useless sections
printf '\x02' | dd conv=notrunc of=./challenge bs=1 seek=5                                              # change endianness
./strip_bin challenge                                                                                   # 0 out section parts of the header
