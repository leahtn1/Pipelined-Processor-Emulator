//processor.h

#ifndef PROCESSOR_H
#define PROCESSOR_H

#define BITWIDTH 32

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

void init();
void deinit();

typedef struct {
    uint32_t now;
    uint32_t next;

}reg;

reg genregs[32];         //31 general purpose registers for stack memory


typedef struct {
    uint32_t instr;
    uint8_t opcode;
    uint8_t rs;
    uint8_t rt;
    uint8_t rd;
    uint32_t immediate;
    uint32_t address;
}fd_pipeline;

typedef struct {
    fd_pipeline now;
    fd_pipeline next;
}fd_register;

typedef struct {
    uint32_t instr;
    uint32_t data1;
    uint32_t data2;
    uint32_t destination;
    uint32_t address;
}de_pipeline;

typedef struct {
    de_pipeline now;
    de_pipeline next;
}de_register;

typedef struct {
    uint32_t instr;
    uint32_t result;
    uint32_t data;     //if alu bypassed for mem acces
    uint32_t regaddress;  
    uint32_t memaddress;  
}exmem_pipeline;

typedef struct {
    exmem_pipeline now;
    exmem_pipeline next;
}exmem_register;

typedef struct {
    uint32_t instr;
    uint32_t result;
    uint32_t data;     //if alu bypassed for mem acces
    uint32_t memaddress;  
    uint32_t regaddress;  
}memwb_pipeline;

typedef struct {
    memwb_pipeline now;
    memwb_pipeline next;
}memwb_register;

extern fd_register fd;
extern de_register de;
extern exmem_register exmem;
extern memwb_register memwb;

extern reg pc;

//file handling
extern FILE *progptr;
extern FILE *dataptr;
char progpath[100];
char datapath[100];
char fetched[BITWIDTH];
bool end_ptr;
bool DATAHAZARD1;
bool DATAHAZARD2;
bool LOADUSEHAZARD;
int end_count; 
uint32_t MainMemory[131071];        //2^17 - 1 Memory addresses
long int cycles;

void fetch();
void decode();
void execute();
void memoryaccess();
void writeback();
void update();
void output();
void check_hazard();
void stall();
#endif