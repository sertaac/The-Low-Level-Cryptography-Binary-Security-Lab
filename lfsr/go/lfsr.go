package main

import (
	"fmt"
)

/* ═══════════════════════════════════════════════════════════════════════════
 * LFSR Core Structure
 * ═══════════════════════════════════════════════════════════════════════════ */

type LFSR struct {
	state uint16 // Current register state
	taps  uint16 // Feedback tap mask (polynomial)
}

/* ═══════════════════════════════════════════════════════════════════════════
 * LFSR Operations
 * ═══════════════════════════════════════════════════════════════════════════ */

// Initialize LFSR with seed and tap configuration
func lfsrInit(seed, taps uint16) LFSR {
	if seed == 0 {
		seed = 1
	}
	return LFSR{state: seed, taps: taps}
}

// Count set bits (parity check for XOR feedback)
func popcount(x uint16) uint8 {
	var count uint8 = 0
	for x != 0 {
		count += uint8(x & 1)
		x >>= 1
	}
	return count
}

// Clock the LFSR once, return output bit
func (reg *LFSR) step() uint8 {
	feedback := popcount(reg.state&reg.taps) & 1            // XOR of tapped bits
	out := uint8(reg.state & 1)                             // Output: LSB
	reg.state = (reg.state >> 1) | (uint16(feedback) << 15) // Shift right, inject at MSB
	return out
}

// Generate 8-bit keystream byte
func (reg *LFSR) getByte() uint8 {
	var b uint8 = 0
	for i := 0; i < 8; i++ {
		b |= reg.step() << i
	}
	return b
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Encryption / Decryption (XOR-based stream cipher)
 * ═══════════════════════════════════════════════════════════════════════════ */

func (reg *LFSR) crypt(data []byte) {
	for i := range data {
		data[i] ^= reg.getByte() // XOR data with keystream
	}
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Utility: Print bytes as hex
 * ═══════════════════════════════════════════════════════════════════════════ */

func printHex(label string, data []byte) {
	fmt.Printf("%s: ", label)
	for _, b := range data {
		fmt.Printf("%02X ", b)
	}
	fmt.Println()
}

func printASCII(label string, data []byte) {
	fmt.Printf("%s: %s\n", label, string(data))
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Demo
 * ═══════════════════════════════════════════════════════════════════════════ */

func main() {
	// Polynomial: x^16 + x^14 + x^13 + x^11 + 1  →  Taps at bits 15,13,12,10
	const TAPS uint16 = 0xB400
	const SEED uint16 = 0xACE1

	// Sample plaintext
	plaintext := []byte("HELLO_LFSR!")

	fmt.Println("=== LFSR Stream Cipher Demo ===")
	fmt.Println()
	printASCII("Plaintext ", plaintext)
	printHex("Plaintext ", plaintext)

	// Encrypt
	enc := lfsrInit(SEED, TAPS)
	enc.crypt(plaintext)
	printHex("Ciphertext", plaintext)

	// Decrypt (same seed resets the keystream)
	dec := lfsrInit(SEED, TAPS)
	dec.crypt(plaintext)
	printHex("Decrypted ", plaintext)
	printASCII("Decrypted ", plaintext)
}
