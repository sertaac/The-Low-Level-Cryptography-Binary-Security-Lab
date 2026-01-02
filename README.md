# The Low-Level Cryptography & Binary Security Lab

This is my main laboratory for Reverse Engineering.

## Repository Structure

### 1. CrackMes
Solutions, analysis scripts, and detailed write-ups for "crackme" challenges from crackmes.one.

* **1 - 2 Difficulty:**
    * Fundamental reverse engineering concepts.
    * Techniques include basic static analysis (Ghidra), dynamic analysis (GDB, ltrace), memory inspection, and handling ASLR/PIE protections.

* **2 - 3 Difficulty:**
    * Focuses on intermediate reverse engineering tasks.
    * Techniques involve "Keygening" (writing custom key generators), algorithm reversing, and unpacking binaries.

## Tools & Environment
The analyses are made by using these tools:
* **Disassemblers/Decompilers:** Ghidra
* **Debuggers:** GDB (GNU Debugger)
* **Binary Analysis:** strings, ltrace, strace, readelf, binwalk
* **Scripting:** Python
