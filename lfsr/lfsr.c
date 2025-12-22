#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * LFSR Core Structure
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    uint16_t state;     // Current register state
    uint16_t taps;      // Feedback tap mask (polynomial)
} LFSR;

/* ═══════════════════════════════════════════════════════════════════════════
 * LFSR Operations
 * ═══════════════════════════════════════════════════════════════════════════ */

// Initialize LFSR with seed and tap configuration
LFSR lfsr_init(uint16_t seed, uint16_t taps) {
    return (LFSR){ .state = seed ? seed : 1, .taps = taps };
}

// Count set bits (parity check for XOR feedback)
static inline uint8_t popcount(uint16_t x) {
    uint8_t count = 0;
    while (x) { count += x & 1; x >>= 1; }
    return count;
}

// Clock the LFSR once, return output bit
uint8_t lfsr_step(LFSR *reg) {
    uint8_t feedback = popcount(reg->state & reg->taps) & 1;  // XOR of tapped bits
    uint8_t out = reg->state & 1;                              // Output: LSB
    reg->state = (reg->state >> 1) | (feedback << 15);         // Shift right, inject at MSB
    return out;
}

// Generate n-bit keystream
uint8_t lfsr_get_byte(LFSR *reg) {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++)
        byte |= (lfsr_step(reg) << i);
    return byte;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Encryption / Decryption (XOR-based stream cipher)
 * ═══════════════════════════════════════════════════════════════════════════ */

void lfsr_crypt(LFSR *reg, uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++)
        data[i] ^= lfsr_get_byte(reg);  // XOR data with keystream
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Utility: Print bytes as hex
 * ═══════════════════════════════════════════════════════════════════════════ */

void print_hex(const char *label, const uint8_t *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) printf("%02X ", data[i]);
    printf("\n");
}

void print_ascii(const char *label, const uint8_t *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) printf("%c", data[i]);
    printf("\n");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Demo
 * ═══════════════════════════════════════════════════════════════════════════ */

int main(void) {
    // Polynomial: x^16 + x^14 + x^13 + x^11 + 1  →  Taps at bits 15,13,12,10
    const uint16_t TAPS = 0xB400;
    const uint16_t SEED = 0xACE1;

    // Sample plaintext
    uint8_t plaintext[] = "HELLO_LFSR!";
    size_t len = strlen((char *)plaintext);

    printf("=== LFSR Stream Cipher Demo ===\n\n");
    print_ascii("Plaintext ", plaintext, len);
    print_hex("Plaintext ", plaintext, len);

    // Encrypt
    LFSR enc = lfsr_init(SEED, TAPS);
    lfsr_crypt(&enc, plaintext, len);
    print_hex("Ciphertext", plaintext, len);

    // Decrypt (same seed resets the keystream)
    LFSR dec = lfsr_init(SEED, TAPS);
    lfsr_crypt(&dec, plaintext, len);
    print_hex("Decrypted ", plaintext, len);
    print_ascii("Decrypted ", plaintext, len);

    return 0;
}