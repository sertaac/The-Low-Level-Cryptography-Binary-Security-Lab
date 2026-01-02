The program is an executable named "argc", to be executed on a linux machine, with the following details:

File Name : argc
Format : ELF 64-bit LSB pie executable, x86-64
Interpreter : /lib64/ld-linux-x86-64.so.2
Scope : to find the correct input to satisfy the argument check

A quick static analysis reveals that this is a 64-bit ELF binary. The name "argc" strongly suggests that the challenge involves the "Argument Count" passed to the program.

### Initial Behavioral Analysis

When running the program with various numbers of arguments, it consistently fails with a friendly error message:

||=====================
||$ ./argc
||please try again and make sure to give the correct amount of arguments („ᵕᴗᵕ„)
||$ ./argc a a a
||please try again and make sure to give the correct amount of arguments („ᵕᴗᵕ„)
||=====================

However, by pure experimentation (fuzzing the argument count), I discovered that providing **5 or 6 arguments** triggers a different path:

||=====================
||$ ./argc a a a a a
||correct! (˶ᵔ ᵕ ᵔ˶)
||$ ./argc a a a a a a
||correct! (˶ᵔ ᵕ ᵔ˶)
||=====================

This is unexpected behavior because standard logic dictates that "argc" counts the program name + arguments. So "./argc a a a a a" should be "argc = 6". Why does the program accept this specific count?

### Static Analysis (Ghidra Decompilation)

Opening the binary in Ghidra reveals the `main` function logic:
"
undefined4 main(int param_1, long param_2) {
  int iVar1;
  
  // param_1 corresponds to 'argc' (Argument Count)
  if (param_1 == 3) {
    // If argc is 3, compare the 1st and 2nd arguments
    iVar1 = strcmp(*(char **)(param_2 + 8), *(char **)(param_2 + 0x10));
    if (iVar1 == 0) {
      puts("correct! (˶ᵔ ᵕ ᵔ˶)"); // Success message
      return 0;
    }
    puts("wrong passwords...");
  }
  else {
    puts("please try again..."); // Fail message
  }
  return 1;
}
"

The code explicitly checks "if (param_1 == 3)".
In C programming, "argc" includes the program name. So "argc == 3" normally means:
"./program arg1 arg2"

But my testing showed that I needed **5 or 6 arguments** to pass!

"./argc arg1 arg2" (Total 3) -> FAILED
"./argc arg1 arg2 arg3 arg4 arg5" (Total 6) -> PASSED

Why does "6" satisfy the condition "param_1 == 3"?

### The Root Cause: Bitwise Shift in "_start"

The answer lies not in "main", but in how "main" is called. The "_start" function (the entry point of the binary) manipulates the argument count before passing it to main.

According to the disassembly of "_start" (and verified by community analysis):
"shr al, 1  ; Shift Right Logical by 1 bit"

This instruction performs a division by 2.

If we pass 6 arguments: Binary 110 >> 1 = 011 (Decimal 3). If we pass 5 arguments: Binary 101 >> 1 = 010 (Decimal 2).

So, the "main" function receives a modified "argc". To make "param_1" equal to 3 inside "main", the real "argc" must be 6 or 7. (Note: "./argc" itself counts as 1. So we need 5 user arguments for Total=6, or 6 user arguments for Total=7).
Secondary Check: strcmp

Once inside the "if (argc == 3)" block, the code compares the arguments:
strcmp(*(char **)(param_2 + 8), *(char **)(param_2 + 0x10));

"param_2 + 8" points to "argv[1]" "param_2 + 0x10" (16) points to "argv[2]"

It checks if "argv[1]" is identical to "argv[2]".

However, in my successful run ("./argc a a a a a"), since the Argument Vector ("argv") pointer logic is not shifted (only the count "argc" was), it likely compared "argv[1]" and "argv[2]" normally. Since I input "a a ...", they were identical ("a" == "a"), so it printed "correct!".
Solution

To solve this crackme, we must satisfy two conditions:

- Modified Argc: Provide enough arguments so that Real_Argc / 2 == 3. This means providing 5 arguments (Total argc = 6) or 6 arguments (Total argc = 7).
- String Compare: Ensure "argv[1]" equals "argv[2]".

Command: "./argc A A x x x" (Where A matches A, and x are filler arguments).

||=====================
||$ ./argc a a 1 2 3
||correct! (˶ᵔ ᵕ ᵔ˶)
||=====================
