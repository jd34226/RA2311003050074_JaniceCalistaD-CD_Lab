/*
 * Lab 12 - Simple Code Generator
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#define MAX_INSTRS  50
#define MAX_VARS    30
#define NUM_REGS     4   /* R0 .. R3 */

/* ---- TAC Instruction ---- */
typedef struct {
    char op[5];    /* +, -, *, /, =, IFLT, GOTO, etc. */
    char res[10];
    char arg1[10];
    char arg2[10];
} TAC;

TAC tac[MAX_INSTRS]; int tac_cnt = 0;

/* ---- Descriptors ---- */
/* Register descriptor: what variable is in each register */
char reg_desc[NUM_REGS][10];
int  reg_in_use[NUM_REGS];

/* Address descriptor: where is each variable? (register or memory) */
typedef struct {
    char name[10];
    int  reg;       /* -1 = memory only */
    int  in_mem;    /* also in memory */
} AddrDesc;

AddrDesc addr[MAX_VARS]; int addr_cnt = 0;

/* ---- Generated Code ---- */
char code[MAX_INSTRS*3][80]; int code_cnt = 0;

void emit(char *fmt, ...) {
    char buf[80];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 80, fmt, args);
    va_end(args);
    strcpy(code[code_cnt++], buf);
    printf("  %s\n", buf);
}

void initRegs() {
    for(int i=0;i<NUM_REGS;i++) { reg_desc[i][0]='\0'; reg_in_use[i]=0; }
}

int findAddr(char *name) {
    for(int i=0;i<addr_cnt;i++) if(strcmp(addr[i].name,name)==0) return i;
    return -1;
}

AddrDesc *getAddr(char *name) {
    int idx=findAddr(name);
    if(idx>=0) return &addr[idx];
    strcpy(addr[addr_cnt].name,name);
    addr[addr_cnt].reg=-1;
    addr[addr_cnt].in_mem=1;
    return &addr[addr_cnt++];
}

/* Get a free register, or spill LRU */
int getFreeReg(char *for_var) {
    /* Find free */
    for(int i=0;i<NUM_REGS;i++) if(!reg_in_use[i]) return i;

    /* Spill: find reg holding a var not needed soon (simplified: pick R0) */
    int spill = 0;
    if(reg_desc[spill][0] != '\0') {
        /* spill to memory */
        emit("ST  R%d, %s", spill, reg_desc[spill]);
        AddrDesc *ad = getAddr(reg_desc[spill]);
        ad->reg = -1; ad->in_mem = 1;
    }
    reg_in_use[spill]=0; reg_desc[spill][0]='\0';
    return spill;
}

int getReg(char *var) {
    /* Already in a register? */
    AddrDesc *ad = getAddr(var);
    if(ad->reg >= 0) return ad->reg;

    /* Allocate new */
    int r = getFreeReg(var);
    emit("LD  R%d, %s", r, var);
    reg_in_use[r]=1; strcpy(reg_desc[r],var);
    ad->reg=r; return r;
}

void assignReg(char *var, int r) {
    /* Update descriptors */
    for(int i=0;i<NUM_REGS;i++) {
        if(i!=r && strcmp(reg_desc[i],var)==0) { reg_desc[i][0]='\0'; reg_in_use[i]=0; }
    }
    reg_in_use[r]=1; strcpy(reg_desc[r],var);
    AddrDesc *ad=getAddr(var); ad->reg=r; ad->in_mem=0;
}

char opToAsm(char op) {
    switch(op) { case '+': return 'A'; case '-': return 'S'; case '*': return 'M'; case '/': return 'D'; }
    return '?';
}

void generateCode() {
    initRegs();
    printf("\n=== Generated Assembly Code ===\n");
    for(int i=0;i<tac_cnt;i++) {
        TAC *t=&tac[i];
        printf("; %s = %s %s %s\n", t->res, t->arg1, t->op, t->arg2);

        if(strcmp(t->op,"=")==0) {
            /* Simple assignment: res = arg1 */
            int r=getReg(t->arg1);
            assignReg(t->res, r);
            emit("LD  R%d, %s", r, t->arg1);
            emit("ST  R%d, %s", r, t->res);
        } else if(strlen(t->op)==1 && strchr("+-*/",t->op[0])) {
            int r1=getReg(t->arg1);
            int r2=getReg(t->arg2);
            int rd=getFreeReg(t->res);
            char mn[5]; snprintf(mn,5,"%-3cD",opToAsm(t->op[0]));
            /* MOV rd <- r1, then op */
            emit("MOV R%d, R%d", rd, r1);
            emit("%c%c  R%d, R%d", t->op[0]=='+'?'A':t->op[0]=='-'?'S':t->op[0]=='*'?'M':'D',
                 'D', rd, r2);
            (void)mn;
            assignReg(t->res, rd);
            emit("ST  R%d, %s", rd, t->res);
        } else if(strcmp(t->op,"PRINT")==0) {
            int r=getReg(t->arg1);
            emit("OUT R%d", r);
        }
        printf("\n");
    }

    /* Flush remaining regs to memory */
    printf("; --- End: flush registers ---\n");
    for(int i=0;i<NUM_REGS;i++) {
        if(reg_in_use[i] && reg_desc[i][0]) {
            emit("ST  R%d, %s", i, reg_desc[i]);
        }
    }
}

void sampleTAC() {
    /* Sample: a = b + c * d - e */
    /* TAC:
       t1 = c * d
       t2 = b + t1
       t3 = t2 - e
       a  = t3
    */
    tac_cnt=0;
    strcpy(tac[tac_cnt].op,"*"); strcpy(tac[tac_cnt].arg1,"c");
    strcpy(tac[tac_cnt].arg2,"d"); strcpy(tac[tac_cnt].res,"t1"); tac_cnt++;

    strcpy(tac[tac_cnt].op,"+"); strcpy(tac[tac_cnt].arg1,"b");
    strcpy(tac[tac_cnt].arg2,"t1"); strcpy(tac[tac_cnt].res,"t2"); tac_cnt++;

    strcpy(tac[tac_cnt].op,"-"); strcpy(tac[tac_cnt].arg1,"t2");
    strcpy(tac[tac_cnt].arg2,"e"); strcpy(tac[tac_cnt].res,"t3"); tac_cnt++;

    strcpy(tac[tac_cnt].op,"="); strcpy(tac[tac_cnt].arg1,"t3");
    strcpy(tac[tac_cnt].arg2,""); strcpy(tac[tac_cnt].res,"a"); tac_cnt++;
}

void customTAC() {
    printf("Enter TAC instructions (format: res = arg1 op arg2, or 'end' to finish)\n");
    printf("Example: t1 = b + c\n");
    tac_cnt=0;
    char line[100];
    while(1) {
        printf("Instr %d: ",tac_cnt+1);
        fgets(line,100,stdin); line[strcspn(line,"\n")]='\0';
        if(strcmp(line,"end")==0) break;
        char res[10],eq[3],arg1[10],op[5],arg2[10];
        int n=sscanf(line,"%s %s %s %s %s",res,eq,arg1,op,arg2);
        if(n<3) break;
        strcpy(tac[tac_cnt].res,res);
        strcpy(tac[tac_cnt].arg1,arg1);
        if(n>=5) { strcpy(tac[tac_cnt].op,op); strcpy(tac[tac_cnt].arg2,arg2); }
        else     { strcpy(tac[tac_cnt].op,"="); strcpy(tac[tac_cnt].arg2,""); }
        tac_cnt++;
    }
}

int main() {
    printf("=== Simple Code Generator ===\n");
    printf("Register Machine with %d registers (R0-R%d)\n\n", NUM_REGS, NUM_REGS-1);

    printf("1. Use sample TAC (a = b + c*d - e)\n");
    printf("2. Enter custom TAC\n");
    printf("Choice: "); int ch; scanf("%d",&ch); getchar();

    if(ch==1) {
        sampleTAC();
        printf("\n=== Three-Address Code (Input) ===\n");
        for(int i=0;i<tac_cnt;i++) {
            if(strlen(tac[i].arg2)>0)
                printf("  %s = %s %s %s\n",tac[i].res,tac[i].arg1,tac[i].op,tac[i].arg2);
            else
                printf("  %s = %s\n",tac[i].res,tac[i].arg1);
        }
    } else {
        customTAC();
    }

    generateCode();

    printf("\n=== Register State at End ===\n");
    for(int i=0;i<NUM_REGS;i++) {
        if(reg_desc[i][0]) printf("  R%d = %s\n",i,reg_desc[i]);
        else                printf("  R%d = (free)\n",i);
    }
    return 0;
}
