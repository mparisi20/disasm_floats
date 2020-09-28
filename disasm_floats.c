#if 0

disassemble a float table with the specified file offset and size
./disasm_floats DOL, offset, size, name, floats_per_line

#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <inttypes.h>

typedef uint32_t u32;
typedef uint8_t u8;

static unsigned long parseHex(const char *hexStr)
{
    char *endptr;
    errno = 0;
    unsigned long result = strtoul(hexStr, &endptr, 16);
    if (errno == ERANGE) {
        fprintf(stderr, "ERROR: '%s' is out of range\n", hexStr);
        exit(EXIT_FAILURE);
    } else if (endptr == hexStr || *endptr != '\0')  {
        fprintf(stderr, "ERROR: '%s' did not parse as a 32-bit hex number\n", hexStr);
        exit(EXIT_FAILURE);
    }
    return result;
}

static const size_t DOL_SECTIONS = 18;
typedef struct SectionInfo
{
    unsigned offset;
    unsigned addr;
    unsigned len;
} SectionInfo;

static void tryRead(FILE *fp, long offset, void *ptr, size_t size, size_t nmemb)
{
    fseek(fp, offset, SEEK_SET);
    if (fread(ptr, size, nmemb, fp) != nmemb) {
        fprintf(stderr, "fread error\n");
        exit(EXIT_FAILURE);
    }
}

static u32 be32_to_cpu(const u8 *buf) {
    return (u32)buf[3] | (u32)buf[2] << 8 | (u32)buf[1] << 16 | (u32)buf[0] << 24;
}

static float befloat_to_cpu(const u8 *buf) {
   float retVal;
   char *retValBytePtr = (char *)&retVal;

   retValBytePtr[0] = buf[3];
   retValBytePtr[1] = buf[2];
   retValBytePtr[2] = buf[1];
   retValBytePtr[3] = buf[0];

   return retVal;
}

static const size_t OFFSETS = 0x0;
static const size_t ADDRS = 0x48;
static const size_t LENS = 0x90;
static void fillSectionInfoTab(FILE *fp, SectionInfo *tab)
{
    u32 offset, addr, len;
    for (size_t i = 0; i < DOL_SECTIONS; i++) {
        tryRead(fp, OFFSETS + i*sizeof(u32), &offset, sizeof(u32), 1);        
        tab[i].offset = be32_to_cpu((const u8 *)&offset);
        
        tryRead(fp, ADDRS + i*sizeof(u32), &addr, sizeof(u32), 1);
        tab[i].addr = be32_to_cpu((const u8 *)&addr);

        tryRead(fp, LENS + i*sizeof(u32), &len, sizeof(u32), 1);
        tab[i].len = be32_to_cpu((const u8 *)&len);
    }
}

static unsigned long getFileOffset(FILE *fp, unsigned long absAddr)
{
    rewind(fp);
    SectionInfo tab[DOL_SECTIONS];
    fillSectionInfoTab(fp, tab);
    
    // figure out what section this address is in 
    int foundIdx = -1;
    bool found = false;
    for (size_t i = 0; i < DOL_SECTIONS; i++) {
        if (absAddr >= tab[i].addr && absAddr < tab[i].addr + tab[i].len) {
            if (found) {
                fprintf(stderr, "ERROR: found more than one section containing %08lx\n", absAddr);
                exit(EXIT_FAILURE);
            }
            foundIdx = i;
            found = true;
        }
    }
    if (!found) {
        fprintf(stderr, "ERROR: could not find absolute address 0x%08lx in any of the DOL sections\n", absAddr);
        exit(EXIT_FAILURE);
    }
    return absAddr - tab[foundIdx].addr + tab[foundIdx].offset;
}

int main(int argc, char *argv[])
{
    if (argc != 6) {
        fprintf(stderr, "usage: ./disasm_floats <dol_file>, <abs_address>, <size>, <table_name>, <floats_per_line>\n"
                        "expects all numbers to be in hexadecimal\n");
        return EXIT_FAILURE;
    }
    
    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        fprintf(stderr, "ERROR: failed to open file '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }
    unsigned long fileOffset = getFileOffset(fp, parseHex(argv[2]));
    unsigned long size = parseHex(argv[3]);
    const char *tableName = argv[4];
    unsigned long perLine = parseHex(argv[5]);
    if (size % sizeof(float) != 0) {
        fprintf(stderr, "ERROR: the given size is not a multiple of %lu\n", sizeof(float));
        return EXIT_FAILURE;
    }
    if (perLine == 0) {
        fprintf(stderr, "ERROR: floats_per_line cannot be 0\n");
        return EXIT_FAILURE;
    }
    
    size_t numLines = size / sizeof(float) / perLine;
    size_t leftover = (size / sizeof(float)) % perLine;
    size_t off = fileOffset; // current file offset as the table is read from the DOL
    
    printf(".global %s\n", tableName);
    printf("%s:\n", tableName);
    for (size_t i = 0; i < numLines; i++) {
        printf("    .float ");
        // number of prints per line, with a special case for the last line
        size_t iterations = (leftover == 0 || i != numLines - 1) ? perLine : leftover;
        for (size_t j = 0; j < iterations; j++) {
            float val;
            tryRead(fp, off, &val, sizeof(float), 1);
            val = befloat_to_cpu((const u8 *)&val);
            printf("%.6f", val);
            off += sizeof(float);
            if (j < iterations-1)
                printf(", ");
        }
        putchar('\n');
    }
    return 0;
}
