/*
 * Lab 11 - Intermediate code generation – Quadruple, Triple, Indirect triple
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX 100

/* ---- Quadruple: (op, arg1, arg2, result) ---- */
typedef struct {
    char op[10];
    char arg1[10];
    char arg2[10];
    char result[10];
} Quadruple;

/* ---- Triple: (op, arg1, arg2) — result is the index itself ---- */
typedef struct {
    char op[10];
    char arg1[10];
    char arg2[10];
} Triple;

/* ---- Indirect Triple: list of pointers to triple table ---- */

Quadruple quads[MAX]; int quad_cnt = 0;
Triple    trips[MAX]; int trip_cnt = 0;
int       indirect[MAX]; int ind_cnt = 0;

int temp_q = 1, temp_t = 1;

char newTempQ() { static char buf[10]; snprintf(buf,10,"t%d",temp_q++); return buf[0]; }
char *newTempQStr() { static char buf[10]; snprintf(buf,10,"t%d",temp_q++); return buf; }
char *newTempTStr() { static char buf[10]; snprintf(buf,10,"t%d",temp_t++); return buf; }

/* ========== Infix -> Postfix for parsing ========== */
char cstack[MAX]; int ctop=-1;
void cpush(char c) { cstack[++ctop]=c; }
char cpop()        { return cstack[ctop--]; }
char cpeek()       { return ctop>=0?cstack[ctop]:'\0'; }
int  cempty()      { return ctop<0; }
int prec(char op)  { return (op=='+'||op=='-')?1:(op=='*'||op=='/')?2:0; }
int isOp(char c)   { return c=='+'||c=='-'||c=='*'||c=='/'; }

void infixToPostfix(char *in, char *out) {
    int pi=0; ctop=-1;
    for(int i=0;in[i];i++) {
        char c=in[i]; if(isspace(c)) continue;
        if(isalnum(c)) { out[pi++]=c; }
        else if(c=='(') cpush(c);
        else if(c==')') { while(!cempty()&&cpeek()!='(') out[pi++]=cpop(); if(!cempty()) cpop(); }
        else if(isOp(c)) { while(!cempty()&&cpeek()!='('&&prec(cpeek())>=prec(c)) out[pi++]=cpop(); cpush(c); }
    }
    while(!cempty()) out[pi++]=cpop();
    out[pi]='\0';
}

/* ========== Generate all three representations ========== */
void generateAll(char *infix) {
    char postfix[MAX];
    infixToPostfix(infix, postfix);

    /* Reset */
    quad_cnt=0; trip_cnt=0; ind_cnt=0; temp_q=1; temp_t=1;

    /* Stack of operand strings for quads */
    char qstack[MAX][10]; int qs=-1;
    /* Stack of operand strings for triples (either name or "(n)" index ref) */
    char tstack[MAX][10]; int ts=-1;

    printf("\n=== Processing: %s ===\n", infix);
    printf("Postfix: %s\n\n", postfix);

    for(int i=0;postfix[i];i++) {
        char c=postfix[i];
        if(isalnum(c)) {
            /* Push operand */
            char buf[10]={c,'\0'};
            strcpy(qstack[++qs],buf);
            strcpy(tstack[++ts],buf);
        } else if(isOp(c)) {
            /* ---- Quadruple ---- */
            char arg2q[10],arg1q[10],resq[10];
            strcpy(arg2q,qstack[qs--]);
            strcpy(arg1q,qstack[qs--]);
            snprintf(resq,10,"t%d",temp_q++);
            char opstr[3]={c,'\0'};
            strcpy(quads[quad_cnt].op,opstr);
            strcpy(quads[quad_cnt].arg1,arg1q);
            strcpy(quads[quad_cnt].arg2,arg2q);
            strcpy(quads[quad_cnt].result,resq);
            quad_cnt++;
            strcpy(qstack[++qs],resq);

            /* ---- Triple ---- */
            char arg2t[10],arg1t[10];
            strcpy(arg2t,tstack[ts--]);
            strcpy(arg1t,tstack[ts--]);
            strcpy(trips[trip_cnt].op,opstr);
            strcpy(trips[trip_cnt].arg1,arg1t);
            strcpy(trips[trip_cnt].arg2,arg2t);
            /* result is implicit: (trip_cnt) */
            char ref[10]; snprintf(ref,10,"(%d)",trip_cnt);
            strcpy(tstack[++ts],ref);
            indirect[ind_cnt++]=trip_cnt;
            trip_cnt++;
        }
    }
}

void printQuadruples() {
    printf("=== Quadruple Table ===\n");
    printf("%-6s %-8s %-8s %-8s %-10s\n","Index","Op","Arg1","Arg2","Result");
    printf("-----------------------------------------------\n");
    for(int i=0;i<quad_cnt;i++)
        printf("%-6d %-8s %-8s %-8s %-10s\n",i,quads[i].op,quads[i].arg1,quads[i].arg2,quads[i].result);
}

void printTriples() {
    printf("\n=== Triple Table ===\n");
    printf("%-6s %-8s %-8s %-8s\n","Index","Op","Arg1","Arg2");
    printf("---------------------------------\n");
    for(int i=0;i<trip_cnt;i++)
        printf("%-6d %-8s %-8s %-8s\n",i,trips[i].op,trips[i].arg1,trips[i].arg2);
}

void printIndirectTriples() {
    printf("\n=== Indirect Triple ===\n");
    printf("%-10s %-6s\n","Statement","Points to Triple");
    printf("------------------------\n");
    for(int i=0;i<ind_cnt;i++)
        printf("%-10d -> (%d): %-4s %-6s %-6s\n",
               i, indirect[i], trips[indirect[i]].op,
               trips[indirect[i]].arg1, trips[indirect[i]].arg2);
}

void printComparison() {
    printf("\n=== Comparison ===\n");
    printf("Quadruples : stores result name explicitly => more space, easy for code optimization\n");
    printf("Triples    : result is the index => less space, but hard to reorder\n");
    printf("Ind. Triple: uses pointer list => easy to reorder while keeping triple table intact\n");
}

int main() {
    printf("=== Intermediate Code: Quadruple / Triple / Indirect Triple ===\n\n");
    char infix[MAX], cont='y';
    while(cont=='y'||cont=='Y') {
        printf("Enter infix expression (single-char operands, e.g. a+b*c-d): ");
        scanf(" "); fgets(infix,MAX,stdin);
        infix[strcspn(infix,"\n")]='\0';

        generateAll(infix);
        printQuadruples();
        printTriples();
        printIndirectTriples();
        printComparison();

        printf("\nAnother? (y/n): "); scanf(" %c",&cont);
    }
    return 0;
}
