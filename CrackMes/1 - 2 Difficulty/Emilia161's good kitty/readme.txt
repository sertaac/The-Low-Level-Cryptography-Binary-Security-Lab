The program is an executable named "crack" (originally "good kitty"), to be executed on a linux machine, with the following details:

File Name : crack
Format : ELF 64-bit LSB pie executable, x86-64
Interpreter : /lib64/ld-linux-x86-64.so.2 (Dynamically Linked)
Attributes : BuildID[sha1]=8c584d707909182cb49dab6ebe51cca2217ab1ed, not stripped
Scope : to find the password using multiple reverse engineering techniques

A brief analysis using the file command reveals that the binary is dynamically linked and "not stripped". This implies that debugging symbols are present, simplifying the analysis. Crucially, it is a "PIE executable", meaning memory addresses will be randomized every time it runs (ASLR).

Initial Behavioral Analysis

When executing the program, it outputs a prompt character-by-character and checks the input.

||====================
||$ ./crack
||enter the right password
||test
||bad kitty!
||====================

Using ltrace, we observe calls to mathematical functions (cbrt) and raw system calls (read, write). This hints that the password might be mathematically generated rather than hardcoded in plain text.

Static Analysis (Ghidra Decompilation)

Decompiling the main function in Ghidra reveals a three-step process:

Generation: The program calculates a large number using ppeuler_3 (likely related to Project Euler Problem 3), takes its cube root (cbrt), and then calculates the factorial of the result.

Transformation: It runs a byte-manipulation loop to transform this huge integer into a string stored in the stack (variable local_b8).

Comparison: It compares the user input against this generated string using a loop.

### Solution 1: Binary Patching (The Easy Way)

Instead of solving the math, we can modify the program flow to always succeed.
At address offset 0x14f0, the program checks the comparison result:

TEST AL, AL
JZ 0x101562  ; Jump to "bad kitty!" if AL is 0


By overwriting the conditional jump JZ (Opcode 74 6e) with NOP instructions (Opcode 90 90), the program falls through to the success message regardless of the input.

||====================
||$ ./crack_patched
||enter the right password
||test
||good kitty!
||====================

### Solution 2: Algorithm Analysis

Analyzing the helper functions reveals the mathematical logic:

ppeuler_3() returns 6857 (Largest prime factor of 600851475143).

cbrt(6857) is approx 18.99, cast to int 18.

factorial(18) results in a large 64-bit integer.

The byte manipulation loop converts this integer into ASCII characters.

While valid, this method is tedious as it requires re-implementing the logic in a script.

### Solution 3: Dynamic Analysis (GDB) - The Efficient Way

Since the program generates the password at runtime and stores it in the stack before comparison, we can use GDB to inspect the memory and retrieve the key.

IMPORTANT NOTE ON ASLR/PIE:
Attempting to set a breakpoint at the raw offset (break *0x001014bc) directly will fail with "Cannot access memory". This is because the binary is "PIE" (Position Independent Executable). We must let GDB start the program and map the memory segments first.

Correct Steps:

Start GDB: gdb -q ./crack
Load the program to finding the base address: start (The program pauses at main).

Disassemble to find the comparison logic: disas
Locate the instruction immediately after the read call returns.

Set a breakpoint at that real memory address (it was *0x5555555554bc in my attempt so command is "break *0x5555555554bc").
Continue execution: c

Enter input ("test").
When the breakpoint is hit, inspect the stack memory at offset 0x10.
GDB Output:
(gdb) x/s $rsp + 0x10
0x7fffffffe340: "00sGo4M0passwordenter..."
The first 8 characters "00sGo4M0" constitute the generated password.

||====================
||$ ./crack
||enter the right password
||00sGo4M0
||good kitty!
||====================
