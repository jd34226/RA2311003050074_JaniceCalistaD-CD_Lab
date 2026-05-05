/*
 * Lab 9 - Computation of LR (0) items
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_PROD   30
#define MAX_LEN    50
#define MAX_ITEMS  200
#define MAX_STATES 100
#define MAX_SYMS   20

/* Augmented grammar */
char prod_lhs[MAX_PROD];
char prod_rhs[MAX_PROD][MAX_LEN];
int  prod_count;

/* An LR(0) item: production index + dot position */
typedef struct { int prod; int dot; } Item;

/* A state = set of items */
typedef struct {
    Item items[MAX_ITEMS];
    int  count;
    int  trans[256];   /* transitions: symbol -> state index (-1 = none) */
} State;

State states[MAX_STATES];
int state_count = 0;

int isNT(char c) { return isupper(c); }

int itemsEqual(Item a, Item b) { return a.prod==b.prod && a.dot==b.dot; }

int stateHasItem(State *s, Item it) {
    for(int i=0;i<s->count;i++) if(itemsEqual(s->items[i],it)) return 1;
    return 0;
}

/* Closure of a set of items */
void closure(State *s) {
    int changed=1;
    while(changed) {
        changed=0;
        for(int i=0;i<s->count;i++) {
            Item it=s->items[i];
            char *rhs=prod_rhs[it.prod];
            int rlen=strlen(rhs);
            if(it.dot>=rlen) continue;          /* dot at end */
            char B=rhs[it.dot];
            if(!isNT(B)) continue;               /* dot before terminal */

            /* Add all B -> •γ */
            for(int p=0;p<prod_count;p++) {
                if(prod_lhs[p]!=B) continue;
                Item ni; ni.prod=p; ni.dot=0;
                if(!stateHasItem(s,ni)) { s->items[s->count++]=ni; changed=1; }
            }
        }
    }
}

/* Goto(s, X) */
State gotoState(State *s, char X) {
    State ns; ns.count=0; memset(ns.trans,-1,sizeof(ns.trans));
    for(int i=0;i<s->count;i++) {
        Item it=s->items[i];
        char *rhs=prod_rhs[it.prod];
        if(it.dot>=(int)strlen(rhs)) continue;
        if(rhs[it.dot]==X) {
            Item ni; ni.prod=it.prod; ni.dot=it.dot+1;
            if(!stateHasItem(&ns,ni)) ns.items[ns.count++]=ni;
        }
    }
    closure(&ns);
    return ns;
}

int statesEqual(State *a, State *b) {
    if(a->count!=b->count) return 0;
    for(int i=0;i<a->count;i++) {
        int found=0;
        for(int j=0;j<b->count;j++) if(itemsEqual(a->items[i],b->items[j])) { found=1; break; }
        if(!found) return 0;
    }
    return 1;
}

int findState(State *s) {
    for(int i=0;i<state_count;i++) if(statesEqual(&states[i],s)) return i;
    return -1;
}

/* Collect all grammar symbols */
char symbols[MAX_SYMS]; int sym_count=0;
void collectSymbols() {
    for(int p=0;p<prod_count;p++) {
        /* LHS */
        int found=0;
        for(int i=0;i<sym_count;i++) if(symbols[i]==prod_lhs[p]) found=1;
        if(!found) symbols[sym_count++]=prod_lhs[p];
        /* RHS */
        for(int i=0;prod_rhs[p][i];i++) {
            char c=prod_rhs[p][i]; found=0;
            for(int j=0;j<sym_count;j++) if(symbols[j]==c) found=1;
            if(!found) symbols[sym_count++]=c;
        }
    }
}

void buildAutomaton() {
    collectSymbols();

    /* Initial state: closure({S' -> •S}) */
    state_count=0;
    states[0].count=0; memset(states[0].trans,-1,sizeof(states[0].trans));
    Item start; start.prod=0; start.dot=0;
    states[0].items[states[0].count++]=start;
    closure(&states[0]);
    state_count=1;

    int queue[MAX_STATES], qh=0, qt=0;
    queue[qt++]=0;

    while(qh<qt) {
        int cur=queue[qh++];
        for(int si=0;si<sym_count;si++) {
            char X=symbols[si];
            State ns=gotoState(&states[cur],X);
            if(ns.count==0) continue;

            int idx=findState(&ns);
            if(idx==-1) {
                idx=state_count;
                states[state_count++]=ns;
                queue[qt++]=idx;
            }
            states[cur].trans[(unsigned char)X]=idx;
        }
    }
}

void printItem(Item it) {
    printf("%c -> ",prod_lhs[it.prod]);
    char *rhs=prod_rhs[it.prod];
    int rlen=strlen(rhs);
    for(int i=0;i<rlen;i++) {
        if(i==it.dot) printf("•");
        printf("%c",rhs[i]);
    }
    if(it.dot==rlen) printf("•");
}

void printAutomaton() {
    printf("\n=== LR(0) Canonical Collection ===\n");
    for(int i=0;i<state_count;i++) {
        printf("\nI%d:\n",i);
        for(int j=0;j<states[i].count;j++) {
            printf("  "); printItem(states[i].items[j]); printf("\n");
        }
    }

    printf("\n=== Goto Transitions ===\n");
    printf("%-8s","State");
    for(int s=0;s<sym_count;s++) printf("  %-6c",symbols[s]);
    printf("\n");
    for(int i=0;i<state_count;i++) {
        printf("I%-7d",i);
        for(int s=0;s<sym_count;s++) {
            int t=states[i].trans[(unsigned char)symbols[s]];
            if(t==-1) printf("  %-6s","  -");
            else       printf("  I%-5d",t);
        }
        printf("\n");
    }

    /* Action hints */
    printf("\n=== Action Hints ===\n");
    for(int i=0;i<state_count;i++) {
        for(int j=0;j<states[i].count;j++) {
            Item it=states[i].items[j];
            char *rhs=prod_rhs[it.prod];
            if(it.dot==(int)strlen(rhs)) {
                /* Dot at end => Reduce */
                printf("I%d: Reduce by %c -> %s\n",i,prod_lhs[it.prod],rhs);
            }
        }
    }
}

int main() {
    printf("=== LR(0) Items Computation ===\n\n");
    printf("First production should be the augmented start: S'->S\n");
    printf("UPPERCASE=non-terminals, lowercase=terminals\n\n");

    printf("Enter number of productions: "); scanf("%d",&prod_count); getchar();
    for(int i=0;i<prod_count;i++) {
        char line[MAX_LEN]; printf("Production %d: ",i+1);
        fgets(line,MAX_LEN,stdin); line[strcspn(line,"\n")]='\0';
        prod_lhs[i]=line[0]; strcpy(prod_rhs[i],line+3);
    }

    buildAutomaton();
    printAutomaton();
    return 0;
}
