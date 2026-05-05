/*
 * Lab 2 - Conversion from Regular Expression to NFA
 */

#include <stdio.h>
#include <string.h>

#define EPSILON 'e'

typedef struct {
    int from, to;
    char symbol;
} Transition;

Transition transitions[100];
int trans_count = 0;
int state_count = 0;

typedef struct { int start, end; } NFA;

/* Utility functions */
int newState() { return state_count++; }
void addTrans(int from, int to, char sym) {
    transitions[trans_count++] = (Transition){from, to, sym};
}

/* Basic NFA builders */
NFA buildChar(char c) {
    NFA n = { newState(), newState() };
    addTrans(n.start, n.end, c);
    return n;
}

NFA buildUnion(NFA a, NFA b) {
    NFA n = { newState(), newState() };
    addTrans(n.start, a.start, EPSILON);
    addTrans(n.start, b.start, EPSILON);
    addTrans(a.end, n.end, EPSILON);
    addTrans(b.end, n.end, EPSILON);
    return n;
}

NFA buildConcat(NFA a, NFA b) {
    addTrans(a.end, b.start, EPSILON);
    return (NFA){a.start, b.end};
}

NFA buildStar(NFA a) {
    NFA n = { newState(), newState() };
    addTrans(n.start, a.start, EPSILON);
    addTrans(n.start, n.end, EPSILON);
    addTrans(a.end, a.start, EPSILON);
    addTrans(a.end, n.end, EPSILON);
    return n;
}

/* Very simple regex parser: handles a|b, ab, a* */
NFA regexToNFA(const char *re) {
    NFA stack[50]; int top = -1;
    int i = 0, len = strlen(re);

    while (i < len) {
        char c = re[i];
        if (c == '|') {
            NFA b = stack[top--];
            NFA a = stack[top--];
            stack[++top] = buildUnion(a, b);
        } else if (c == '*') {
            NFA a = stack[top--];
            stack[++top] = buildStar(a);
        } else {
            stack[++top] = buildChar(c);
            /* implicit concatenation */
            if (i+1 < len && re[i+1] != '|' && re[i+1] != '*') {
                NFA b = buildChar(re[i+1]);
                stack[top] = buildConcat(stack[top], b);
                i++; // skip next char (already used)
            }
        }
        i++;
    }
    return stack[top];
}

void printNFA(NFA n) {
    printf("\n=== NFA ===\n");
    printf("Start: q%d, End: q%d\n", n.start, n.end);
    printf("Transitions:\n");
    for (int i = 0; i < trans_count; i++) {
        char sym = transitions[i].symbol;
        if (sym == EPSILON) printf("q%d -- ε --> q%d\n", transitions[i].from, transitions[i].to);
        else                printf("q%d -- %c --> q%d\n", transitions[i].from, sym, transitions[i].to);
    }
}

int main() {
    char re[50];
    printf("Enter regex (use | and *): ");
    scanf("%s", re);

    NFA nfa = regexToNFA(re);
    printNFA(nfa);
    return 0;
}
