ft32-elf-objcopy -I binary -O elf32-ft32 -B ft32 --rename-section .data=.text ../Certificates/ft900device1_cert.pem ft900device1_cert.pem.o
ft32-elf-objcopy -I binary -O elf32-ft32 -B ft32 --rename-section .data=.text ../Certificates/ft900device1_pkey.pem ft900device1_pkey.pem.o
ft32-elf-objcopy -I binary -O elf32-ft32 -B ft32 --rename-section .data=.text ../Certificates/rootca.pem rootca.pem.o