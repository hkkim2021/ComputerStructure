
/* Assembler code fragment for LC-2K */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000

typedef struct LABEL
{
    char name[7];
    int value;
    int addr;
} LABEL;

LABEL Label[MAXLINELENGTH];

/*instruction types*/
int Rtype(char *opcode, char *regA, char *regB, char *destReg);
int Itype(char *opcode, char *regA, char *regB, char *offset, int pc);
int Jtype(char *regA, char *regB);
int Otype(char *opcode);

int readAndParse(FILE *, char *, char *, char *, char *, char *);

/*error checking*/
int isNumber(char *);
void checkReg(char *);

int idx = 0;

int main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
        arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3)
    {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
               argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL)
    {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL)
    {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    /* TODO: Phase-1 label calculation */
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        if (strcmp(label, ""))
        {
            if (strlen(label) > 6 || isNumber(label))
            {
                printf("Error1: Use of undefined labels");
                exit(1);
            }
            // 중복 check
            for (int i = 0; i < idx; i++)
            {
                if (!strcmp(Label[i].name, label))
                {
                    printf("Error2: Duplicated definition of labels");
                    exit(1);
                }
            }

            strcpy(Label[idx].name, label);
            Label[idx].addr = idx;

            if (!strcmp(opcode, ".fill"))
            {
                if (isNumber(arg0))
                {
                    Label[idx].value = atoi(arg0);
                }

                else
                {
                    int check = 0;
                    for (int i = 0; i < idx; i++)
                    {
                        if (!strcmp(Label[i].name, arg0))
                        {
                            Label[idx].value = Label[i].addr;
                            check = 1;
                            break;
                        }
                    }

                    if (check == 0)
                    {
                        printf("Error1: Use of undefined labels");
                        exit(1);
                    }
                }
            }
        }
        idx++;
    }

    /* this is how to rewind the file ptr so that you start reading from the
        beginning of the file */
    rewind(inFilePtr);

    /* TODO: Phase-2 generate machine codes to outfile */
    int line = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        int k = 0;

        if (!strcmp(opcode, "add") || !strcmp(opcode, "nor"))
        {
            k = Rtype(opcode, arg0, arg1, arg2);
        }
        else if (!strcmp(opcode, "lw") || !strcmp(opcode, "sw") || !strcmp(opcode, "beq"))
        {
            k = Itype(opcode, arg0, arg1, arg2, line);
        }
        else if (!strcmp(opcode, "jalr"))
        {
            k = Jtype(arg0, arg1);
        }
        else if (!strcmp(opcode, "halt") || !strcmp(opcode, "noop"))
        {
            k = Otype(opcode);
        }
        else if (!strcmp(opcode, ".fill"))
        {
            if (isNumber(arg0))
            {
                k = atoi(arg0);
            }
            else
            {
                int check = 0;
                for (int i = 0; i < idx; i++)
                {
                    if (!strcmp(Label[i].name, arg0))
                    {
                        k = Label[i].addr;
                        check = 1;
                        break;
                    }
                }

                if (check == 0)
                {
                    printf("Error1: Use of undefined labels");
                    exit(1);
                }
            }
        }
        else
        {
            printf("Error4: Unrecognized opcodes");
            exit(1);
        }
        fprintf(outFilePtr, "%d\n", k);
        printf("(address %d): %d\n", line, k);

        line++;
    }
    if (inFilePtr)
    {
        fclose(inFilePtr);
    }
    if (outFilePtr)
    {
        fclose(outFilePtr);
    }
    exit(0);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
                 char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL)
    {
        /* reached end of file */
        return (0);
    }

    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL)
    {
        /* line too long */
        printf("error: line too long\n");
        exit(1);
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label))
    {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%"
                "[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
           opcode, arg0, arg1, arg2);
    return (1);
}

int isNumber(char *string)
{
    /* return 1 if string is a number */
    int i;
    return ((sscanf(string, "%d", &i)) == 1);
}

void checkReg(char *reg)
{
    if (!isNumber(reg))
    {
        printf("Error5: None-integer register arguments");
        exit(1);
    }
    if (atoi(reg) < 0 || atoi(reg) > 7)
    {
        printf("Error6: Registers outside the range [0,7]");
        exit(1);
    }
}

int Rtype(char *opcode, char *regA, char *regB, char *destReg)
{
    int k = 0;

    checkReg(regA);
    checkReg(regB);
    checkReg(destReg);

    if (!strcmp(opcode, "add"))
    {
        k = (0 << 22);
    }
    else if (!strcmp(opcode, "nor"))
    {
        k = (1 << 22);
    }

    k |= atoi(regA) << 19;
    k |= atoi(regB) << 16;
    k |= atoi(destReg);

    return k;
}

int Itype(char *opcode, char *regA, char *regB, char *offset, int pc)
{
    int k = 0;
    int address = 0;

    checkReg(regA);
    checkReg(regB);

    if (!strcmp(opcode, "lw"))
    {
        k = (2 << 22);
    }
    else if (!strcmp(opcode, "sw"))
    {
        k = (3 << 22);
    }
    else if (!strcmp(opcode, "beg"))
    {
        k = (4 << 22);
    }

    k |= atoi(regA) << 19;
    k |= atoi(regB) << 16;

    // offsetfield
    if (isNumber(offset))
    {
        address = atoi(offset);
    }
    else
    {
        int check = 0;
        for (int i = 0; i < idx; i++)
        {
            if (!strcmp(Label[i].name, offset))
            {
                address = Label[i].addr;
                check = 1;
                break;
            }
        }

        if (check == 0)
        {
            printf("Error1: Use of undefined labels");
            exit(1);
        }

        if (!strcmp(opcode, "beq"))
        {
            address = address - pc - 1;
        }
    }

    if (address > 32787 || address < -32768)
    {
        printf("Error3: offsetFields that does not fit in 16 bits ");
        exit(1);
    }

    address &= 0xFFFF;
    k |= address;

    return k;
}

int Jtype(char *regA, char *regB)
{
    int k = 0;

    checkReg(regA);
    checkReg(regB);

    k = 5 << 22;
    k |= atoi(regA) << 19;
    k |= atoi(regB) << 16;

    return k;
}

int Otype(char *opcode)
{
    int k = 0;

    if (!strcmp(opcode, "halt"))
    {
        k = 6 << 22;
    }
    else if (!strcmp(opcode, "noop"))
    {
        k = 7 << 22;
    }

    return k;
}
