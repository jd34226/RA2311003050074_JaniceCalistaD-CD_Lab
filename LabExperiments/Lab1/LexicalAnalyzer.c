/*
 * Lab 1 - Implementation of Lexical Analyzer
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 100

const char *keywords[] = {
    "int","float","char","double","void","if","else","while","for","do",
    "return","break","continue","switch","case","struct","typedef",
    "include","main","printf","scanf"
};
const int num_keywords = sizeof(keywords)/sizeof(keywords[0]);

int isKeyword(const char *word) {
    for (int i = 0; i < num_keywords; i++)
        if (strcmp(keywords[i], word) == 0) return 1;
    return 0;
}

int isOperator(char ch) {
    return strchr("+-*/=%<>&|!^~", ch) != NULL;
}

int isDelimiter(char ch) {
    return strchr(" \t\n;,(){}[]", ch) != NULL;
}

void analyze(const char *src) {
    int i = 0, len = strlen(src);

    printf("\n%-20s %-15s\n", "Token", "Type");
    printf("------------------------------------\n");

    while (i < len) {
        if (isspace(src[i])) { i++; continue; }

        /* String literal */
        if (src[i] == '"') {
            char str[MAX] = "\"";
            i++;
            while (i < len && src[i] != '"') { strncat(str, &src[i], 1); i++; }
            strcat(str, "\""); i++;
            printf("%-20s %-15s\n", str, "String Literal");
            continue;
        }

        /* Number */
        if (isdigit(src[i])) {
            char num[MAX] = "";
            while (i < len && (isdigit(src[i]) || src[i] == '.')) { strncat(num, &src[i], 1); i++; }
            printf("%-20s %-15s\n", num, "Number");
            continue;
        }

        /* Identifier or keyword */
        if (isalpha(src[i]) || src[i] == '_') {
            char word[MAX] = "";
            while (i < len && (isalnum(src[i]) || src[i] == '_')) { strncat(word, &src[i], 1); i++; }
            printf("%-20s %-15s\n", word, isKeyword(word) ? "Keyword" : "Identifier");
            continue;
        }

        /* Two-char operators */
        char two[3] = {src[i], src[i+1], '\0'};
        if (i + 1 < len && 
            (!strcmp(two,"==") || !strcmp(two,"!=") || !strcmp(two,">=") || !strcmp(two,"<=") ||
             !strcmp(two,"&&") || !strcmp(two,"||") || !strcmp(two,"++") || !strcmp(two,"--") ||
             !strcmp(two,"+=") || !strcmp(two,"-="))) {
            printf("%-20s %-15s\n", two, "Operator");
            i += 2; continue;
        }

        /* Single-char operator */
        if (isOperator(src[i])) {
            printf("%-20c %-15s\n", src[i], "Operator");
            i++; continue;
        }

        /* Delimiter */
        if (isDelimiter(src[i])) {
            if (!isspace(src[i]))
                printf("%-20c %-15s\n", src[i], "Delimiter");
            i++; continue;
        }

        /* Unknown */
        printf("%-20c %-15s\n", src[i], "Unknown");
        i++;
    }
}

int main() {
    char source[1000];
    printf("=== Lexical Analyzer ===\n");
    printf("Enter source code (end with #):\n");

    int i = 0, ch;
    while ((ch = getchar()) != '#' && i < sizeof(source)-1) source[i++] = ch;
    source[i] = '\0';

    analyze(source);
    return 0;
}
