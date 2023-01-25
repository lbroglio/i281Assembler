# i281Assembler
This program takes in code written in the i281 Assembly language. The assembly language is written for the i281 Simulated CPU as part of Iowa State's CprE 281 program.
The program turns the given assembly code into a format that the simulated CPU can accept.

### How to use the assembler
1. If the C++  program isn’t already compiled, compile it. This can be done with any C++ compiler. If you choose to use g++, the program can be compiled by opening a terminal,  navigating to the directory in which the i281Assembler.cpp program is stored, and entering the command:  g++ -o i281Assembler i218Assembler.cpp 
2. Once the program is compiled, run it from the command line with the command ./i281Assembler (Or whatever name you gave the .exe file)
3. At this point, the program will run and prompt you to enter a file path.  Enter the path to the file you wish to compile here.
4. After this, the program should output the compiled code.
5. If your code contains errors, the assembler will print to the console the problem, the problem it found, and what line it is on.


### Output
- #### The i281 assembler program outputs to 5 different sources; 4 unique files and the console. 
  - The first output source is the file User_Data.v. This is a Verilog HDL file used to program the i281’s CPU data memory. 
    All user-declared variables will be set using Verilog assign statements to their user-designated values (0 if the user doesn’t assign a value). 
    Verilog // style comments are also used to denote which variable each assign statement is for. 
  - The second output source is the file User_Code_Low.v This file is a Verilog HDL file used to program the CPU’s code memory with the first 16 instructions (or all if the program is less than 16 instructions) of the user's program. The instructions are in machine code and programmed using Verilog assign statements.
  - The third output is the file User_Code_High.v. It is the same as Output two, except it holds the last 16 instructions (or however many are left) of the program.
  - The fourth output is the file [program _name].bin (The bracket contents are  replaced with the name of the user-submitted file). It is a file containing information about the program provided by the assembler. It has four sections. 
    - The first section is simply the user-provided program with no modification. 
    - The second section is the machine code instructions created by the assembler.
    - The third is a visual representation of the program’s data section. It lists the values of all the variables in the order they are stored in memory.
    - The fourth and final section contains a list of all the possible addresses the program could jump to with their assigned line numbers.
  - The fifth and final output of the program is simply the contents of the .bin file also being output to the console.

