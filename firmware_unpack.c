#include <stdio.h>
#include <stdlib.h>
#include "minilzo.h"
#include <arpa/inet.h>

#define INPUT_CHUNK_SIZE 68500

int decompress_file(const char *input_filename, const char *output_filename) {
    FILE *input_file = fopen(input_filename, "rb");
    if (input_file == NULL) {
        perror("Failed to open input file");
        return -1;
    }

    FILE *output_file = fopen(output_filename, "wb");
    if (output_file == NULL) {
        perror("Failed to open output file");
        fclose(input_file);
        return -1;
    }

    if (fseek(input_file, 154, SEEK_SET) != 0) {
        perror("fseek failed to skip header");
        fclose(input_file);
        return -1;
    }

    lzo_bytep wrkmem = NULL;
    lzo_uint wrklen = LZO1X_1_MEM_COMPRESS;

    int r;
    unsigned char input_buf[INPUT_CHUNK_SIZE]; // TO DO: Make the buffer size something more in tune with the maximum compressed value
    unsigned char *output_buf = NULL;
    unsigned long readsize, writesize;
    char statbuf[4];
    size_t total_output_size = 0;

    // Allocate work memory
    wrkmem = (lzo_bytep)malloc(wrklen);
    if (wrkmem == NULL) {
        perror("Memory allocation failed");
        fclose(input_file);
        fclose(output_file);
        return -1;
    }

    if (lzo_init() != LZO_E_OK) {
        perror("LZO initialization failed");
        free(wrkmem);
        fclose(input_file);
        fclose(output_file);
        return -1;
    }

    // Read and decompress the file in chunks
    while (!feof(input_file)) {
        printf("DEBUG: File pointer is currently at %ld\n", ftell(input_file));
        size_t bytes_read = fread(statbuf, 1, 4, input_file);
        if (bytes_read == 0 && !feof(input_file)) {
            perror("Error reading input file");
            break;
        }
        writesize = *(unsigned long *) statbuf; // The number is technically big endian, so this won't work >.<
        writesize = ntohl(writesize); // Endianness conversion
        printf("DEBUG: writesize is 0x%08lx\n", writesize);
        if (writesize == 0) {
            printf("Write size of 0 encountered! Exiting...\n");
            break;
        }
        // We're mallocing based on some random bytes in a binary.
        // It's probably a good idea to clip that if it goes above a certain size.
        // I reaaaaally don't want to make a 4 GB call to malloc.
        if (writesize > 274000) {
            printf("WARNING: Write size greater than 274000 bytes! Clipping.\n");
            writesize = 274000;
        }
        // Does a maximum of a 4:1 compression ratio seem okay? LZO is notorious for a weak
        // compression ratio, and this seems to work fine. Change this if it's ever actually
        // a problem.

        bytes_read = fread(statbuf, 1, 4, input_file);
        if (bytes_read == 0 && !feof(input_file)) {
            perror("Error reading input file");
            break;
        }

        readsize = *(unsigned long *) statbuf;
        readsize = ntohl(readsize);
        printf("DEBUG: Readsize is 0x%08lx\n", readsize);
        if (readsize > 68500) {
            printf("WARNING: Read size greater than 68500 bytes! Clipping.\n");
            readsize = 68500;
        }

        bytes_read = fread(input_buf, 1, readsize, input_file);
        if (bytes_read == 0 && !feof(input_file)) {
            perror("Error reading input file");
            break;
        }
        printf("DEBUG: bytes_read is %lu\n", bytes_read);

        // Allocate output buffer based on expected decompressed size
        size_t max_output_size = writesize;
        output_buf = (unsigned char *)malloc(max_output_size);
        if (output_buf == NULL) {
            perror("Memory allocation failed");
            break;
        }
        // This was using the unsafe function once upon a time.
        // The problem with that is if it tries to decompress an invalid
        // chunk, it segfaults. This is... not ideal.
        r = lzo1x_decompress_safe(input_buf, (lzo_uint)bytes_read,
        //r = lzo1x_decompress_safe(input_buf, (lzo_uint)bytes_read,
                             output_buf, (lzo_uint *)&max_output_size, wrkmem);
        printf("DEBUG: Result code is %d, out length is %lu\n", r, max_output_size);

        if ((r != LZO_E_OK) && (r != LZO_E_INPUT_NOT_CONSUMED)) {
            perror("Decompression failed");
            free(output_buf);
            break;
        }

        // Write decompressed data to output file
        fwrite(output_buf, 1, max_output_size, output_file);
        total_output_size += max_output_size;

        // Free the output buffer for next iteration
        free(output_buf);
        output_buf = NULL;
    }

    free(wrkmem);
    fclose(input_file);
    fclose(output_file);

    printf("Decompressed file '%s' to '%s' (%zu bytes)\n", input_filename, output_filename, total_output_size);

    return 0;
}

int main(int argc, char** argv) {
    //const char *input_filename = "compressed_file.lzo";
    //const char *output_filename = "decompressed_file.txt";
    if (argc != 3) {
        printf("USAGE: %s <input filename> <output filename>\n", argv[0]);
        return -1;
    }

    if (decompress_file(argv[1], argv[2]) == 0) {
        printf("Decompression successful.\n");
    } else {
        fprintf(stderr, "Decompression failed.\n");
    }

    return 0;
}
