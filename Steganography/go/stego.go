/*
 * LSB Steganography Tool
 * Hide and reveal text in 24-bit BMP image files
 * Usage: go run stego.go hide input.bmp "message" | go run stego.go reveal output.bmp
 */

package main

import (
	"encoding/binary"
	"fmt"
	"os"
)

const (
	BMPHeaderSize = 54
	OutputFile    = "output.bmp"
)

// ==================== BMP Header Structure ====================

type BMPInfo struct {
	Header       [BMPHeaderSize]byte
	FileSize     uint32
	DataOffset   uint32
	Width        uint32
	Height       uint32
	BitsPerPixel uint16
}

// ==================== Helper Functions ====================

// Read and validate BMP header
func readBMPHeader(file *os.File) (*BMPInfo, error) {
	info := &BMPInfo{}

	if _, err := file.Read(info.Header[:]); err != nil {
		return nil, fmt.Errorf("failed to read header")
	}

	// BMP signature check: 'B' and 'M'
	if info.Header[0] != 'B' || info.Header[1] != 'M' {
		return nil, fmt.Errorf("invalid BMP signature")
	}

	info.FileSize = binary.LittleEndian.Uint32(info.Header[2:6])
	info.DataOffset = binary.LittleEndian.Uint32(info.Header[10:14])
	info.Width = binary.LittleEndian.Uint32(info.Header[18:22])
	info.Height = binary.LittleEndian.Uint32(info.Header[22:26])
	info.BitsPerPixel = binary.LittleEndian.Uint16(info.Header[28:30])

	// Only 24-bit BMP supported
	if info.BitsPerPixel != 24 {
		return nil, fmt.Errorf("only 24-bit BMP supported")
	}

	return info, nil
}

// ==================== LSB Bit Manipulation ====================

/*
 * Embed a bit into the LSB of a byte.
 * 1. (b & 0xFE) -> Clears LSB. 0xFE = 11111110, AND masks out LSB.
 * 2. (... | bit) -> Sets the new bit value. OR inserts the bit.
 */
func embedBit(b byte, bit byte) byte {
	return (b & 0xFE) | (bit & 0x01)
}

/*
 * Extract the LSB from a byte.
 * (b & 0x01) -> Isolates only the lowest bit, masking others.
 */
func extractBit(b byte) byte {
	return b & 0x01
}

/*
 * Get a specific bit from a character at given position.
 * (ch >> pos) -> Shifts character right.
 * (... & 0x01) -> Isolates the lowest bit.
 * Reads from MSB to LSB (7 to 0).
 */
func getBitAt(ch byte, pos int) byte {
	return (ch >> (7 - pos)) & 0x01
}

// ==================== Hide Function ====================

func hideMessage(inputFile, message string) error {
	fin, err := os.Open(inputFile)
	if err != nil {
		return fmt.Errorf("cannot open %s: %v", inputFile, err)
	}
	defer fin.Close()

	info, err := readBMPHeader(fin)
	if err != nil {
		return fmt.Errorf("invalid BMP file: %v", err)
	}

	// Append NULL terminator to message
	msgBytes := append([]byte(message), 0)
	msgLen := len(msgBytes)

	// Check message size (each char needs 8 bytes)
	requiredBytes := msgLen * 8
	availableBytes := int(info.FileSize) - BMPHeaderSize

	if requiredBytes > availableBytes {
		return fmt.Errorf("message too long. Max %d chars", availableBytes/8-1)
	}

	// Read all pixel data
	pixelData := make([]byte, info.FileSize-BMPHeaderSize)
	if _, err := fin.Read(pixelData); err != nil {
		return fmt.Errorf("failed to read pixel data: %v", err)
	}

	// Embed message into pixel data
	charIdx := 0 // Current character index in message
	bitIdx := 0  // Current bit index in character (0-7)

	for i := range pixelData {
		if charIdx < msgLen {
			// Get bit from message and embed into pixel byte
			msgBit := getBitAt(msgBytes[charIdx], bitIdx)
			pixelData[i] = embedBit(pixelData[i], msgBit)

			bitIdx++
			if bitIdx == 8 {
				bitIdx = 0
				charIdx++
			}
		}
	}

	// Create output file
	fout, err := os.Create(OutputFile)
	if err != nil {
		return fmt.Errorf("cannot create %s: %v", OutputFile, err)
	}
	defer fout.Close()

	// Write header and modified pixel data
	fout.Write(info.Header[:])
	fout.Write(pixelData)

	fmt.Printf("Message hidden successfully: %s\n", OutputFile)
	fmt.Printf("Hidden: \"%s\" (%d chars)\n", message, len(message))

	return nil
}

// ==================== Reveal Function ====================

func revealMessage(inputFile string) error {
	fin, err := os.Open(inputFile)
	if err != nil {
		return fmt.Errorf("cannot open %s: %v", inputFile, err)
	}
	defer fin.Close()

	info, err := readBMPHeader(fin)
	if err != nil {
		return fmt.Errorf("invalid BMP file: %v", err)
	}

	// Read all pixel data
	pixelData := make([]byte, info.FileSize-BMPHeaderSize)
	if _, err := fin.Read(pixelData); err != nil {
		return fmt.Errorf("failed to read pixel data: %v", err)
	}

	// Buffer for maximum possible message size
	maxChars := len(pixelData) / 8
	message := make([]byte, 0, maxChars)

	bitIdx := 0
	var currentChar byte = 0

	for _, pixelByte := range pixelData {
		// Extract LSB and add to character
		bit := extractBit(pixelByte)

		/*
		 * Bit placement:
		 * currentChar |= (bit << (7 - bitIdx))
		 * Shifts bit to correct position and ORs it in.
		 * Builds character from MSB to LSB.
		 */
		currentChar |= (bit << (7 - bitIdx))

		bitIdx++
		if bitIdx == 8 {
			// NULL terminator found, message complete
			if currentChar == 0 {
				break
			}

			message = append(message, currentChar)
			currentChar = 0
			bitIdx = 0
		}
	}

	fmt.Printf("Hidden message: \"%s\"\n", string(message))

	return nil
}

// ==================== Usage Info ====================

func printUsage(progName string) {
	fmt.Println("LSB Steganography Tool")
	fmt.Println("Usage:")
	fmt.Printf("  %s hide <input.bmp> \"message\"  - Hide message in BMP\n", progName)
	fmt.Printf("  %s reveal <input.bmp>          - Reveal hidden message\n", progName)
}

// ==================== Main Program ====================

func main() {
	if len(os.Args) < 3 {
		printUsage(os.Args[0])
		os.Exit(1)
	}

	command := os.Args[1]
	bmpFile := os.Args[2]

	switch command {
	case "hide":
		if len(os.Args) < 4 {
			fmt.Fprintln(os.Stderr, "Error: Message not specified.")
			printUsage(os.Args[0])
			os.Exit(1)
		}
		if err := hideMessage(bmpFile, os.Args[3]); err != nil {
			fmt.Fprintf(os.Stderr, "Error: %v\n", err)
			os.Exit(1)
		}

	case "reveal":
		if err := revealMessage(bmpFile); err != nil {
			fmt.Fprintf(os.Stderr, "Error: %v\n", err)
			os.Exit(1)
		}

	default:
		fmt.Fprintf(os.Stderr, "Error: Invalid command '%s'\n", command)
		printUsage(os.Args[0])
		os.Exit(1)
	}
}
