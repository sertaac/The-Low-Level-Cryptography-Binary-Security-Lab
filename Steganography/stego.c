/*
 * LSB Steganography Tool
 * Hide and reveal text in 24-bit BMP image files
 * Usage: ./stego hide input.bmp "message" | ./stego reveal output.bmp
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BMP_HEADER_SIZE 54
#define OUTPUT_FILE "output.bmp"

/* ==================== BMP Header Structure ==================== */
typedef struct {
    uint8_t header[BMP_HEADER_SIZE];
    uint32_t file_size;
    uint32_t data_offset;
    uint32_t width;
    uint32_t height;
    uint16_t bits_per_pixel;
} BMPInfo;

/* ==================== Helper Functions ==================== */

// Read 4 bytes in little-endian
uint32_t read_le32(uint8_t *buf) {
    return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

// Read 2 bytes in little-endian
uint16_t read_le16(uint8_t *buf) {
    return buf[0] | (buf[1] << 8);
}

// Read and validate BMP header
int read_bmp_header(FILE *fp, BMPInfo *info) {
    if (fread(info->header, 1, BMP_HEADER_SIZE, fp) != BMP_HEADER_SIZE)
        return -1;
    
    // BMP signature check: 'B' and 'M'
    if (info->header[0] != 'B' || info->header[1] != 'M')
        return -1;
    
    info->file_size = read_le32(&info->header[2]);
    info->data_offset = read_le32(&info->header[10]);
    info->width = read_le32(&info->header[18]);
    info->height = read_le32(&info->header[22]);
    info->bits_per_pixel = read_le16(&info->header[28]);
    
    // Only 24-bit BMP supported
    if (info->bits_per_pixel != 24) {
        fprintf(stderr, "Error: Only 24-bit BMP supported.\n");
        return -1;
    }
    
    return 0;
}

/* ==================== LSB Bit Manipulation ==================== */

/*
 * Embed a bit into the LSB of a byte.
 * 1. (byte & 0xFE) -> Clears LSB. 0xFE = 11111110, AND masks out LSB.
 * 2. (... | bit)   -> Sets the new bit value. OR inserts the bit.
 */
uint8_t embed_bit(uint8_t byte, uint8_t bit) {
    return (byte & 0xFE) | (bit & 0x01);
}

/*
 * Extract the LSB from a byte.
 * (byte & 0x01) -> Isolates only the lowest bit, masking others.
 */
uint8_t extract_bit(uint8_t byte) {
    return byte & 0x01;
}

/*
 * Get a specific bit from a character at given position.
 * (ch >> pos) -> Shifts character right.
 * (... & 0x01) -> Isolates the lowest bit.
 * Reads from MSB to LSB (7 to 0).
 */
uint8_t get_bit_at(char ch, int pos) {
    return (ch >> (7 - pos)) & 0x01;
}

/* ==================== Hide Function ==================== */

int hide_message(const char *input_file, const char *message) {
    FILE *fin = fopen(input_file, "rb");
    if (!fin) {
        fprintf(stderr, "Error: Cannot open %s.\n", input_file);
        return -1;
    }
    
    BMPInfo info;
    if (read_bmp_header(fin, &info) != 0) {
        fprintf(stderr, "Error: Invalid BMP file.\n");
        fclose(fin);
        return -1;
    }
    
    // Check message size (including NULL, each char needs 8 bytes)
    size_t msg_len = strlen(message) + 1;  // +1 for NULL terminator
    size_t required_bytes = msg_len * 8;
    size_t available_bytes = info.file_size - BMP_HEADER_SIZE;
    
    if (required_bytes > available_bytes) {
        fprintf(stderr, "Error: Message too long. Max %zu chars.\n", available_bytes / 8 - 1);
        fclose(fin);
        return -1;
    }
    
    // Create output file
    FILE *fout = fopen(OUTPUT_FILE, "wb");
    if (!fout) {
        fprintf(stderr, "Error: Cannot create %s.\n", OUTPUT_FILE);
        fclose(fin);
        return -1;
    }
    
    // Copy header as-is
    fwrite(info.header, 1, BMP_HEADER_SIZE, fout);
    
    // Process pixel data and embed message
    size_t char_idx = 0;   // Current character index in message
    int bit_idx = 0;       // Current bit index in character (0-7)
    uint8_t pixel_byte;
    
    while (fread(&pixel_byte, 1, 1, fin) == 1) {
        if (char_idx < msg_len) {
            // Get bit from message and embed into pixel byte
            uint8_t msg_bit = get_bit_at(message[char_idx], bit_idx);
            pixel_byte = embed_bit(pixel_byte, msg_bit);
            
            bit_idx++;
            if (bit_idx == 8) {
                bit_idx = 0;
                char_idx++;
            }
        }
        fwrite(&pixel_byte, 1, 1, fout);
    }
    
    fclose(fin);
    fclose(fout);
    
    printf("Message hidden successfully: %s\n", OUTPUT_FILE);
    printf("Hidden: \"%s\" (%zu chars)\n", message, msg_len - 1);
    
    return 0;
}

/* ==================== Reveal Function ==================== */

int reveal_message(const char *input_file) {
    FILE *fin = fopen(input_file, "rb");
    if (!fin) {
        fprintf(stderr, "Error: Cannot open %s.\n", input_file);
        return -1;
    }
    
    BMPInfo info;
    if (read_bmp_header(fin, &info) != 0) {
        fprintf(stderr, "Error: Invalid BMP file.\n");
        fclose(fin);
        return -1;
    }
    
    // Buffer for maximum possible message size
    size_t max_chars = (info.file_size - BMP_HEADER_SIZE) / 8;
    char *message = malloc(max_chars + 1);
    if (!message) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        fclose(fin);
        return -1;
    }
    
    size_t char_idx = 0;
    int bit_idx = 0;
    char current_char = 0;
    uint8_t pixel_byte;
    
    while (fread(&pixel_byte, 1, 1, fin) == 1 && char_idx < max_chars) {
        // Extract LSB and add to character
        uint8_t bit = extract_bit(pixel_byte);
        
        /*
         * Bit placement:
         * current_char |= (bit << (7 - bit_idx))
         * Shifts bit to correct position and ORs it in.
         * Builds character from MSB to LSB.
         */
        current_char |= (bit << (7 - bit_idx));
        
        bit_idx++;
        if (bit_idx == 8) {
            message[char_idx] = current_char;
            
            // NULL terminator found, message complete
            if (current_char == '\0')
                break;
            
            current_char = 0;
            bit_idx = 0;
            char_idx++;
        }
    }
    
    message[char_idx] = '\0';  // Safety null-terminate
    
    printf("Hidden message: \"%s\"\n", message);
    
    free(message);
    fclose(fin);
    
    return 0;
}

/* ==================== Usage Info ==================== */

void print_usage(const char *prog_name) {
    printf("LSB Steganography Tool\n");
    printf("Usage:\n");
    printf("  %s hide <input.bmp> \"message\"  - Hide message in BMP\n", prog_name);
    printf("  %s reveal <input.bmp>          - Reveal hidden message\n", prog_name);
}

/* ==================== Main Program ==================== */

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char *command = argv[1];
    const char *bmp_file = argv[2];
    
    if (strcmp(command, "hide") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: Message not specified.\n");
            print_usage(argv[0]);
            return 1;
        }
        return hide_message(bmp_file, argv[3]);
    }
    else if (strcmp(command, "reveal") == 0) {
        return reveal_message(bmp_file);
    }
    else {
        fprintf(stderr, "Error: Invalid command '%s'\n", command);
        print_usage(argv[0]);
        return 1;
    }
}
