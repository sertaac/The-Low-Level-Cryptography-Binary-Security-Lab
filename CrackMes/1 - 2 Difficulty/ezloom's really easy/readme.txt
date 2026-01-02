The program is an executable named simp-password, to be executed on a linux machine, with the following details:

File Name : simp-password
Format: ELF 64-bit LSB pie executable, x86-64
Interpreter : /lib64/ld-linux-x86-64.so.2 (Dynamically Linked)
Attributes : BuildID[sha1]=0ce043e55a600ecf0dc30ee1949e298d314402fe, not stripped
Scope : to find the password for the crackme

A brief analysis using the `file` command reveals that the binary is dynamically linked and "not stripped". This implies that debugging symbols are present, which makes our analysis significantly easier compared to stripped binaries.

When we execute the program simply to test behavior, it asks for a password. Inputting "test" results in a "Wrong try again." message.

At the `strings` command output, we can see standard library functions, but towards the end, a cluster of interesting strings appears:
||====================
||$ strings simp-password
||Enter password: 
||%41s
||iloveicecream
||I love ice cream too!
||Wrong try again.
||====================

These lines look suspicious because they follow a logical flow: A prompt ("Enter password:"), a format specifier for input ("%41s"), a candidate string ("iloveicecream"), and success/failure messages. The string "iloveicecream" stands out as a potential hardcoded password.

To confirm this suspicion without decompiling, I continued on the second approach: dynamic analysis with "ltrace". This tool allows us to see library calls (like string comparisons) in real-time.

||====================
||$ ltrace ./simp-password 
||printf("Enter password: ")                       = 16
||__isoc99_scanf(0x55cc51271015, 0x7ffcb03d5300, 0, 0Enter password: test) = 1
||strcmp("test", "iloveicecream")                  = 11
||puts("Wrong try again."Wrong try again.)         = 17
||+++ exited (status 0) +++
||====================

When we run "ltrace", we clearly see the program taking our input "test" and immediately calling "strcmp" to compare it against "iloveicecream". Since "strcmp" returned 11 (non-zero), the program proceeded to the failure message "puts("Wrong try again.")".

This confirms that the application compares user input directly against a hardcoded string.

Finally, we run the program with the discovered password:

||====================
||$ ./simp-password 
||Enter password: iloveicecream
||I love ice cream too!
||====================
