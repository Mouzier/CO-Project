# Project-Phase-1

Project Phase 1 - Team R.A.M.

Instructions on running an assembly code on the program.

1. The assembly code can be of any file type (Preferably .txt or .asm). The instruction file should be in the same folder as the assembler program.
2. On running the simulator, the file name should be entered in the console.
3. It outputs 32 registers in a line followed by 1024 memory elements.

General Instructions (Assembly code)

The register names are same as qtspin. 

1.  Each instruction should be on a different line.
2.  "label:" should be in a different line
3.  Code only checks for valid instructions - add, addi, sub, subi, li, bne, lw, sw, beq, ble, j
4.  Any instructions other than the ones above will be skipped.
5.  Any other arguements after the ones that are required by the instructions will be ignored. (EX: add $r1, $r2, $r3 I am a human ---- "I am a human" will be skipped)
6.  To write any comment "# " is not necessary.
7.  Code works even without .txt, .globl main, .data.
8.  Memory works only for integer type elements (Any other type of data type will give error if attempted to be stored in the memory)
9.  All the integer to be stored in the memory should be there on the same line following a ".word". (In that line there should be only the input of character "0-9", ",",      "+","-" and spaces) (Any other characters will result in error)
10. Input of numbers should be of valid type.(Ex "2-3" is invalid, "-23" is valid) 
11. The array size to be entered as "n-1" (where n is the actual size)
	(In the bubblesort.asm, it is on line 17)
12. li $(register_name), "number" is the format to be followed while loading ("number" can be decimal or hexadecimal)
13. lw $(register_name), offset($(register_name)). This offset should be of type int and offset mod 4 should be 0.
14. In addi instructions, the number should be a decimal.
15.  $zero value can be changed so be careful.
16. The base address is considered as 0x10010000 for memory.


While working on the project, we encountered some problems with the ide,we used. There were some problems regarding how tab spaces are recognized by our code. Sometimes, the code runs without any problem and sometimes it gives an error. We are unable to exactly pinpoint the problem.
We thought that there may be a problem in the input of the instructions and we found out that it recognized many more whitespaces than required. (We output the samething again to see if it matches the input and found many more whitespaces in some of the cases)
