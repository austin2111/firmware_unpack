#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("USAGE: %s infile outfile offset\n", argv[0]);
        return 0;
    }
    unsigned short start = strtol(argv[3], NULL, 10);
    if (start > 5000) {
        printf("No.\n");
        return -1;
    }
    unsigned char input_buf[65536];
    FILE *input_file = fopen(argv[1], "rb");
    if (input_file == NULL) {
        perror("Failed to open input file");
        return -1;
    }

    FILE *output_file = fopen(argv[2], "wb");
    if (output_file == NULL) {
        perror("Failed to open output file");
        fclose(input_file);
        return -1;
    }

    if (fseek(input_file, start, SEEK_SET) != 0) {
        perror("fseek failed to skip offset");
        fclose(input_file);
        fclose(output_file);
        return -1;
    }

    while(true) {


        size_t bytes_read = fread(input_buf, 1, 65536, input_file);
        if (bytes_read == 0 && !feof(input_file)) {
            perror("Error reading input file");
            fclose(input_file);
            fclose(output_file);
            return -1;
        }
        else if (bytes_read != 65536) {
            printf("DEBUG: Likely reaching end of file\n");
            fwrite(input_buf, 1, bytes_read, output_file);
            break;
        }

        fwrite(input_buf, 1, 65536, output_file);

        if (fseek(input_file, 65536, SEEK_CUR) != 0) {
            perror("fseek failed to skip offset");
            fclose(input_file);
            fclose(output_file);
            return -1;
        }

    }

    return 0;
}
