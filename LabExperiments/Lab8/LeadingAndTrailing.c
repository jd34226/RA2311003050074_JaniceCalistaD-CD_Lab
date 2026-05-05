/*
 * Lab 8 - Computation of LEADING AND TRAILING
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_NT   10
#define MAX_PROD 30
#define MAX_LEN  50
#define MAX_SET  20

char prod_lhs[MAX_PROD];
char prod_rhs[MAX_PROD][MAX_LEN];
int  prod_count;

char non_terminals[MAX_NT]; int nt_count;

char LEADING [MAX_NT][MAX_SET]; int LEAD_cnt[MAX_NT];
char TRAILING[MAX_NT][MAX_SET]; int TRAIL_cnt[MAX_NT];

int isNT(char c) { return isupper(c); }

int ntIndex(char c) {
    for(int i=0;i<nt_count;i++) if(non_terminals[i]==c) return i;
    return -1;
}

int addToSet(char *set, int *cnt, char c) {
    for(int i=0;i<*cnt;i++) if(set[i]==c) return 0;
    set[(*cnt)++]=c; return 1;
}

/*
 * LEADING(A):
 *   For each A -> X1 X2 ... Xn:
 *     If X1 is terminal t: add t
 *     If X1 is non-terminal B: add LEADING(B)
 *     If X1 is non-terminal B and ε ∈ B: continue to X2
 *   (simplified: we don't handle ε-productions here)
 */
void computeLEADING() {
    memset(LEAD_cnt,0,sizeof(LEAD_cnt));
    int changed=1;
    while(changed) {
        changed=0;
        for(int p=0;p<prod_count;p++) {
            int ai=ntIndex(prod_lhs[p]);
            char *rhs=prod_rhs[p];
            int rlen=strlen(rhs);

            for(int i=0;i<rlen;i++) {
                char Xi=rhs[i];
                if(!isNT(Xi)) {
                    /* Terminal: add it */
                    if(addToSet(LEADING[ai],&LEAD_cnt[ai],Xi)) changed=1;
                    break;  /* stop */
                } else {
                    /* Non-terminal: add its LEADING */
                    int bi=ntIndex(Xi);
                    for(int j=0;j<LEAD_cnt[bi];j++)
                        if(addToSet(LEADING[ai],&LEAD_cnt[ai],LEADING[bi][j])) changed=1;
                    /* simplified: break (no ε handling) */
                    break;
                }
            }
        }
    }
}

/*
 * TRAILING(A):
 *   For each A -> X1 X2 ... Xn:
 *     Look at Xn (rightmost)
 *     If terminal t: add t
 *     If non-terminal B: add TRAILING(B)
 */
void computeTRAILING() {
    memset(TRAIL_cnt,0,sizeof(TRAIL_cnt));
    int changed=1;
    while(changed) {
        changed=0;
        for(int p=0;p<prod_count;p++) {
            int ai=ntIndex(prod_lhs[p]);
            char *rhs=prod_rhs[p];
            int rlen=strlen(rhs);
            if(rlen==0) continue;

            for(int i=rlen-1;i>=0;i--) {
                char Xi=rhs[i];
                if(!isNT(Xi)) {
                    if(addToSet(TRAILING[ai],&TRAIL_cnt[ai],Xi)) changed=1;
                    break;
                } else {
                    int bi=ntIndex(Xi);
                    for(int j=0;j<TRAIL_cnt[bi];j++)
                        if(addToSet(TRAILING[ai],&TRAIL_cnt[ai],TRAILING[bi][j])) changed=1;
                    break;
                }
            }
        }
    }
}

void printSet(char *set, int cnt) {
    printf("{ ");
    for(int i=0;i<cnt;i++) printf("%c ",set[i]);
    printf("}");
}

int main() {
    printf("=== LEADING and TRAILING Computation ===\n\n");
    printf("UPPERCASE = non-terminals, lowercase/symbols = terminals\n\n");

    printf("Enter number of productions: "); scanf("%d",&prod_count); getchar();
    nt_count=0;
    printf("Format: A->alpha (e.g. E->E+T)\n");
    for(int i=0;i<prod_count;i++) {
        char line[MAX_LEN]; printf("Production %d: ",i+1);
        fgets(line,MAX_LEN,stdin); line[strcspn(line,"\n")]='\0';
        prod_lhs[i]=line[0]; strcpy(prod_rhs[i],line+3);
        if(ntIndex(line[0])<0) non_terminals[nt_count++]=line[0];
        for(int j=3;line[j];j++)
            if(isNT(line[j])&&ntIndex(line[j])<0) non_terminals[nt_count++]=line[j];
    }

    computeLEADING();
    computeTRAILING();

    printf("\n%-15s %-25s %-25s\n","Non-Terminal","LEADING","TRAILING");
    printf("--------------------------------------------------------------\n");
    for(int i=0;i<nt_count;i++) {
        printf("%-15c ",non_terminals[i]);
        printSet(LEADING[i],LEAD_cnt[i]);
        printf("\t\t");
        printSet(TRAILING[i],TRAIL_cnt[i]);
        printf("\n");
    }

    /* Operator Precedence Relations (bonus) */
    printf("\n=== Operator Precedence Relations ===\n");
    printf("For operators a, b in adjacent positions:\n");
    printf("  a <· b  if b ∈ LEADING of the NT after a\n");
    printf("  a ·> b  if a ∈ TRAILING of the NT before b\n");
    printf("  a ·= b  if a and b are both in a production like ...aAb...\n\n");

    /* Collect all terminals */
    char ops[MAX_SET]; int op_cnt=0;
    for(int p=0;p<prod_count;p++) {
        char *rhs=prod_rhs[p];
        for(int i=0;rhs[i];i++) {
            if(!isNT(rhs[i])) {
                int found=0;
                for(int j=0;j<op_cnt;j++) if(ops[j]==rhs[i]) found=1;
                if(!found) ops[op_cnt++]=rhs[i];
            }
        }
    }

    printf("Operators found: ");
    for(int i=0;i<op_cnt;i++) printf("%c ",ops[i]);
    printf("\n\n");

    /* Print precedence table */
    printf("  ");
    for(int j=0;j<op_cnt;j++) printf("  %c ",ops[j]);
    printf("\n");

    for(int i=0;i<op_cnt;i++) {
        printf("%c ",ops[i]);
        for(int j=0;j<op_cnt;j++) {
            /* Simplified: same priority = ·= for same op, < for + before *, > for * before + */
            if(ops[i]==ops[j]) printf(" ·= ");
            else if((ops[i]=='+'||ops[i]=='-') && (ops[j]=='*'||ops[j]=='/')) printf(" <· ");
            else if((ops[i]=='*'||ops[i]=='/') && (ops[j]=='+'||ops[j]=='-')) printf(" ·> ");
            else printf("  ? ");
        }
        printf("\n");
    }

    return 0;
}
