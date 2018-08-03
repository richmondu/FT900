ft32-elf-objcopy -I binary -O elf32-ft32 -B ft32 --rename-section .data=.text ../Certificates/cert.crt cert.crt.o
ft32-elf-objcopy -I binary -O elf32-ft32 -B ft32 --rename-section .data=.text ../Certificates/cert.key cert.key.o
ft32-elf-objcopy -I binary -O elf32-ft32 -B ft32 --rename-section .data=.text ../Certificates/ca.crt ca.crt.o
ft32-elf-objcopy -I binary -O elf32-ft32 -B ft32 --rename-section .data=.text ../Certificates/ggca.crt ggca.crt.o