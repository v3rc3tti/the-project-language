#include <stdint.h>
#include <stdbool.h>
#include "interpreter.h"

static int32_t store[MAX_STORE];
static int pc;
static int bp;
static int sp;
static int32_t stackBottom;
static bool isRunning;

static void error(int lineNo, const char *text) {
    printf("%d: %s\n", lineNo, text);
    isRunning = false;
}

static void allocate(int wordCount) {
    sp = sp + wordCount;
    if (sp > MAX_STORE) {
        printf("Stack Overflow\n");
        isRunning = false;
    }
}

static void opVariable(int level, int disp) {
    allocate(1);
    int x = bp;
    while (level > 0) {
        x = store[x];
        level--;
    }
    store[sp] = x + disp;
    pc += 3;
}

static void opIndex(int bound, int lineNo) {
    int i = store[sp];
    sp--;
    if (i < 1 || i > bound) {
        error(lineNo, "Range Error");
    } else {
        store[sp] = store[sp] + i - 1;
    }
    pc += 3;
}

static void opConstant(int value) {
    allocate(1);
    store[sp] = value;
    pc += 2;
}

static void opValue() {
    store[sp] = store[store[sp]];
    pc++;
}

static void opNot() {
    store[sp] = 1 - store[sp];
    p++;
}

static void opMultiply() {
    pc++;
    sp--;
    store[sp] = store[sp] * store[sp + 1];
}

static void opDivide() {
    pc++;
    sp--;
    store[sp] = store[sp] / store[sp + 1];
}

static void opModulo() {
    pc++;
    sp--;
    store[sp] = store[sp] % store[sp + 1];
}

static void opMinus() {
    store[sp] = -store[sp];
    pc++;
}

static void opAdd() {
    pc++;
    sp--;
    store[sp] = store[sp] + store[sp + 1];
}

static void opSubtract() {
    pc++;
    sp--;
    store[sp] = store[sp] - store[sp + 1];
}

static void opLess() {
    pc++;
    sp--;
    store[sp] = (store[sp] < store[sp + 1]) ? 1 : 0;
}

static void opEqual() {
    pc++;
    sp--;
    store[sp] = (store[sp] == store[sp + 1]) ? 1 : 0;
}

static void opGreater() {
    pc++;
    sp--;
    store[sp] = (store[sp] > store[sp + 1]) ? 1 : 0;
}

static void opAnd() {
    pc++;
    sp--;
    if (store[sp] == 1) {
        store[sp] = store[sp + 1];
    }
}

static void opOr() {
    pc++;
    sp--;
    if (store[sp] == 0) {
        store[sp] = store[sp + 1];
    }        
}

static void opRead(int num) {
    pc += 2;
    sp = sp - num;
    int x = sp;
    while (x < sp + num) {
        x++;
        scanf("%d", &store[store[x]]);
    }
}    

static void opWrite(int num) {
    pc += 2;
    sp = sp - num;
    int x = sp;
    while (x < sp + num) {
        x++;
        printf("%d\n", store[x]);
    }
}

static void opAssign(int num) {
    pc += 2;
    sp = sp - 2 * num;
    int x = sp;
    while (x < sp + num) {
        x++;
        store[store[x]] = store[x + num];
    }
}

static void opCall(int level, int addr) {
    allocate(3);
    int x = bp;
    while (level > 0) {
        x = store[x];
        level = level - 1;
    }
    store[sp - 2] = x;
    store[sp - 1] = bp;
    store[sp] = pc + 3;
    bp = sp - 2;
    pc = addr;
}
