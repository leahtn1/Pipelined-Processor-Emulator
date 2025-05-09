//main.c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "processor.h"
void testfetch();
void testdecode();
void testprocessor();
void testpc();
void displayprocessor();
void display_memory_contents();

int main(){
    
    displayprocessor();
    
    return 0;
}


void displayprocessor(){
    printf("\nPress enter to begin execution");
    getchar();
    init();
    printf("\n\n");

    while (end_count < 5){
        //getchar();
        fetch();
        decode();
        execute();
        check_hazard();
        memoryaccess();
        writeback();
        output();
        update();

    }
    
    deinit();
    printf("\nProgram Execution complete.");
    display_memory_contents();


   
}

void display_memory_contents(){ //Display contents of main memory using vertical ordering

    printf("\n\nNone-zero data in main memory:\n\n");

    typedef struct {
        int address;
        unsigned int data;
    } MemEntry;

    MemEntry entries[131072];  // max mem addresses
    int n = 0;

    for (int j = 0; j < 131071; j++) {
        if (MainMemory[j] != 0) {
            entries[n].address = j;
            entries[n].data = MainMemory[j];
            n++;
        }
    }

    if (n == 0) {
        //printf("\nNo non-zero data in main memory.\n");
        return;
    }

    int cols = 5;
    int rows = (n + cols - 1) / cols;  // Ceil division


    // headings
    for (int c = 0; c < cols; c++) {
        printf("| %-9s %-10s ", "Address", "Data");
    }
    printf("|\n");

    // Divider
    for (int c = 0; c < cols; c++) {
        printf("|----------------------");
    }
    printf("|\n");

    // Print data
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int index = r + c * rows;
            if (index < n) {
                printf("| %-9d %-10u ", entries[index].address, entries[index].data);
            } else {
                printf("| %-9s %-10s ", "", ""); // Empty cell
            }
        }
        printf("|\n");
    }

}


//test functions used durin gdevelopment
void testfetch(){
    for (int j = 0; j < 10; j++){
        pc.now = j;
        fetch();
        printf("\nfetched:%s", fetched);
        printf("\nfd next: %d", fd.next);
    }
}

void testdecode(){
    init();

    fetch();
    decode();
    execute();
    memoryaccess();
    writeback();
    update();

    printf("\nFetched string: %s", fetched);
    update();
    decode();
    //output 
    printf("\nOpcode: %u\nrs: %u\nrt: %u\nrd: %u\n", fd.now.opcode, fd.now.rs, fd.now.rt, fd.now.rd);
    printf("Propogating:\n Data1: %u\n Data2: %u\nDestination: %u", de.next.data1, de.next.data2, de.next.destination);
}

void testprocessor(){
    getchar();
    init();
    printf("\n\n");
    while(end_count < 5){
        //getchar();
        fetch();
        decode();
        execute();
        check_hazard();
        memoryaccess();
        writeback();
        output();
        update();

        if (genregs[9].next < genregs[9].now){      //this is error detection. if the counter goes backwards, show the state of main memory and quit execution
            printf("\n\nERROR");
                
            for(int j =0; j <131071 ; j++){
                if (MainMemory[j] != 0){
                    printf("\n%-6d %-6u      Should be: %-6d", j,MainMemory[j], j*j);
                }
            
            }
            
            exit(1);
        }
    }
    deinit();
    printf("\nProgram Execution complete.\n\nNone-Zero Data in main memory:");
    printf("\n%-10s, %-10s, %-10s", "Address", "Data", "Expected");
    
    for(int j =0; j <131071 ; j++){
        if (MainMemory[j] != 0){
            printf("\n%-10d %-10u      Should be: %-10d", j,MainMemory[j], j*j);
        }
    }



}

void testpc(){
    init();

    printf("\nPC before: %u", pc.now);
    fetch();
    pc.next  = 5;
    pc.now = 1;
    fetch();
    printf("\nPC after manual change: %u", pc.now);
    update();
    printf("\nPC after update: %u", pc.now);
}
