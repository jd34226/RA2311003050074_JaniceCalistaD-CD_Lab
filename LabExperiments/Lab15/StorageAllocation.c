/*
 * Lab 15 - Implement any one storage allocation strategies (heap, stack, static)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MEM_SIZE 1024   /* bytes */

/* ============================================================
 * STATIC ALLOCATION
 * All memory is allocated at compile time; size known ahead
 * ============================================================ */

#define STATIC_SIZE 256
char static_mem[STATIC_SIZE];
int  static_used = 0;

typedef struct { char name[20]; int offset; int size; } StaticVar;
StaticVar static_vars[20]; int static_var_cnt = 0;

int staticAlloc(char *name, int size) {
    if(static_used + size > STATIC_SIZE) {
        printf("  [STATIC] Out of memory for %s!\n", name);
        return -1;
    }
    strcpy(static_vars[static_var_cnt].name, name);
    static_vars[static_var_cnt].offset = static_used;
    static_vars[static_var_cnt].size   = size;
    static_var_cnt++;
    int off = static_used;
    static_used += size;
    printf("  [STATIC] Allocated '%s' at offset %d, size %d bytes\n", name, off, size);
    return off;
}

void staticWrite(int offset, int value) {
    if(offset + 4 > STATIC_SIZE) { printf("  [STATIC] Write out of bounds!\n"); return; }
    *((int*)(static_mem + offset)) = value;
}

int staticRead(int offset) {
    if(offset + 4 > STATIC_SIZE) { printf("  [STATIC] Read out of bounds!\n"); return -1; }
    return *((int*)(static_mem + offset));
}

void printStaticMap() {
    printf("\n  Static Memory Map:\n");
    printf("  %-15s %-10s %-10s\n","Variable","Offset","Size");
    printf("  ------------------------------------\n");
    for(int i=0;i<static_var_cnt;i++)
        printf("  %-15s %-10d %-10d\n",static_vars[i].name,static_vars[i].offset,static_vars[i].size);
    printf("  Total used: %d / %d bytes\n", static_used, STATIC_SIZE);
}

/* ============================================================
 * STACK ALLOCATION
 * LIFO — activation records pushed/popped on procedure call
 * ============================================================ */

#define STACK_SIZE 256
char stack_mem[STACK_SIZE];
int  sp = 0;  /* stack pointer: grows upward */

typedef struct {
    char func_name[20];
    int  base;       /* base pointer for this frame */
    struct { char name[20]; int offset; int size; } vars[10];
    int  var_cnt;
} ActivationRecord;

ActivationRecord ar_stack[20]; int ar_top = -1;

void stackPush(char *func_name) {
    ar_top++;
    strcpy(ar_stack[ar_top].func_name, func_name);
    ar_stack[ar_top].base = sp;
    ar_stack[ar_top].var_cnt = 0;
    printf("  [STACK] Pushing activation record for '%s' at base %d\n", func_name, sp);
}

int stackAllocVar(char *name, int size) {
    if(ar_top < 0) { printf("  [STACK] No active frame!\n"); return -1; }
    if(sp + size > STACK_SIZE) { printf("  [STACK] Stack overflow!\n"); return -1; }
    ActivationRecord *ar = &ar_stack[ar_top];
    int off = sp - ar->base;
    strcpy(ar->vars[ar->var_cnt].name, name);
    ar->vars[ar->var_cnt].offset = off;
    ar->vars[ar->var_cnt].size   = size;
    ar->var_cnt++;
    printf("  [STACK] '%s': local var '%s' at offset %d (abs: %d)\n", ar->func_name, name, off, sp);
    sp += size;
    return sp - size;
}

void stackPop() {
    if(ar_top < 0) { printf("  [STACK] Stack underflow!\n"); return; }
    ActivationRecord *ar = &ar_stack[ar_top];
    printf("  [STACK] Popping activation record for '%s', freeing %d bytes\n",
           ar->func_name, sp - ar->base);
    sp = ar->base;
    ar_top--;
}

void printStackState() {
    printf("\n  Stack State (SP = %d):\n", sp);
    for(int i=0;i<=ar_top;i++) {
        ActivationRecord *ar = &ar_stack[i];
        printf("  Frame '%s' (base=%d):\n", ar->func_name, ar->base);
        for(int j=0;j<ar->var_cnt;j++)
            printf("    %-15s offset=%-5d size=%d\n",ar->vars[j].name,ar->vars[j].offset,ar->vars[j].size);
    }
}

/* ============================================================
 * HEAP ALLOCATION
 * Free-store — malloc/free, non-LIFO
 * Uses a simple free-list allocator
 * ============================================================ */

#define HEAP_SIZE 512
char heap_mem[HEAP_SIZE];

typedef struct Block { int size; int free; struct Block *next; } HeapBlock;

HeapBlock *heap_head = NULL;

void heapInit() {
    heap_head = (HeapBlock*)heap_mem;
    heap_head->size = HEAP_SIZE - sizeof(HeapBlock);
    heap_head->free = 1;
    heap_head->next = NULL;
    printf("  [HEAP] Initialized: %d bytes available\n", heap_head->size);
}

void *heapMalloc(int size) {
    HeapBlock *cur = heap_head;
    while(cur) {
        if(cur->free && cur->size >= size + (int)sizeof(HeapBlock)) {
            /* Split block */
            HeapBlock *newb = (HeapBlock*)((char*)cur + sizeof(HeapBlock) + size);
            newb->size = cur->size - size - sizeof(HeapBlock);
            newb->free = 1;
            newb->next = cur->next;
            cur->size  = size;
            cur->free  = 0;
            cur->next  = newb;
            int offset = (int)((char*)cur - heap_mem);
            printf("  [HEAP] Allocated %d bytes at heap offset %d\n", size, offset);
            return (char*)cur + sizeof(HeapBlock);
        }
        cur = cur->next;
    }
    printf("  [HEAP] Out of heap memory!\n");
    return NULL;
}

void heapFree(void *ptr) {
    HeapBlock *blk = (HeapBlock*)((char*)ptr - sizeof(HeapBlock));
    blk->free = 1;
    int offset = (int)((char*)blk - heap_mem);
    printf("  [HEAP] Freed %d bytes at heap offset %d\n", blk->size, offset);

    /* Coalesce adjacent free blocks */
    HeapBlock *cur = heap_head;
    while(cur && cur->next) {
        if(cur->free && cur->next->free) {
            cur->size += sizeof(HeapBlock) + cur->next->size;
            cur->next = cur->next->next;
        } else cur = cur->next;
    }
}

void printHeapState() {
    printf("\n  Heap State:\n");
    HeapBlock *cur = heap_head;
    int idx = 0;
    while(cur) {
        printf("  Block %d: offset=%d size=%d %s\n",
               idx++, (int)((char*)cur-heap_mem), cur->size, cur->free?"FREE":"USED");
        cur = cur->next;
    }
}

/* ============================================================
 * DEMONSTRATION
 * ============================================================ */

void demoStatic() {
    printf("\n========== STATIC ALLOCATION ==========\n");
    printf("All variables allocated at compile time\n\n");
    int a = staticAlloc("globalA", 4);
    int b = staticAlloc("globalB", 4);
    int arr = staticAlloc("array[10]", 40);
    staticWrite(a, 100);
    staticWrite(b, 200);
    printf("  Written: globalA=%d, globalB=%d\n", staticRead(a), staticRead(b));
    (void)arr;
    printStaticMap();
}

void demoStack() {
    printf("\n========== STACK ALLOCATION ==========\n");
    printf("Simulating function call chain: main->foo->bar\n\n");
    stackPush("main");
    stackAllocVar("x", 4);
    stackAllocVar("y", 4);

    stackPush("foo");
    stackAllocVar("a", 4);
    stackAllocVar("b", 8);

    stackPush("bar");
    stackAllocVar("temp", 4);

    printStackState();

    printf("\nReturning from bar...\n");
    stackPop();
    printf("Returning from foo...\n");
    stackPop();
    printf("Returning from main...\n");
    stackPop();
    printf("\n  SP after all returns: %d (memory fully reclaimed)\n", sp);
}

void demoHeap() {
    printf("\n========== HEAP ALLOCATION ==========\n");
    printf("Dynamic allocation with malloc/free simulation\n\n");
    heapInit();

    void *p1 = heapMalloc(50);
    void *p2 = heapMalloc(100);
    void *p3 = heapMalloc(75);

    printHeapState();

    printf("\nFreeing p2 (middle block)...\n");
    heapFree(p2);
    printHeapState();

    printf("\nFreeing p1...\n");
    heapFree(p1);
    printf("Coalescing adjacent free blocks...\n");
    printHeapState();

    printf("\nAllocating 120 bytes (should reuse freed space)...\n");
    void *p4 = heapMalloc(120);
    printHeapState();

    heapFree(p3); heapFree(p4);
    printf("\nAll memory freed.\n");
    printHeapState();
}

int main() {
    printf("=== Storage Allocation Strategies ===\n");
    printf("1. Static Allocation\n");
    printf("2. Stack Allocation\n");
    printf("3. Heap Allocation\n");
    printf("4. All three\n");
    printf("Choice: "); int ch; scanf("%d",&ch);

    if(ch==1||ch==4) demoStatic();
    if(ch==2||ch==4) demoStack();
    if(ch==3||ch==4) demoHeap();

    printf("\n=== Summary ===\n");
    printf("Static : Fixed at compile time. Fast access. No runtime overhead.\n");
    printf("Stack  : LIFO. Auto-managed via call/return. Supports recursion.\n");
    printf("Heap   : Dynamic. Flexible. Requires explicit malloc/free. Fragmentation risk.\n");
    return 0;
}
