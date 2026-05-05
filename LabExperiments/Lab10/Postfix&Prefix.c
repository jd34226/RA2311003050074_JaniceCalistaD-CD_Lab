/*
 * Lab 10 - Intermediate code generation – Postfix, Prefix
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX 200

/* Stack for characters */
char cstack[MAX];
int  ctop = -1;

void cpush(char c) { cstack[++ctop] = c; }
char cpop()        { return cstack[ctop--]; }
char cpeek()       { return ctop >= 0 ? cstack[ctop] : '\0'; }
int  cempty()      { return ctop < 0; }

int precedence(char op) {
    switch(op) {
        case '+': case '-': return 1;
        case '*': case '/': return 2;
        case '^':           return 3;
        default:            return 0;
    }
}

int isOperator(char c) { return c=='+' || c=='-' || c=='*' || c=='/' || c=='^'; }
int isOperand(char c)  { return isalnum(c); }

/* Infix to Postfix (Shunting-Yard) */
void infixToPostfix(char *infix, char *postfix) {
    int pi = 0;
    ctop = -1;

    for(int i = 0; infix[i]; i++) {
        char c = infix[i];
        if(isspace(c)) continue;

        if(isOperand(c)) {
            postfix[pi++] = c;
            postfix[pi++] = ' ';
        } else if(c == '(') {
            cpush(c);
        } else if(c == ')') {
            while(!cempty() && cpeek() != '(') {
                postfix[pi++] = cpop();
                postfix[pi++] = ' ';
            }
            if(!cempty()) cpop();  /* pop '(' */
        } else if(isOperator(c)) {
            /* Right-associative for ^ */
            while(!cempty() && cpeek() != '(' &&
                  ((c == '^' && precedence(cpeek()) > precedence(c)) ||
                   (c != '^' && precedence(cpeek()) >= precedence(c)))) {
                postfix[pi++] = cpop();
                postfix[pi++] = ' ';
            }
            cpush(c);
        }
    }
    while(!cempty()) { postfix[pi++] = cpop(); postfix[pi++] = ' '; }
    postfix[pi] = '\0';
}

/* Infix to Prefix: reverse, swap brackets, postfix, reverse */
void infixToPrefix(char *infix, char *prefix) {
    /* Step 1: Reverse infix and swap ( ) */
    int len = strlen(infix);
    char rev[MAX];
    int ri = 0;
    for(int i = len-1; i >= 0; i--) {
        if(isspace(infix[i])) continue;
        if(infix[i] == '(') rev[ri++] = ')';
        else if(infix[i] == ')') rev[ri++] = '(';
        else rev[ri++] = infix[i];
    }
    rev[ri] = '\0';

    /* Step 2: Get postfix of reversed */
    char post[MAX];
    infixToPostfix(rev, post);

    /* Step 3: Remove trailing spaces, reverse */
    /* Remove spaces for clean reversal */
    char clean[MAX]; int ci = 0;
    for(int i = 0; post[i]; i++) if(post[i]!=' ') clean[ci++] = post[i];
    clean[ci] = '\0';

    int clen = strlen(clean);
    for(int i = 0; i < clen; i++) prefix[i] = clean[clen-1-i];
    prefix[clen] = '\0';
}

/* Evaluate postfix */
double evalPostfix(char *postfix) {
    double nstack[MAX]; int ntop = -1;
    char tok[20]; int ti = 0;

    for(int i = 0; postfix[i]; i++) {
        char c = postfix[i];
        if(c == ' ' || postfix[i+1] == '\0') {
            if(c != ' ') { tok[ti++] = c; }
            tok[ti] = '\0';
            if(ti == 0) continue;

            if(isdigit(tok[0]) || (tok[0]=='-' && ti>1)) {
                nstack[++ntop] = atof(tok);
            } else if(strlen(tok)==1 && isOperator(tok[0])) {
                double b = nstack[ntop--];
                double a = nstack[ntop--];
                double r;
                switch(tok[0]) {
                    case '+': r = a+b; break;
                    case '-': r = a-b; break;
                    case '*': r = a*b; break;
                    case '/': r = (b!=0)?a/b:0; break;
                    case '^': { double tmp=1; for(int k=0;k<(int)b;k++) tmp*=a; r=tmp; } break;
                    default:  r = 0;
                }
                nstack[++ntop] = r;
            } else if(isalpha(tok[0])) {
                /* Variable — assign value */
                printf("  Enter value for %s: ", tok);
                double val; scanf("%lf", &val);
                nstack[++ntop] = val;
            }
            ti = 0;
        } else {
            tok[ti++] = c;
        }
    }
    return ntop >= 0 ? nstack[ntop] : 0;
}

/* Three-address code from postfix */
void generateTAC(char *infix) {
    printf("\n--- Three-Address Code (TAC) ---\n");
    /* Parse and generate using stack of temporary names */
    char expr_stack[MAX][20]; int es_top = -1;
    int temp_num = 1;
    ctop = -1;

    char postfix[MAX];
    infixToPostfix(infix, postfix);

    char tok[10]; int ti = 0;
    for(int i = 0; postfix[i]; i++) {
        char c = postfix[i];
        if(c == ' ' || !postfix[i+1]) {
            if(c != ' ') tok[ti++] = c;
            tok[ti] = '\0';
            if(ti == 0) continue;

            if(isOperand(tok[0]) && strlen(tok)==1) {
                strcpy(expr_stack[++es_top], tok);
            } else if(strlen(tok)==1 && isOperator(tok[0])) {
                char b[20], a[20], tmp[20];
                strcpy(b, expr_stack[es_top--]);
                strcpy(a, expr_stack[es_top--]);
                snprintf(tmp, 20, "t%d", temp_num++);
                printf("  %s = %s %c %s\n", tmp, a, tok[0], b);
                strcpy(expr_stack[++es_top], tmp);
            }
            ti = 0;
        } else tok[ti++] = c;
    }
}

int main() {
    printf("=== Intermediate Code Generation: Postfix & Prefix ===\n\n");
    char infix[MAX], cont='y';
    while(cont=='y'||cont=='Y') {
        printf("Enter infix expression: ");
        scanf(" "); fgets(infix, MAX, stdin);
        infix[strcspn(infix,"\n")] = '\0';

        char postfix[MAX], prefix[MAX];
        infixToPostfix(infix, postfix);
        infixToPrefix(infix, prefix);

        printf("\n  Infix  : %s\n", infix);
        printf("  Postfix: %s\n", postfix);
        printf("  Prefix : %s\n", prefix);

        generateTAC(infix);

        /* Evaluate if numeric */
        int numeric = 1;
        for(int i=0;infix[i];i++) if(isalpha(infix[i])) { numeric=0; break; }
        if(numeric) {
            double result = evalPostfix(postfix);
            printf("\n  Result : %.4g\n", result);
        }

        printf("\nAnother? (y/n): "); scanf(" %c",&cont);
    }
    return 0;
}
