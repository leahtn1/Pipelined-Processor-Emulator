//is.h - instruction set

#ifndef IS_H
#define IS_H

#define nop 0b00000 
#define jmp 0b00001 

//immediate type instructions
#define addi 0b01000
#define subi 0b01001 
#define bnq 0b01010 
#define sll 0b01011
#define slr 0b01100
#define sto 0b01101
#define ld 0b01110
#define andi 0b01111
#define ori 0b11000
#define beq 0b11001

//register type instructions
#define add 0b10000
#define sub 0b10001
#define and 0b10010
#define or 0b10011
#define not 0b10100
#define sllr 0b10101        //shift left r type

//Jump
#define jmp 0b00001

#endif