/*
 * Lab 7 - Shift Reduce Parsing
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX  100

/* Productions */
char *prods[] = {
    "E->E+E",
    "E->E*E",
    "E->(E)",
    "E->id"
};
int prod_count = 4;

/* Stack */
char stk[MAX][20];
int  sp = 0;

void push(char *s) { strcpy(stk[sp++], s); }
void pop_n(int n)  { sp -= n; }
char *top()        { return sp>0 ? stk[sp-1] : ""; }

void printStack() {
    printf("[ ");
    for(int i=0;i<sp;i++) printf("%s ",stk[i]);
    printf("]");
}

/* Try to reduce top of stack */
int reduce() {
    /* E -> id */
    if(sp>=1 && strcmp(stk[sp-1],"id")==0) {
        pop_n(1); push("E");
        printf(" => Reduce by E->id");
        return 1;
    }
    /* E -> E+E */
    if(sp>=3 && strcmp(stk[sp-1],"E")==0 && strcmp(stk[sp-2],"+")==0 && strcmp(stk[sp-3],"E")==0) {
        pop_n(3); push("E");
        printf(" => Reduce by E->E+E");
        return 1;
    }
    /* E -> E*E */
    if(sp>=3 && strcmp(stk[sp-1],"E")==0 && strcmp(stk[sp-2],"*")==0 && strcmp(stk[sp-3],"E")==0) {
        pop_n(3); push("E");
        printf(" => Reduce by E->E*E");
        return 1;
    }
    /* E -> (E) */
    if(sp>=3 && strcmp(stk[sp-1],")")==0 && strcmp(stk[sp-2],"E")==0 && strcmp(stk[sp-3],"(")==0) {
        pop_n(3); push("E");
        printf(" => Reduce by E->(E)");
        return 1;
    }
    return 0;
}

/* Tokenize input */
int token_count;
char tokens[MAX][20];
int  tok_idx;

void tokenize(char *input) {
    token_count = 0; tok_idx = 0;
    int i = 0, len = strlen(input);
    while(i < len) {
        if(isspace(input[i])) { i++; continue; }
        if(isalpha(input[i])) {
            char buf[20]="";
            while(i<len && isalnum(input[i])) { strncat(buf,&input[i],1); i++; }
            strcpy(tokens[token_count++], "id");
        } else {
            char buf[3]={input[i],'\0'}; strcpy(tokens[token_count++],buf); i++;
        }
    }
    strcpy(tokens[token_count],"$");
    token_count++;
}

void parse(char *input) {
    sp = 0; tokenize(input);
    printf("\n=== Shift-Reduce Parsing: %s ===\n\n",input);
    printf("%-35s %-20s %-30s\n","Stack","Input","Action");
    printf("------------------------------------------------------------------------------------\n");

    while(1) {
        /* Print state */
        char stack_str[200]="";
        for(int i=0;i<sp;i++) { strcat(stack_str,stk[i]); strcat(stack_str," "); }
        char input_str[200]="";
        for(int i=tok_idx;i<token_count;i++) { strcat(input_str,tokens[i]); strcat(input_str," "); }

        printf("%-35s %-20s",stack_str,input_str);

        /* Check accept */
        if(sp==1 && strcmp(stk[0],"E")==0 && tok_idx==token_count-1 && strcmp(tokens[tok_idx],"$")==0) {
            printf(" => ACCEPT\n");
            break;
        }

        /* Try to reduce */
        if(reduce()) { printf("\n"); continue; }

        /* Shift */
        if(tok_idx < token_count) {
            char *t = tokens[tok_idx++];
            if(strcmp(t,"$")==0 && sp>0) {
                printf(" => ERROR (unexpected end)\n");
                break;
            }
            push(t);
            printf(" => Shift %s\n",t);
        } else {
            printf(" => ERROR\n");
            break;
        }
    }
}

int main() {
    printf("=== Shift-Reduce Parser ===\n");
    printf("Grammar:\n");
    for(int i=0;i<prod_count;i++) printf("  %s\n",prods[i]);
    printf("\nSupports: id, +, *, (, )\n");

    char input[MAX], cont='y';
    while(cont=='y'||cont=='Y') {
        printf("\nEnter expression: "); scanf(" "); fgets(input,MAX,stdin);
        input[strcspn(input,"\n")]='\0';
        parse(input);
        printf("\nParse another? (y/n): "); scanf(" %c",&cont);
    }
    return 0;
}
