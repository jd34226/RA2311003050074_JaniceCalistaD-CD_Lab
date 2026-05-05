/*
 * Lab 5 -FIRST AND FOLLOW Computation
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_PROD  30
#define MAX_LEN   50
#define MAX_NT    10
#define MAX_SET   30

char prod_lhs[MAX_PROD];
char prod_rhs[MAX_PROD][MAX_LEN];
int  prod_count;

char non_terminals[MAX_NT];
int  nt_count;

char FIRST[MAX_NT][MAX_SET];
int  FIRST_count[MAX_NT];

char FOLLOW[MAX_NT][MAX_SET];
int  FOLLOW_count[MAX_NT];

char start_symbol;

int isNT(char c) { return isupper(c); }

int ntIndex(char c) {
    for (int i = 0; i < nt_count; i++)
        if (non_terminals[i] == c) return i;
    return -1;
}

int addToSet(char *set, int *cnt, char c) {
    for (int i = 0; i < *cnt; i++) if (set[i] == c) return 0;
    set[(*cnt)++] = c;
    return 1;
}

/* FIRST of a string */
void firstOfString(char *str, char *result, int *rcount) {
    if (strlen(str) == 0) { addToSet(result, rcount, '#'); return; }

    for (int i = 0; str[i]; i++) {
        char X = str[i];
        if (!isNT(X)) { addToSet(result, rcount, X); break; }

        int idx = ntIndex(X), eps = 0;
        for (int j = 0; j < FIRST_count[idx]; j++) {
            if (FIRST[idx][j] == '#') eps = 1;
            else addToSet(result, rcount, FIRST[idx][j]);
        }
        if (!eps) break;
        if (str[i+1] == '\0') addToSet(result, rcount, '#');
    }
}

/* Compute FIRST sets */
void computeFIRST() {
    memset(FIRST_count, 0, sizeof(FIRST_count));
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int p = 0; p < prod_count; p++) {
            int ai = ntIndex(prod_lhs[p]);
            char tmp[MAX_SET]; int tc = 0;
            firstOfString(prod_rhs[p], tmp, &tc);
            for (int i = 0; i < tc; i++)
                if (addToSet(FIRST[ai], &FIRST_count[ai], tmp[i])) changed = 1;
        }
    }
}

/* Compute FOLLOW sets */
void computeFOLLOW() {
    memset(FOLLOW_count, 0, sizeof(FOLLOW_count));
    addToSet(FOLLOW[ntIndex(start_symbol)], &FOLLOW_count[ntIndex(start_symbol)], '$');

    int changed = 1;
    while (changed) {
        changed = 0;
        for (int p = 0; p < prod_count; p++) {
            char A = prod_lhs[p];
            char *rhs = prod_rhs[p];
            int rlen = strlen(rhs);

            for (int i = 0; i < rlen; i++) {
                char B = rhs[i];
                if (!isNT(B)) continue;
                int bi = ntIndex(B);

                char suffix[MAX_LEN]; strcpy(suffix, rhs+i+1);
                char tmp[MAX_SET]; int tc = 0;
                firstOfString(suffix, tmp, &tc);

                int eps = 0;
                for (int j = 0; j < tc; j++) {
                    if (tmp[j] == '#') eps = 1;
                    else if (addToSet(FOLLOW[bi], &FOLLOW_count[bi], tmp[j])) changed = 1;
                }

                if (eps || suffix[0] == '\0') {
                    int ai = ntIndex(A);
                    for (int j = 0; j < FOLLOW_count[ai]; j++)
                        if (addToSet(FOLLOW[bi], &FOLLOW_count[bi], FOLLOW[ai][j])) changed = 1;
                }
            }
        }
    }
}

void printSet(char *set, int cnt) {
    printf("{ ");
    for (int i = 0; i < cnt; i++)
        printf("%c ", set[i] == '#' ? 'ε' : set[i]);
    printf("}");
}

int main() {
    printf("=== FIRST and FOLLOW Computation ===\n");
    printf("Use UPPERCASE for non-terminals, lowercase for terminals\n");
    printf("Use '#' for epsilon, '$' for end-of-input\n\n");

    printf("Enter start symbol: ");
    scanf(" %c", &start_symbol);
    printf("Enter number of productions: ");
    scanf("%d", &prod_count);
    getchar();

    nt_count = 0;
    for (int i = 0; i < prod_count; i++) {
        char line[MAX_LEN];
        printf("Production %d: ", i+1);
        fgets(line, MAX_LEN, stdin);
        line[strcspn(line, "\n")] = '\0';
        prod_lhs[i] = line[0];
        strcpy(prod_rhs[i], line+3);

        if (ntIndex(prod_lhs[i]) == -1) non_terminals[nt_count++] = prod_lhs[i];
        for (int j = 3; line[j]; j++)
            if (isNT(line[j]) && ntIndex(line[j]) == -1) non_terminals[nt_count++] = line[j];
    }

    computeFIRST();
    computeFOLLOW();

    printf("\n%-12s %-20s %-20s\n", "Non-Terminal", "FIRST", "FOLLOW");
    printf("------------------------------------------------------\n");
    for (int i = 0; i < nt_count; i++) {
        printf("%-12c ", non_terminals[i]);
        printSet(FIRST[i], FIRST_count[i]);
        printf("\t");
        printSet(FOLLOW[i], FOLLOW_count[i]);
        printf("\n");
    }
    return 0;
}
