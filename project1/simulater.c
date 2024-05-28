/* LC-2K Instruction-level simulator */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8       /* number of machine registers */
#define MAXLINELENGTH 1000
typedef struct stateStruct
{
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);
int convertNum(int num);

int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;

    if (argc != 2)
    {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL)
    {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
         state.numMemory++)
    {

        if (sscanf(line, "%d", state.mem + state.numMemory) != 1)
        {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    for (int i = 0; i < NUMREGS; i++)
    {
        state.reg[i] = 0;
    }

    printState(&state);
    int num = 0;
    while (1)
    {
        int instruction, opcode, regA, regB, destReg, offsetField;
        int halt = 0;

        instruction = state.mem[state.pc++];
        opcode = (instruction >> 22) & 7;
        regA = (instruction >> 19) & 7;
        regB = (instruction >> 16) & 7;
        destReg = instruction & 7;
        offsetField = convertNum(instruction & 0xFFFF);

        switch (opcode)
        {
        case 0:
            state.reg[destReg] = state.reg[regA] + state.reg[regB];
            break;
        case 1:
            state.reg[destReg] = ~(state.reg[regA] | state.reg[regB]);
            break;
        case 2:
            state.reg[regB] = state.mem[state.reg[regA] + offsetField];
            break;
        case 3:
            state.mem[state.reg[regA] + offsetField] = state.reg[regB];
            break;
        case 4:
            if (state.reg[regA] == state.reg[regB])
            {
                state.pc += offsetField;
            }
            break;
        case 5:
            state.reg[regB] = state.pc;
            state.pc = state.reg[regA];
            break;
        case 6:
            halt = 1;
            break;
        case 7:
            break;
        default:
            printf("Error: Unrecognized opcode\n");
            exit(1);
        }

        num++;

        if (halt == 1)
        {
            break;
        }

        printState(&state);
    }

    printf("machine halted\n");
    printf("total of %d instructions executed\n", num);
    printf("final state of machine:\n");
    printState(&state);
    exit(0);
}

void printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i = 0; i < statePtr->numMemory; i++)
    {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++)
    {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}

int convertNum(int num)
{
    /* convert a 16-bit number into a 32-bit Linux integer */
    if (num & (1 << 15))
    {
        num -= (1 << 16);
    }
    return (num);
}
