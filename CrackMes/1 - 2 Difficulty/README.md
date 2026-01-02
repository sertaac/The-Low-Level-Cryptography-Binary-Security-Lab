# 1 - 2 Difficulty CrackMes

This directory contains solutions and write-ups for "Level 1-2" difficulty crackmes. The focus here is on fundamental reverse engineering techniques, including basic static analysis, dynamic analysis with GDB, and understanding standard ELF binary behaviors on Linux x86-64 architecture.

## 1. ezloom's really easy (simp-password)
* **Author:** ezloom
* **Difficulty:** 1.0
* **Link:** [crackmes.one/crackme/691fd9362d267f28f69b7f89](https://crackmes.one/crackme/691fd9362d267f28f69b7f89)
* **Description:** A basic introductory challenge involving standard string comparison logic.
* **Analysis:**
    * Identified hardcoded strings using `strings` command.
    * Traced library calls using `ltrace` to capture the `strcmp` execution.
    * **Key Concepts:** Static Analysis, `strcmp` hooking, Linux CLI tools.

## 2. toasterbirb's argc
* **Author:** toasterbirb
* **Difficulty:** 1.3
* **Link:** [crackmes.one/crackme/68698837aadb6eeafb399017](https://crackmes.one/crackme/68698837aadb6eeafb399017)
* **Description:** A logic puzzle that manipulates the Argument Count (argc) before the main function.
* **Analysis:**
    * The binary modifies `argc` in the `_start` entry point using a bitwise shift (`shr al, 1`).
    * Bypassed the standard `main` function logic by calculating the required argument count to satisfy the shifted value.
    * **Key Concepts:** Entry Point (`_start`) Analysis, Control Flow Manipulation, Bitwise Operations.

## 3. Emilia161's good kitty (crack)
* **Author:** Emilia161
* **Difficulty:** 1.8
* **Link:** [crackmes.one/crackme/68c44e20224c0ec5dcedbf4b](https://crackmes.one/crackme/68c44e20224c0ec5dcedbf4b)
* **Description:** A dynamically linked PIE executable that generates a password using a mathematical algorithm.
* **Analysis:**
    * Analyzed the algorithm involving mathematical operations (factorial, cube root) and byte manipulation.
    * Overcame ASLR/PIE protection by mapping memory segments in GDB (`break *offset`).
    * Retrieved the generated password from the stack at runtime.
    * **Key Concepts:** GDB Dynamic Analysis, ASLR/PIE Bypass, Stack Inspection, Algorithm Analysis, Binary Patching.
