/*
 * Lab 3 - Conversion from NFA to DFA
 */

#include <stdio.h>
#include <string.h>

#define MAX_STATES 10
#define MAX_SYMBOLS 5
#define MAX_DFA    20

/* NFA definition */
int nfa[MAX_STATES][MAX_SYMBOLS][MAX_STATES];
int nfa_trans_count[MAX_STATES][MAX_SYMBOLS];
int nfa_states, nfa_symbols;
char symbols[MAX_SYMBOLS];
int nfa_start, nfa_final[MAX_STATES], nfa_final_count;

/* DFA definition */
typedef struct {
    int states[MAX_STATES];
    int count;
} StateSet;

StateSet dfa_sets[MAX_DFA];
int dfa_trans[MAX_DFA][MAX_SYMBOLS];
int dfa_count = 0;
int dfa_final[MAX_DFA];

/* Check if two sets are equal */
int setsEqual(StateSet *a, StateSet *b) {
    if (a->count != b->count) return 0;
    for (int i = 0; i < a->count; i++) {
        int found = 0;
        for (int j = 0; j < b->count; j++)
            if (a->states[i] == b->states[j]) { found = 1; break; }
        if (!found) return 0;
    }
    return 1;
}

/* Add or find DFA state */
int findOrAdd(StateSet *s) {
    for (int i = 0; i < dfa_count; i++)
        if (setsEqual(&dfa_sets[i], s)) return i;
    dfa_sets[dfa_count] = *s;
    return dfa_count++;
}

/* Move function: from a set of NFA states on symbol */
void move(StateSet *s, int sym_idx, StateSet *result) {
    int visited[MAX_STATES] = {0};
    result->count = 0;
    for (int i = 0; i < s->count; i++) {
        int st = s->states[i];
        for (int j = 0; j < nfa_trans_count[st][sym_idx]; j++) {
            int next = nfa[st][sym_idx][j];
            if (!visited[next]) {
                visited[next] = 1;
                result->states[result->count++] = next;
            }
        }
    }
}

/* Check if DFA state is final */
int isFinal(StateSet *s) {
    for (int i = 0; i < s->count; i++)
        for (int j = 0; j < nfa_final_count; j++)
            if (s->states[i] == nfa_final[j]) return 1;
    return 0;
}

/* Subset Construction */
void subsetConstruction() {
    StateSet start;
    start.states[0] = nfa_start;
    start.count = 1;

    dfa_count = 0;
    memset(dfa_trans, -1, sizeof(dfa_trans));
    int start_idx = findOrAdd(&start);

    int queue[MAX_DFA], qhead = 0, qtail = 0;
    queue[qtail++] = start_idx;

    while (qhead < qtail) {
        int cur = queue[qhead++];
        dfa_final[cur] = isFinal(&dfa_sets[cur]);

        for (int si = 0; si < nfa_symbols; si++) {
            StateSet moved;
            move(&dfa_sets[cur], si, &moved);
            if (moved.count == 0) continue;

            int idx = findOrAdd(&moved);
            dfa_trans[cur][si] = idx;

            if (idx == dfa_count - 1) queue[qtail++] = idx;
        }
    }
}

/* Print DFA */
void printSet(StateSet *s) {
    printf("{");
    for (int i = 0; i < s->count; i++) {
        if (i) printf(",");
        printf("q%d", s->states[i]);
    }
    printf("}");
}

int main() {
    printf("=== NFA to DFA Conversion (Subset Construction) ===\n");

    printf("Enter number of NFA states: ");
    scanf("%d", &nfa_states);
    printf("Enter start state: ");
    scanf("%d", &nfa_start);
    printf("Enter number of final states: ");
    scanf("%d", &nfa_final_count);
    printf("Enter final states: ");
    for (int i = 0; i < nfa_final_count; i++) scanf("%d", &nfa_final[i]);

    printf("Enter number of input symbols: ");
    scanf("%d", &nfa_symbols);
    printf("Enter symbols: ");
    for (int i = 0; i < nfa_symbols; i++) scanf(" %c", &symbols[i]);

    memset(nfa_trans_count, 0, sizeof(nfa_trans_count));
    printf("\nEnter transitions (-1 to stop):\n");
    printf("Format: from_state symbol to_state\n");

    int fs, ts; char sym;
    while (1) {
        scanf("%d", &fs);
        if (fs == -1) break;
        scanf(" %c %d", &sym, &ts);
        int si = -1;
        for (int i = 0; i < nfa_symbols; i++) if (symbols[i] == sym) { si = i; break; }
        if (si >= 0) nfa[fs][si][nfa_trans_count[fs][si]++] = ts;
    }

    subsetConstruction();

    printf("\n=== DFA Transition Table ===\n");
    printf("%-6s %-15s", "State", "NFA States");
    for (int i = 0; i < nfa_symbols; i++) printf("  %-6c", symbols[i]);
    printf("  Final\n");
    printf("---------------------------------------------------\n");

    for (int i = 0; i < dfa_count; i++) {
        printf("D%-5d ", i);
        printSet(&dfa_sets[i]);
        for (int si = 0; si < nfa_symbols; si++) {
            if (dfa_trans[i][si] == -1) printf("  %-6s", "∅");
            else printf("  D%-4d", dfa_trans[i][si]);
        }
        printf("  %s\n", dfa_final[i] ? "Yes" : "No");
    }

    printf("\nDFA Start State : D0\n");
    printf("DFA Final States: ");
    for (int i = 0; i < dfa_count; i++) if (dfa_final[i]) printf("D%d ", i);
    printf("\n");
    return 0;
}
