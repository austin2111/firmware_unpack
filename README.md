# firmware_unpack
This is a reverse engineering tool created to be used in conjunction with firmware for a specific embedded device. As responsible disclosure rules for findings with the device don't allow it to be unveiled quite yet, all references to the device itself have been removed.

Firmware for newer devices is decompressed in two stages. While older devices will simply load the firmware into RAM directly, newer devices will extract a bootloader and an LZMA compressed archive to be decompressed with it. This data can be decompressed directly with reverse engineering utilities such as binwalk. However, the compressed data is interleaved with a large number (65536) of null bytes every 65536 bytes. When given the start offset of the LZMA data (after the bootloader), seek_remover.c will eliminate this blank space and output a standard LZMA archive. This appears to be an intentional decision by the designers.

minilzo's source files must be included for the program to compile. For example:

gcc firmware_unpack.c minilzo/minilzo.c minilzo/*.h -o firmware_unpack
