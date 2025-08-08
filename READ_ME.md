# Pipelined Processor Emulator

Code written by Leah Nyahunzvi - ln2u23@soton.ac.uk

## Overview
This is a 5 stage pipeline emulator, based on the MIPS architecture. It consists of a custom instruction set and an assembler built using a python script.

### Instructions & Information:

For the marker reading this, just run "build.bat" and it will run the display program

#### 1. RUNNING PROGRAMS

##### 1.1 Display Program for Marking
- No changes to the code are required to run the display program.
- Simply run the batch file "build.bat"
- The display program code is stored in "Program Data/Test Programs" , and there is a copy in "assembly.txt" which is what runs when the build command runs

##### 1.2 Test Programs
- Locate the text file with the test you wish to run in "Program Data/Test Programs"
- Copy the text and past it in "assembly.txt" in the program data folder, replacing what was previously there
- Save and run the build command
    
##### 1.3 Custom Programs     *see syntax rules in assembler info
- Write your program and paste it in assembly.txt
- Run the build command

##### 1.4 Stepping through the execution
- To slowly step through the program line by line, uncomment the "getchar();" in main.called
- This will require a key press after each cycle, and allows you to see the state of the pipeline and walk through the execution


#### 2. ASSEMBLER INFO

##### 2.1 General
- The assembler is a python script that assembles a text file line by line. 
- Generative AI provided a skeleton code for the script, and it was customised and adapted for the custom ISA
- The assembler looks for an assembly file "assembly.txt" and outputs the assembled binary file into "binary.txt"
- The processor then reads the binary file only, and does not interact with the assembly file

##### 2.2 Syntax Rules
ISA with Example Uses

                I_TYPE INSTRUCTIONS
                    addi t2, t3, 5              t2 = t3 + 5                                                     Add immediate
                    subi t4, t5, 99             t4 = t5 + 99                                                    sub immediate
                    bnq t3, t0, LOOP            if (t3 != t0) pc <= line number where loop is defined.          branch not equal
                    bnq t3, t0, 77              if (t3 != t0) pc <= 77
                    sll t9, t8, 7               t9 = t8 << 7                                                    shift logical left
                    slr t1, t2, t3              t1 = t2 >> t3                                                   shift logical right
                    sto t6, t7, 0               MainMemory[t7 + 0] = t6                                         store
                    ld t4, t5, 0                t4 = MainMemory[t5 + 0]                                         load
                    andi t30, t20, 6            t30 = t20 & 6                                                   and immediate
                    ori t25, t1, 44             t25 = t1 | 44                                                   or immediate
                    beq t6, t7, LOOP            if(t6 == t7) pc <= line number where loop is defined            branch equivalent
                    beq t6, t7, 45              if(t6 == t7) pc <= 45    

                R_TYPE INSTRUCTIONS
                    add t20, t21, t22           t20 = t21 + t22                                                 add r type
                    sub t15, t16, t17           t15 = t16 - t17                                                 sub r type 
                    and t11, t12, t13           t11 = t12 & t13                                                 logical and 
                    or t6, t7, t8               t6 = t7 | t8                                                    logical or 
                    not t6, t0, t5              t6 = !t5                                                        inverse
                    sllr t3, t4, t5             t3 = t4 << t5                                                   shift logical left rtype 
                    jmp 45                      pc <= 45                                                        unconditional jump
                
                Code comments
                    When an entire line is to be disregarded by the assembler, it must begine with a #
                    When a comment is placed at the end of a line, it must be preceded by "//"

                    eg:
                        #This line will be ignored by the assembler
                        add t1, t2, t3      //This part will be discarded by the assembler

                Additional
                    

                Warnings
                    -A Line label must be followed by a nop
                        eg  BRANCHPOINT nop
                            //further instructions 
                        -This is to avoid stale instrucions propogating in the pipeline

                    -Branch jump points must be defined before they are referenced
                        eg  LOOP nop                        The current line number is assigned the label "LOOP"
                            addi t5, t7, 9                  //filler
                            bnq t1, t0, LOOP                Immediate is replaced with the line number
                            //This is valid because by the time bnq is called, LOOP has been defined

                        eg2 beq t6, t7, BRANCHPOINT
                            //filler instructions
                            BRANCHPOINT sllr t5, t9, t1
                            //This is NOT valid because the assembler does not know where BRANCHPOINT is the first time it is referenced

                        The solution:   Write the program as in eg2 if that is easier
                                        Run the assembler and look for the output with the labels and line numbers
                                        Manually replace undefined branch points with the line numbers given by the assembler
                        



