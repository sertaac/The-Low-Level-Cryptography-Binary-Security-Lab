#!/usr/bin/env python3
"""
Target: hacktooth's KeygenMe again (LINUX)
Difficulty: 2.5 / 5.0
Method: Static Analysis & Reversing custom XOR algorithm
by sertaac
"""

import sys
import argparse

# The "Magic Number" identified in Ghidra: 0xAC4C6B37.
# Since x86 architecture is Little Endian, this integer is stored in memory 
# as the byte sequence: [0x37, 0x6B, 0x4C, 0xAC].
SEED_BYTES = [0x37, 0x6B, 0x4C, 0xAC]

def generate_serial(username: str) -> str:
    """
    Generates a valid serial key for the given username based on the reversed algorithm.
    
    Algorithm Summary:
    1. Initialize a 4-byte accumulator with zeros.
    2. XOR each character of the username into the accumulator (cycling index % 4).
    3. XOR the final accumulator with the hardcoded Seed Bytes.
    4. Convert the result to an Uppercase Hex String.
    """
    
    # Handle edge case: empty username returns the raw seed (as seen in disassembly)
    if not username:
        return "AC4C6B37"

    # Initialize a 4-byte buffer (accumulator)
    accumulator = [0, 0, 0, 0]

    # Process the username string
    for i, char in enumerate(username):
        # XOR the ASCII value of the character with the accumulator.
        # The index wraps around every 4 bytes (equivalent to 'i & 3' in C).
        accumulator[i % 4] ^= ord(char)

    # Final Stage: XOR with the Seed
    final_bytes = []
    for i in range(4):
        # XOR the user-derived byte with the corresponding seed byte
        val = accumulator[i] ^ SEED_BYTES[i]
        final_bytes.append(val)

    # Format the result as a Hexadecimal string (e.g., "A1B2C3D4")
    serial_key = "".join(f"{b:02X}" for b in final_bytes)
    
    return serial_key

def main():
    # Setup argument parser for a professional CLI feel
    parser = argparse.ArgumentParser(description="Keygen for 'KeygenMe again (LINUX)'")
    parser.add_argument("username", nargs="?", help="The username to register")
    
    args = parser.parse_args()
    user_input = args.username

    # If no argument provided, switch to interactive mode
    if user_input is None:
        try:
            print("--- [ KeygenMe Solver v1.0 ] ---")
            user_input = input("[?] Enter Username: ").strip()
        except KeyboardInterrupt:
            print("\n[!] Operation cancelled.")
            sys.exit(0)

    # Generate and display the key
    serial = generate_serial(user_input)
    
    print("-" * 30)
    print(f"[*] Target User: {user_input}")
    print(f"[+] Serial Key:  {serial}")
    print("-" * 30)

if __name__ == "__main__":
    main()
