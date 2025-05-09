//processor.c

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "processor.h"
#include "is.h"

//pipeline registers
//initialise all to zero
fd_register fd = {
    {0, 0, 0, 0, 0, 0, 0},  // fd.now
    {0, 0, 0, 0, 0, 0, 0}   // fd.next
};


de_register de = {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0}
};

exmem_register exmem = {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0}
};

memwb_register memwb = {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0}
};

//components
//ALU alu  = {0, 0, 0, 0, 0, 0};
reg pc = {0, 0};

//file handling
FILE *progptr;
FILE *dataptr;
bool end_ptr = 0;
bool DATAHAZARD1 = 0;
bool DATAHAZARD2 = 0;
bool LOADUSEHAZARD = 0;
int end_count = 0;      //when end_ptr goes high, this counts 6 nop to be fed into the pipeline to allow the final instructions to complete
long int cycles = 0;

//char progpath[100] = "program.txt";
char progpath[100] = "Program Data/binary.txt";
char datalocation[100] = "data.bin";

const char* name(uint8_t opcode) {      //This function takes a binary opcode and returns a string name. Used in debugging print statements
    switch(opcode) {
        case nop:  return "nop";
        case addi:  return "addi";
        case subi:  return "subi";
        case bnq:   return "bnq";
        case sll:   return "sll";
        case slr:   return "slr";
        case sto:   return "sto";
        case ld:    return "ld";
        case andi:  return "andi";
        case ori:   return "ori";
        case beq:   return "beq";
        case add:   return "add";
        case sub:   return "sub";
        case and:   return "and";
        case or:    return "or";
        case not:   return "not";
        case sllr:  return "sllr";
        default:    return "unknown";
    }
}


void init(){
    //set all general purpose registers to zero
    for (int i = 0; i < 32; i++){
        genregs[i].now = 0;
        genregs[i].next = 0;
    }

    for (int i = 0; i < 131017; i++){
        MainMemory[i] = 0;
    }
    DATAHAZARD1 = 0;
    DATAHAZARD2 = 0;

    // Read in the assembly file
    progptr = fopen(progpath,"r");
    if (progptr == NULL){
        printf("File cannot be opened");
        exit(1);
    }
}
void deinit(){
    //set all general purpose registers to zero
    fclose(progptr);

}

void fetch(){
    //printf("\nFetch Hazards: %u %u", DATAHAZARD1, DATAHAZARD2);


    progptr = fopen(progpath,"r");
    if (progptr == NULL){
        printf("File cannot be opened");
        exit(1);
    }
    
    memset(fetched, '0', sizeof(fetched));
    pc.next = pc.now + 1;
    char temp[BITWIDTH*2] = {'0'};

    int i =0;
    

    while (fgets(temp, sizeof(temp), progptr)) {
        if (end_ptr){           //This branch is taken when the last instruction has been read. NOP is fed into the pipeline until the final instruction has completed
            end_count++;
            memset(fetched, '0', sizeof(fetched) - 1);  // Fill the string with '0's
            fetched[sizeof(fetched) - 1] = '\0';  // Null-terminate the string
            fd.next.instr = 0;
            //printf("\nI>PC. RESETTING");
            pc.next = pc.now;
            break;
        }else if (i == pc.now){
            temp[strcspn(temp, "\r\n")] = '\0';           //strip newlines
            memcpy(fetched, temp, sizeof(fetched)); 

            //convert fetched string into 32bit integer and prepare pipeline register
            fd.next.instr = 0;
            for (int p = 0; p < strlen(fetched); p++){
                fd.next.instr <<= 1;  // Shift left to make room for the next bit
                if(fetched[p] == '1'){
                    fd.next.instr += 1;
                }
            }

            // Check if we've reached the end of the file.  If end, endflag set high and next is NOP
            if (fgets(temp, sizeof(temp), progptr) == NULL) {
                printf("\nEND****************");
                end_ptr = 1;  // Set the end pointer flag
            } else {
                // If a line is successfully read, it must be "unread" for future use
                fseek(progptr, -strlen(temp), SEEK_CUR);            //AI assistance with structuring the fseek parameters
            }

            break;
        }
        i++;
    }

    fclose(progptr);
}


void decode(){
    cycles++;
    //printf("\nDecode Hazards: %u %u", DATAHAZARD1, DATAHAZARD2);
    fd.now.opcode = (fd.now.instr >> 27) & 0x1F;            //0x1F = 00011111
    uint8_t op = fd.now.opcode;
    uint32_t instr = fd.now.instr;

    de.next.instr = instr;

    if ((op & 0b11000) == 0b10000){      //R-Type instruction in the form 10xxx
        //printf("\nDECODING R_TYPE");
        fd.now.rs = (instr>>22) & 0x1F;
        fd.now.rt = (instr>>17) & 0x1F;
        fd.now.rd = (instr>>12) & 0x1F;
        fd.now.immediate = 0;
        fd.now.address = 0;

        //setting up the data to be propogated to the next pipeline register - decode/execute

        de.next.data1 = genregs[fd.now.rs].now;             
        de.next.data2 = genregs[fd.now.rt].now;             
        de.next.destination = fd.now.rd;       
        
    }else if ((op & 0b01000) == 0b01000){        //I type instruction in the form of 01xxx or 11xxx
        fd.now.rs = (instr>>22) & 0x1F;
        fd.now.rt = (instr>>17) & 0x1F;
        fd.now.immediate = instr & 0x1FFFF;         //0x1FFFF = 0001 1111 1111 1111 1111
        //printf("\nDECODING I_TYPE\nrs: %u\nrt: %u\nimm: %u\n", fd.now.rs, fd.now.rt, fd.now.immediate);
        fd.now.rd = 0;
        fd.now.address = 0;
        de.next.data1 = genregs[fd.now.rs].now;             
        de.next.data2 = fd.now.immediate;             
        de.next.destination = fd.now.rt;

        //structure changes for branch and memory instructions
        if ((op == bnq)||(op == beq)){
            de.next.data2 = genregs[fd.now.rt].now;         //second compare value     
            de.next.destination = fd.now.immediate;         //branch address
            //printf("\nDecoding branch. data1 %u data2 %u destination %u", de.next.data1, de.next.data2, de.next.destination);
        }else if (op == ld){
            de.next.data1 = fd.now.rs;                  //destination register
            de.next.data2 = fd.now.immediate;           //offset
            de.next.destination = genregs[fd.now.rt].now;     //memory address
        }else if (op == sto){
            de.next.data1 = genregs[fd.now.rs].now;     //data to be written to memory
            de.next.destination = genregs[fd.now.rt].now;     //memory address target
            de.next.data2 =  fd.now.immediate;          //offset
        }

    }else if ((op & 0b00001) == 0b00001){       //FIX JUMP
        //printf("\nDECODING J_TYPE");
        de.next.destination = fd.now.instr & 0x07FFFFFF;               //07FFFFFF = 0000 0111 1111 1111 1111 1111 1111 1111
    }else if (op == nop){
        de.next.data1 = 0;             
        de.next.data2 = 0;             
        de.next.destination = 0; 
        fd.now.instr = 0;    
        fd.now.opcode = 0;    
        fd.now.rs = 0;    
        fd.now.rt = 0;    
        fd.now.rd = 0;    
        fd.now.immediate = 0;    

    }

        //This forwarding logic prevents data hazards. Ex/Mem register has the highest priority bc it holds the most updated data
        if ((exmem.now.regaddress == fd.now.rs) && (fd.now.rs != 0)) {              //if exmem target register == fd source register, forward from EXMEM
            //printf("/nexmem.now.address %u fd.now.rs %u", exmem.now.regaddress, fd.now.rs);
            de.next.data1 = exmem.now.result;
            //printf("\nForward from EXMEM to rs");
        }
        else if ((memwb.now.regaddress == fd.now.rs) && (fd.now.rs != 0) && fd.now.opcode!=sto) {
            de.next.data1 = memwb.now.data;
            //printf("\nForward from MEMWB to rs");
        }


        //same logic as above except store is explicitly excluded from the condition  because sto instructions take a different format
        if ((exmem.now.regaddress == fd.now.rt) && (fd.now.rt != 0) && (fd.now.opcode != sto) && ( ( (exmem.now.instr >> 27) & 0x1f) != sto)) {
            //printf("\nForward from EXMEM to rt");
            //printf("\nBefore: %u -- Replaced with: %u", de.next.data2, exmem.now.result);
            de.next.data2 = exmem.now.result;
            //printf("\nexmem.now.address %u op %s \nfd.now.rt %u op %s", exmem.now.regaddress,name((exmem.now.instr>>27) & 0x1f), fd.now.rt, name(fd.now.opcode));
        }
        else if ((memwb.now.regaddress == fd.now.rt) && (fd.now.rt != 0) && (fd.now.opcode != sto)) {
            de.next.data2 = memwb.now.data;
            //printf("\nForward from MEMWB to rt");
        }
    
        //printf("\naftercheck\nNEXT Data1: %u       Data2: %u", de.next.data1, de.next.data2);
        //printf("\naftercheck\nNOW Data1: %u       Data2: %u", de.now.data1, de.now.data2);
/*
        //this forwarding prevents stale instructions during branching
        if (de.next.instr == bnq){
            if ( (fd.now.rs == exmem.now.regaddress) && (fd.now.rs != 0) ){
                de.next.data1 = exmem.now.data;
                printf("\n\n\n1ACHEIVED*******************************************");
            }else if ( (fd.now.rs == memwb.now.regaddress) && (fd.now.rs != 0)) {
                printf("\n\n\n2ACHEIVED*******************************************");
                de.next.data1 = memwb.now.data;

            }
                
            if( (fd.now.rt == exmem.now.regaddress) && (fd.now.rt != 0)){
                printf("\n\n\n3ACHEIVED*******************************************");
                de.next.data2 = exmem.now.data;
            }else if ( (fd.now.rt == memwb.now.regaddress) && (fd.now.rt != 0)) {
                printf("\n\n\n4ACHEIVED*******************************************");
                de.next.data2 = memwb.now.data;

            }

        }
*/

}
void execute(){
    //printf("\nExecute Hazards: %u %u", DATAHAZARD1, DATAHAZARD2);
    uint8_t op = (de.now.instr >> 27) & 0x1F;
    exmem.next.instr = de.now.instr;
    exmem.next.regaddress = de.now.destination;

    //printf("\nAlu NOW. Data1 %u    Data2: %u", de.now.data1, de.now.data2);
    //printf("\nAlu NEXT. Data1 %u    Data2: %u", de.next.data1, de.next.data2);

    switch (op) {
        case (nop):                 //no operation
            exmem.next.result = 0;
            printf("\nExecute NOP");
            break;

        case jmp:                   //unconditional jump
            pc.next  = de.now.destination;
            //insert NOP to lush
            fd.next.instr = 0;
            fd.now.instr =  0;
            de.now.instr = 0;
            de.next.instr = 0;
            printf("\nUncoditional Jump to %u", pc.next);
            break;

        case 0b00010:                   //
            break;

        case 0b00011:                   //
            break;

        case 0b00100:                   //
            break;
            
        case 0b00101:                   //
            break;

        case 0b00110:                   //
            break;

        case 0b00111:                 //
            break;

        case addi:                   //add immediate
            exmem.next.result = de.now.data1 + de.now.data2;
            printf("\naddi t%u = %u + %u", exmem.next.regaddress, de.now.data1, de.now.data2);
            break;

        case subi:                   //sub immediate
            exmem.next.result = de.now.data1 + de.now.data2;
            printf("\nsubi t%u = %u + %u", exmem.next.regaddress, de.now.data1, de.now.data2);
            break;

        case bnq:                   //BNQ
            if (de.now.data1 != de.now.data2) {
                int32_t offset = de.now.destination;
                printf("\ncomp %u, %u     BNQ to %u",de.now.data1, de.now.data2, offset);
                // Sign-extend 17-bit immediate
                //offset = (int32_t)(de.now.address << 15) >> 15;  // Sign-extend from 17 bits to 32
                pc.next = offset;  // Update PC with offset ( branch)
                
                memset(fetched, '0', sizeof(fetched));
                fd.next.instr = 0;  // Insert NOP to flush
                //fd.now.instr = 0; 
                de.next.instr = 0;
                //de.now.instr = 0;

                //exmem.next = exmem.now;           //CHECK this !!!!!!!!!
                //memwb.next.instr = 0;
            }
            break;
            
        case sll:                   //shift left
            printf("\nsll t%u = %u << %u", exmem.next.regaddress, de.now.data1, de.now.data2);
            exmem.next.result = de.now.data1 << de.now.data2;
            
            break;

        case slr:                   //shift right
            printf("\nslr t%u = %u >> %u", exmem.next.regaddress, de.now.data1, de.now.data2);
            exmem.next.result = de.now.data1 >> de.now.data2;
            break;

        case sto:                   //store
            exmem.next.memaddress = de.now.destination + de.now.data2;          //base + offset
            exmem.next.data = de.now.data1;                                     //data to be written

            printf("\nStore Data: %u to address: %u *************************************************************************************************************", exmem.next.data, exmem.next.memaddress);
            break;

        case ld:                   //load
            exmem.next.memaddress = de.now.destination + de.now.data2;          //base + offset  
            exmem.next.regaddress = de.now.data1;
            printf("\nLoad data from memory address: %u to t%u", exmem.next.memaddress, exmem.next.regaddress);
            break;

        case andi:                   //and immediate
            printf("\nt%u = %u & %u", exmem.next.regaddress, de.now.data1, de.now.data2);
            exmem.next.result = de.now.data1 & de.now.data2;
            break;
            
        case add:                   //add
            printf("\nt%u = %u + %u", exmem.next.regaddress, de.now.data1, de.now.data2);
            exmem.next.result = de.now.data1 + de.now.data2;
            break;

        case sub:                   //subtract
            printf("\nt%u = %u - %u", exmem.next.regaddress, de.now.data1, de.now.data2);
            exmem.next.result = de.now.data1 - de.now.data2;
            break;
            
        case and:                   //and
            printf("\nt%u = %u & %u", exmem.next.regaddress, de.now.data1, de.now.data2);
            exmem.next.result = de.now.data1 & de.now.data2;
            break;

        case or:                   //or
            printf("\nt%u = %u or %u", exmem.next.regaddress, de.now.data1, de.now.data2);
            exmem.next.result = de.now.data1 | de.now.data2;
            break;
        
        case not:                   //Not
            printf("\nt%u = !%u ", exmem.next.regaddress, de.now.data1);
            exmem.next.result = !de.now.data1;
            break;

        case sllr:                   //
            printf("\nt%u = %u << %u", exmem.next.regaddress, de.now.data1, de.now.data2);
            exmem.next.result = de.now.data1 << de.now.data2;
            break;
            
        case 0b10110:                   //
            break;

        case 0b10111:
            break;
        
        case ori:                   //or immediate
            printf("\nt%u = %u or %u", exmem.next.regaddress, de.now.data1, de.now.data2);
            exmem.next.result = de.now.data1 | de.now.data2;
            break;

        case beq:                   //BEQ
            if (de.now.data1 == de.now.data2) {
                int32_t offset = de.now.destination;
                printf("\nCompare %u, %u    BEQ to %u", de.now.data1, de.now.data2, offset);
                // Sign-extend 17-bit immediate
                //offset = (int32_t)(de.now.address << 15) >> 15;  // Sign-extend from 17 bits to 32
                pc.next = offset;  // Update PC with offset ( branch)
                
                
                fd.next.instr = 0;  // Insert NOP to flush
                //fd.now.instr = 0; 
                de.next.instr = 0;
                //de.now.instr = 0;
                memset(fetched, '0', sizeof(fetched));
            }
            break;
            
        case 0b11010:                   //
            break;

        case 0b11011:
            break;
        
        case 0b11100:                   //
            break;

        case 0b11101:                   //
            break;
            
        case 0b11110:                   //
            break;

        case 0b11111:
            break;

    }
    
    
}

void memoryaccess(){
    //printf("\nMemory Hazards: %u %u", DATAHAZARD1, DATAHAZARD2);
    uint8_t op = (memwb.next.instr >> 27) & 0x1F;
    memwb.next.instr = exmem.now.instr;
    memwb.next.result = exmem.now.result;
    memwb.next.regaddress = exmem.now.regaddress;
    memwb.next.data = exmem.now.result;

    switch(op){
        //if load (from memory)
        case ld:
        /*
            dataptr = fopen(datapath, "rb");
            uint32_t temp = 0;
            int i = 0;

            fseek(dataptr, exmem.now.memaddress*sizeof(uint32_t), SEEK_SET);
            fread(&temp, sizeof(uint32_t), 1, dataptr);
            memwb.next.data = temp;
            printf("\nTo be loaded from nmemory: %u", temp);
            fclose(dataptr);
        */
            memwb.next.data = MainMemory[exmem.now.memaddress];
            

        //write to memory
        case sto:
        /*
            dataptr = fopen(datapath, "wb");
            fseek(dataptr, sizeof(uint32_t) * exmem.now.memaddress, SEEK_SET);
            fwrite(&exmem.now.data, sizeof(uint32_t), 1, dataptr);
            printf("\nTo be stored to nmemory: %u", temp);
            fclose(dataptr);
        */
            MainMemory[exmem.now.memaddress] = exmem.now.data;
            //memwb.next.data = genregs[exmem.now.regaddress].now;
            break;

    }

}
void writeback(){
    //printf("\nWB Hazards: %u %u", DATAHAZARD1, DATAHAZARD2);
    //only r type and some I type write back to registers
    uint8_t op = (memwb.now.instr >> 27) & 0x1F;
    
    //all R and I type instructions write back except sto, bnq & beq
    if ( ((op & 0b10000) == 0b10000) && (op != sto) && (op != beq) && (op != bnq)) {                                //all r type instructions write back
        //printf("\nShould wb now. Target addy %u, data %u \nOP: %u", memwb.now.regaddress, memwb.now.data, op);
        genregs[memwb.now.regaddress].next = memwb.now.data;

    }else if( ((op & 0b01000) == 0b01000) &&  (op != sto) && (op != beq) && (op != bnq)){        //only wb if i type and not store
        //printf("\nShould wb now. Target addy %u, data %u \nOP: %u", memwb.now.regaddress, memwb.now.data, op);
        genregs[memwb.now.regaddress].next = memwb.now.data;
    }

}

void update(){
    pc.now = pc.next;
    //update pipeline registers
    fd.now.instr = fd.next.instr;
    de.now = de.next;
    exmem.now = exmem.next;
    memwb.now = memwb.next;


    //update general purpose registers
    for (int i = 0; i < 32; i++){
        genregs[i].now = genregs[i].next;
        
    }
    genregs[0].now = 0;
    
}

void check_hazard(){

        //These checks must take place after the execute stage, because they aim to forward the most updated alu result back to the input. Once again, sto is excluded from these conditions
        DATAHAZARD1 = 0;
        DATAHAZARD2 = 0;
        LOADUSEHAZARD = 0;
        //printf("\nChecking hazards\nfd.rs: %u   fd.rt: %u   destination: %u", fd.now.rs, fd.now.rt, de.now.destination);
   

        
        if( (de.now.destination == fd.now.rs) && (fd.now.rs != 0) && ( ((de.now.instr >>27) & 0x1f) != sto)){
            DATAHAZARD1 = 1;
            //printf("\nData hazard detected. Forwarded from alu result to rs");
            //printf("\nSource: %u\nDestination: %u\nCurrent: %u\nReplace with: %u", de.now.destination, fd.now.rs, de.next.data1, exmem.next.result);
            de.next.data1 = exmem.next.result;
        
        }
        
        if( (de.now.destination == fd.now.rt) && (fd.now.rt !=0) && (fd.now.opcode != sto) && ( ((de.now.instr >>27) & 0x1f) != sto)){
            DATAHAZARD2 = 1;
            //printf("\nData hazard detected. Forwarded from alu result to rt");
            //printf("\nSource: %u\nDestination: %u\nCurrent: %u\nReplace with: %u", de.now.destination, fd.now.rt, de.next.data2, exmem.next.result);
            de.next.data2 = exmem.next.result;
        }
        
        


    //load-use hazard
    //if executing a load, is the next instruction in DECODE using the same reg address?
    if((de.now.instr >> 27) == ld){
        if( ((fd.now.rs == de.now.destination) || (fd.now.rt == de.now.destination)) && (de.now.destination != 0)){
            //stall
            printf("\nLoad/Use hazard detected. Pipeline stalled");
            fd.next = fd.now;
            de.next = de.now;
            pc.next = pc.now;
        }
    }
}

void output(){
    printf("\n\nCycles: %d", cycles);
    printf("\nPC: %d", pc.now);
    printf("\n\nPipeline:");
    printf("\nStage:       FD         DE         EXMEM       MEMWB");
    printf("\n            %-10s %-10s %-10s %-10s\n",
        name(fd.now.opcode),
        name((de.now.instr >> 27) & 0x1F),
        name((exmem.now.instr >> 27) & 0x1F),
        name((memwb.now.instr >> 27) & 0x1F));
       
    //regs
    
    printf("\n\n%-10s%-20s%-20s%-20s%-20s\n", "Index", "Col 1", "Col 2", "Col 3", "Col 4");
    printf("_____________________________________________________________________________\n");

    for (int row = 0; row < 8; row++) {
        printf("%-10d", row + 1); // Just an index label for the row
        for (int col = 0; col < 4; col++) {
            int regIndex = col * 8 + row;
            printf("t%-2d: %6u/%-6u    ", regIndex, genregs[regIndex].now, genregs[regIndex].next);
        }
        printf("\n");
    }
    printf("_______________________________________________________________________________________________________");
}
void stall(){
    printf("\nstalled");
    fd.next.instr = 0;
    pc.next = pc.now;
    de.next = de.now;
    exmem.next = exmem.now;
}



            