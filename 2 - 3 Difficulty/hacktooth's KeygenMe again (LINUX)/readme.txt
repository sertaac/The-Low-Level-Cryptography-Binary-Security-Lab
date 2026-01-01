The program is an executable named keygenme, to be executed on a linux machine, with the following details:

Input SHA256 : 63FE1E3EB10C62A59EF9A84DD36D70E562BBFC0EAF05DCEF6A64AD46654ABE0C 
Input MD5 : 14ED313EAC3E55FF6ADD1165D64C9B9E

File Name : keygenme 
Size : 2.0M (1994512 bytes) 
Format : ELF 64-bit LSB pie executable, 
x86-64 Interpreter : None (Statically Linked) 
Library used : None (Statically Linked - not a dynamic executable) 
Attributes : BuildID[sha1]=abab371cbf0b8ea9db621f3d381aa5717152c75a, no section header 
Scope : to write a keygen for the crackme

A brief analysis using the file command reveals that the binary is statically linked and has no section header. This implies that standard C libraries (like libc) are embedded within the 2.0M binary, and the stripped headers might slightly complicate the static analysis as section information is missing.

At Ghidra's Defined Strings, we can only see 7 strings which they tell us that program is compressed with UPX and needs to be unpacked before decompiling with upx:
00100001	ELF	"ELF"	ds
00100164	GNU	"GNU"	ds
00100184	GNU	"GNU"	ds
001001a8	GNU	"GNU"	ds
009bfa46	/dev/shm	"/dev/shm"	ds
009c090c	$Info: This file is packed with the UPX executable packer http://upx.sf.net $
	"$Info: This file is packed with the UPX executable packer http://upx.sf.net $\n"	ds
009c095b	$Id: UPX 5.02 Copyright (C) 1996-2025 the UPX Team. All Rights Reserved. $
	"$Id: UPX 5.02 Copyright (C) 1996-2025 the UPX Team. All Rights Reserved. $\n"	ds
||=====================
||
||$ upx -d -o keygenme_unpacked keygenme 
||                       Ultimate Packer for eXecutables
||                          Copyright (C) 1996 - 2024
||UPX 4.2.4       Markus Oberhumer, Laszlo Molnar & John Reiser    May 9th 2024
||
||        File size         Ratio      Format      Name
||   --------------------   ------   -----------   -----------
||   8508708 <-   1994512   23.44%   linux/amd64   keygenme_unpacked
||
||Unpacked 1 file.
||=====================

After successfully unpacking the binary with UPX, we verify the file details again to confirm that symbols are restored and the binary structure is readable:
||=====================
||$ file keygenme_unpacked
||keygenme_unpacked: ELF 64-bit LSB pie executable, x86-64, version 1 (GNU/Linux), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=abab371cbf0b8ea9db621f3d381aa5717152c75a, for GNU/Linux 3.2.0, not stripped
||=====================

Now, we can see 31849 defined strings on Ghidra. 
Also, short decompiled main function:
"
void main(undefined8 param_1,size_t param_2,ulong param_3,char *param_4,ulong param_5,
         undefined8 param_6,undefined8 param_7,undefined8 param_8,int param_9,long param_10,
         undefined8 param_11,long *param_12,char *param_13,undefined8 param_14)

{
  int local_c [3];
  
  wxTheAssertHandler = (undefined *)0x0;
  wxLog::ms_logLevel = 5;
  local_c[0] = param_9;
  wxEntry(param_1,param_2,param_3,param_4,param_5,param_6,param_7,param_8,local_c,param_10,param_11,
          param_12,param_13,param_14);
  return;
}
"
This confirms that the application is built using the wxWidgets C++ GUI Framework.

"005fd168	key != WXK_LBUTTON && key != WXK_RBUTTON && key != WXK_MBUTTON	"key != WXK_LBUTTON && key != WXK_RBUTTON && key != WXK_MBUTTON"	ds" and ".strtab::000bb7c4	_ZN12KeygenMeForm7OnLoginER14wxCommandEvent	u8"_ZN12KeygenMeForm7OnLoginER14wxCommandEvent"	utf8" These lines look suspicious because when we execute the program, we get 1 button (register) and 2 spaces (1 for username and 1 for key), but when we press the register button, nothing happens. We should track the OnLogin Command Event.


When we decompile the OnLogin Command Event, there are lots of complicated stuff goes there, that's why we start to clean this mess:
#1: We change the parameter param_1 to "this" because first parameters of classes are the class self.
#2: wxString are the main strings program uses from us. We can rename these wxStrings to simple readable variables. First wxString used for lVar5, which takes local_68 variable, we rename it to usernamePTR. 
#3: Besides memory allocations, a specific hardcoded value is spoted: "0xAC4C6B37". This value is used in an XOR operation towards the end of the calculation loop. This is a seed value for core logic, we'll name it as "seed".
#4: Calculation loop uses our seed in an iterating loop over "usernamePTR": "local_buffer[index & 3] ^= username[index]". Algorithm takes the username, splits it into 4 bytes (since "index & 3" is equivalent to "index % 4") and XOR's the characters. 
#5: Finally, we see a "sprintf" with the format: "%02X". This function converts the calculated integer value into a Hexadecimal String. This hex string is the expected serial key.

I wrote a python script to get key with using this algorithm and run:
||=====================
||$ python3 keygen.py kali             
||------------------------------
||[*] Target User: kali
||[+] Serial Key:  5C0A20C5
||------------------------------
||=====================
When I tried, nothing happened, that's why I continued on second approach: debugging with gdb:
#1: Started gdb with "$ gdb -q ./keygenme_unpacked"
#2: I set a breakpoint at OnLogin event (pressing the register button) "break 'KeygenMeForm::OnLogin(wxCommandEvent&)'"
#3: I run the program by "run" and then entered username "kali" password "123123", then clicked register and program freezed.
#4 When freezed, I disassembled and searched the memory comparison, then found a suspicious row:
"
  0x00005555556dc959 <+969>:   call   0x55555569c1c0 <sprintf@plt> 
"
Then I set a checkpoint to stop when "sprintf" function is called (because it will give 4 bytes of output), it started to give values to rdx (I got them by info registers rdx):
1: 0x5c
2: 0xa
3: 0x20
4: 0xc5
It proves, key found correctly.
