rm -f ./*.elf32
xenon-objcopy -O elf32-powerpc --adjust-vma 0x80000000 ./rr ./rrootage.elf32
xenon-strip ./rrootage.elf32
cp *.elf32 /srv/tftp
