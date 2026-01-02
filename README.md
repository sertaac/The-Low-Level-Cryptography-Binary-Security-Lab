# The Low-Level Cryptography & Binary Security Lab

This repository serves as a centralized laboratory for documentation, research, and practical exercises in reverse engineering, binary exploitation, and low-level cryptography. It aims to document the learning process, ranging from basic binary analysis to advanced exploitation techniques and custom tool development.

## Repository Structure

The repository is organized by category and difficulty level to track progression in technical skills.

### 1. CrackMes
Contains solutions, analysis scripts, and detailed write-ups for various "crackme" challenges sourced from platforms like crackmes.one.

* **1 - 2 Difficulty:**
    * Focuses on fundamental reverse engineering concepts.
    * Techniques include basic static analysis (Ghidra), dynamic analysis (GDB, ltrace), memory inspection, and handling ASLR/PIE protections.
    * Includes write-ups for introductory challenges such as basic string comparison and simple algorithm analysis.

* **2 - 3 Difficulty:**
    * Focuses on intermediate reverse engineering tasks.
    * Techniques involve "Keygening" (writing custom key generators), algorithm reversing, and unpacking binaries.
    * Aims to move beyond binary patching towards full understanding and replication of internal logic.

## Tools & Environment
The analyses within this repository are primarily conducted using standard industry tools:
* **Disassemblers/Decompilers:** Ghidra, IDA Pro
* **Debuggers:** GDB (GNU Debugger), x64dbg
* **Binary Analysis:** strings, ltrace, strace, readelf, binwalk
* **Scripting:** Python (for exploit development and keygening), C/C++
