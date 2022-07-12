#include <stdio.h>
#include "scanner.h"
#include "parser.h"

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        FILE *f = fopen(argv[1], "rb");
        fseek(f, 0, SEEK_END);
        int srcLen = ftell(f);
        rewind(f);
        char *src = (char*)malloc(srcLen+1);
        size_t last = fread(src, 1, srcLen+1, f);
        src[last] = '\0';
        fclose(f);

        initScan(src);
        /*SymbolType type = scanNext();
        while (type != T_EOF) {
            printf("%d ", type);
            type = scanNext();
        }
        putchar('\n');*/
        parse();
    } else {
        printf("Usage: %s <source file> [log file]\n", argv[0]);
    }
    return 0;
}