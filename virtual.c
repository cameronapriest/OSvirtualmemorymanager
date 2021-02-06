#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define BLU "\x1B[34m"
#define PUR "\x1B[35m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define RED "\x1B[31m"
#define END "\x1B[0m"

#define SIZE 32
#define PAGE_TABLE_ENTRIES 256
#define PAGE_SIZE_IN_BYTES 256
#define TLB_ENTRIES 16
#define FRAME_SIZE_IN_BYTES 256
#define NUM_FRAMES 256
#define PHYSICAL_MEMORY 65536 /* in bytes (256 frames x 256-byte frame size) */

/* given a page number from 0-255, reads 256 bytes of data from backing store
   at the location of the page number and stores it in physical memory at the
   next available location. */
void addToMem(int pageNumber);

int TLB[TLB_ENTRIES][2];
char physicalMemory[256][256];
int pageTable[PAGE_TABLE_ENTRIES];

int physicalCounter = 0;
int tlbCounter = 0;

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf(RED "\nPlease enter a file with addresses as input:\n" END);
        printf(GRN "./virtual addresses.txt\n\n" END);
        return -1;
    }

    int i;
    for (i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        pageTable[i] = -1;
    }

    for (i = 0; i < TLB_ENTRIES; i++) {
        TLB[i][0] = -1;
        TLB[i][1] = -1;
    }

    FILE *file;
    FILE *output;
    char *temp;
    char nums[SIZE];
    int currentAddr;
    int numAddresses = 1;

    file = fopen(argv[1], "r");
    output = fopen("output.txt", "a");

    while (fgets(nums, SIZE, file)) {
        temp = strdup(nums);
        currentAddr = atoi(temp);

        int p; /* page number */
        int d; /* page offset */
        int f = -1; /* frame number, between 0-255 */

        int pa; /* the physical address */
        int val; /* the value at the physical address in physical memory */

        int addr = currentAddr & 0xFFFF; /* 32-bit Addr & 00000000000000001111111111111111 */

        p = addr & 0xFF00;  /* 16-bit Addr & 1111111100000000 */
        d = addr & 0xFF;    /* 16-bit Addr & 0000000011111111 */

        p = p >> 8;

        printf("Addr " PUR "%d" END ": " BLU "%d " END, numAddresses, addr);

        for (i = 0; i < TLB_ENTRIES; i++) {
            if (TLB[i][0] == p) {

                /* TLB hit */

                f = TLB[i][1];
                printf(GRN "TLB Hit! TLB[%d], Page #%d, Frame #%d\n" END, i, p, f);

                break;
            }
        }

        if (f < 0) {

            /* TLB miss */
            
            if (pageTable[p] >= 0) {

                /* pageTable[p] holds the frame number.
                   To find the physical address, add (the frame number << 8) + the offset, d.
                   To find the value, index into physical memory at the physical address. */

                printf(YEL "Page table hit at page #%d\n" END, p);

                f = pageTable[p];

            } else {
                printf(RED "Page fault!\n" END);

                /* Page fault:
                   Add the frame corresponding to the page number to physical memory.
                   Update the page table with the frame number.
                   Update the TLB with the page number and the frame number. */

                addToMem(p);

                pageTable[p] = physicalCounter;
                f = physicalCounter;
                
                TLB[tlbCounter][0] = p;
                TLB[tlbCounter][1] = physicalCounter;

                physicalCounter++;

                if (tlbCounter < 15) {
                    tlbCounter++;
                } else {
                    tlbCounter = 0;
                }
            }
        }

        pa = (f << 8) + d;
        val = physicalMemory[f][d];

        fprintf(output, "Virtual address: %d Physical address: %d Value: %d\n", addr, pa, val);
        
        numAddresses++;

        free(temp);
    }

    fclose(file);
    fclose(output);

    return 0;
}

void addToMem(int pageNumber) {

    pageNumber = pageNumber << 8;

    FILE *file;
    file = fopen("BACKING_STORE.bin", "r");

    fseek(file, pageNumber, SEEK_SET);

    char data[FRAME_SIZE_IN_BYTES];

    fread(data, sizeof(char), FRAME_SIZE_IN_BYTES, file);
    fclose(file);

    int i;
    for (i = 0; i < FRAME_SIZE_IN_BYTES; i++) {
        physicalMemory[physicalCounter][i] = data[i];
    }
}