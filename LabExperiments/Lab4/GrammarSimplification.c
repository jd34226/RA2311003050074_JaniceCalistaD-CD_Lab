/*
 * Lab 4 - Elimination of Ambiguity, Left Recursion and Left Factoring
 */

#include <stdio.h>
#include <string.h>

#define MAX_PRODS 20
#define MAX_LEN   100

char lhs[MAX_PRODS];
char rhs[MAX_PRODS][MAX_LEN];
int prod_count = 0;

void readGrammar() {
    printf("Enter number of productions: ");
    scanf("%d", &prod_count);
    getchar();
    printf("Format: A->alpha (e.g. E->E+T or E->T)\n");
    for (int i = 0; i < prod_count; i++) {
        char line[MAX_LEN];
        printf("Production %d: ", i+1);
        fgets(line, MAX_LEN, stdin);
        line[strcspn(line, "\n")] = '\0';
        lhs[i] = line[0];
        strcpy(rhs[i], line+3);  // skip "A->"
    }
}

void printGrammar(char l[], char r[][MAX_LEN], int n) {
    for (int i = 0; i < n; i++)
        printf("  %c -> %s\n", l[i], r[i]);
}

/* Left Recursion Elimination */
void eliminateLeftRecursion() {
    printf("\n=== Eliminating Left Recursion ===\n");
    printGrammar(lhs, rhs, prod_count);

    for (int i = 0; i < prod_count; i++) {
        char A = lhs[i];
        if (rhs[i][0] == A) {
            // Example: E -> E+T
            printf("\nAfter Elimination:\n");
            printf("  %c  -> %s%c'\n", A, rhs[i]+1, A);
            printf("  %c' -> %s%c' | ε\n", A, rhs[i]+2, A);
        }
    }
}

/* Left Factoring */
void leftFactoring() {
    printf("\n=== Left Factoring ===\n");
    printGrammar(lhs, rhs, prod_count);

    for (int i = 0; i < prod_count-1; i++) {
        if (lhs[i] == lhs[i+1] && rhs[i][0] == rhs[i+1][0]) {
            char A = lhs[i];
            printf("\nAfter Factoring:\n");
            printf("  %c  -> %c%c'\n", A, rhs[i][0], A);
            printf("  %c' -> %s | %s\n", A, rhs[i]+1, rhs[i+1]+1);
        }
    }
}

/* Ambiguity Examples */
void ambiguityExample() {
    printf("\n=== Ambiguity Examples ===\n");

    printf("\nDangling Else:\n");
    printf("  stmt -> if expr then stmt | if expr then stmt else stmt | other\n");
    printf("Unambiguous:\n");
    printf("  stmt -> matched_stmt | open_stmt\n");
    printf("  matched_stmt -> if expr then matched_stmt else matched_stmt | other\n");
    printf("  open_stmt -> if expr then stmt | if expr then matched_stmt else open_stmt\n");

    printf("\nArithmetic Grammar:\n");
    printf("  E -> E + E | E * E | (E) | id\n");
    printf("Unambiguous:\n");
    printf("  E -> E + T | T\n");
    printf("  T -> T * F | F\n");
    printf("  F -> (E) | id\n");
}

int main() {
    int choice;
    printf("=== Grammar Transformation Tool ===\n");
    printf("1. Eliminate Left Recursion\n");
    printf("2. Left Factoring\n");
    printf("3. Ambiguity Examples\n");
    printf("4. All\n");
    printf("Choice: ");
    scanf("%d", &choice);

    if (choice == 1) { readGrammar(); eliminateLeftRecursion(); }
    else if (choice == 2) { readGrammar(); leftFactoring(); }
    else if (choice == 3) { ambiguityExample(); }
    else if (choice == 4) {
        readGrammar();
        eliminateLeftRecursion();
        leftFactoring();
        ambiguityExample();
    }

    return 0;
}
