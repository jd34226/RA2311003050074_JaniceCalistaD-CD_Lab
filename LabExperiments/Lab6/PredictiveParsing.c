/*
 * Lab 6 - Predictive Parsing Table
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_PROD  30
#define MAX_LEN   50
#define MAX_NT    10
#define MAX_T     15
#define MAX_SET   20
#define EMPTY     ""

char prod_lhs[MAX_PROD];
char prod_rhs[MAX_PROD][MAX_LEN];
int  prod_count;

char non_terminals[MAX_NT]; int nt_count;
char terminals[MAX_T];      int t_count;

char FIRST[MAX_NT][MAX_SET];   int FIRST_count[MAX_NT];
char FOLLOW[MAX_NT][MAX_SET];  int FOLLOW_count[MAX_NT];

/* Parsing table: table[nt_idx][t_idx] = production index (-1 = error) */
int table[MAX_NT][MAX_T];

char start_symbol;

int isNT(char c)  { return isupper(c); }
int ntIndex(char c) { for(int i=0;i<nt_count;i++) if(non_terminals[i]==c) return i; return -1; }
int tIndex(char c)  { for(int i=0;i<t_count;i++)  if(terminals[i]==c) return i; return -1; }

int addToSet(char *set, int *cnt, char c) {
    for(int i=0;i<*cnt;i++) if(set[i]==c) return 0;
    set[(*cnt)++]=c; return 1;
}

void computeFirstOfString(char *str, char *res, int *rc) {
    int len = strlen(str);
    if (len==0||str[0]=='#') { addToSet(res,rc,'#'); return; }
    for (int i=0;i<len;i++) {
        char Xi=str[i];
        if (!isNT(Xi)) { addToSet(res,rc,Xi); break; }
        int idx=ntIndex(Xi); int had_e=0;
        for(int j=0;j<FIRST_count[idx];j++) {
            if(FIRST[idx][j]=='#'){had_e=1;continue;}
            addToSet(res,rc,FIRST[idx][j]);
        }
        if(!had_e) break;
        if(i==len-1) addToSet(res,rc,'#');
    }
}

void computeFIRST() {
    memset(FIRST_count,0,sizeof(FIRST_count));
    int changed=1;
    while(changed) {
        changed=0;
        for(int p=0;p<prod_count;p++) {
            int ai=ntIndex(prod_lhs[p]);
            char tmp[MAX_SET]; int tc=0;
            computeFirstOfString(prod_rhs[p],tmp,&tc);
            for(int i=0;i<tc;i++) if(addToSet(FIRST[ai],&FIRST_count[ai],tmp[i])) changed=1;
        }
    }
}

void computeFOLLOW() {
    memset(FOLLOW_count,0,sizeof(FOLLOW_count));
    addToSet(FOLLOW[ntIndex(start_symbol)],&FOLLOW_count[ntIndex(start_symbol)],'$');
    int changed=1;
    while(changed) {
        changed=0;
        for(int p=0;p<prod_count;p++) {
            char A=prod_lhs[p]; char *rhs=prod_rhs[p]; int rlen=strlen(rhs);
            for(int i=0;i<rlen;i++) {
                char B=rhs[i]; if(!isNT(B)) continue;
                int bi=ntIndex(B);
                char suffix[MAX_LEN]; strncpy(suffix,rhs+i+1,MAX_LEN-1); suffix[MAX_LEN-1]='\0';
                char tmp[MAX_SET]; int tc=0;
                computeFirstOfString(suffix,tmp,&tc);
                int eps=0;
                for(int j=0;j<tc;j++) { if(tmp[j]=='#'){eps=1;continue;} if(addToSet(FOLLOW[bi],&FOLLOW_count[bi],tmp[j])) changed=1; }
                if(eps||strlen(suffix)==0) {
                    int ai=ntIndex(A);
                    for(int j=0;j<FOLLOW_count[ai];j++) if(addToSet(FOLLOW[bi],&FOLLOW_count[bi],FOLLOW[ai][j])) changed=1;
                }
            }
        }
    }
}

void buildTable() {
    memset(table,-1,sizeof(table));
    for(int p=0;p<prod_count;p++) {
        char A=prod_lhs[p]; int ai=ntIndex(A);
        char *rhs=prod_rhs[p];

        char tmp[MAX_SET]; int tc=0;
        computeFirstOfString(rhs,tmp,&tc);

        for(int i=0;i<tc;i++) {
            if(tmp[i]=='#') continue;
            int ti=tIndex(tmp[i]);
            if(ti>=0) {
                if(table[ai][ti]!=-1) printf("Conflict at [%c][%c]!\n",A,tmp[i]);
                table[ai][ti]=p;
            }
        }
        /* If epsilon in FIRST(rhs), add p for each in FOLLOW(A) */
        int has_eps=0;
        for(int i=0;i<tc;i++) if(tmp[i]=='#') has_eps=1;
        if(has_eps||strcmp(rhs,"#")==0) {
            for(int i=0;i<FOLLOW_count[ai];i++) {
                int ti=tIndex(FOLLOW[ai][i]);
                if(ti>=0) {
                    if(table[ai][ti]!=-1) printf("Conflict at [%c][%c]!\n",A,FOLLOW[ai][i]);
                    table[ai][ti]=p;
                }
            }
        }
    }
}

void printTable() {
    printf("\n=== LL(1) Predictive Parsing Table ===\n");
    /* Header */
    printf("%-6s", "NT\\T");
    for(int j=0;j<t_count;j++) printf(" %-12c",terminals[j]);
    printf("\n");
    for(int i=0;i<nt_count;i++) {
        printf("%-6c",non_terminals[i]);
        for(int j=0;j<t_count;j++) {
            if(table[i][j]==-1) printf(" %-12s","error");
            else {
                char buf[15];
                snprintf(buf,15,"%c->%s",prod_lhs[table[i][j]],prod_rhs[table[i][j]]);
                printf(" %-12s",buf);
            }
        }
        printf("\n");
    }
}

void parseString(char *input) {
    printf("\n=== Parsing: %s ===\n",input);
    char stack[100]; int sp=0;
    stack[sp++]='$';
    stack[sp++]=start_symbol;

    int ip=0; int ilen=strlen(input);

    printf("%-30s %-15s %-20s\n","Stack","Input","Action");
    printf("------------------------------------------------------------------\n");

    while(sp>0) {
        /* Print stack */
        char stk_str[100]=""; for(int i=sp-1;i>=0;i--) { char tmp[3]={stack[i],'\0'}; strcat(stk_str,tmp); }
        char inp_str[100]=""; for(int i=ip;i<=ilen;i++) { char tmp[3]={i<ilen?input[i]:'$','\0'}; strcat(inp_str,tmp); }

        char X=stack[sp-1];
        char a=(ip<ilen)?input[ip]:'$';

        if(X=='$' && a=='$') {
            printf("%-30s %-15s %-20s\n",stk_str,inp_str,"Accept!");
            return;
        }
        if(X==a) {
            printf("%-30s %-15s Match %c\n",stk_str,inp_str,a);
            sp--; ip++;
        } else if(isNT(X)) {
            int xi=ntIndex(X), ti=tIndex(a);
            if(ti<0||table[xi][ti]==-1) {
                printf("%-30s %-15s Error!\n",stk_str,inp_str);
                return;
            }
            int p=table[xi][ti];
            printf("%-30s %-15s %c->%s\n",stk_str,inp_str,prod_lhs[p],prod_rhs[p]);
            sp--;
            char *rhs=prod_rhs[p];
            if(strcmp(rhs,"#")!=0) {
                for(int i=strlen(rhs)-1;i>=0;i--) stack[sp++]=rhs[i];
            }
        } else {
            printf("%-30s %-15s Error!\n",stk_str,inp_str);
            return;
        }
    }
}

int main() {
    printf("=== LL(1) Predictive Parser ===\n\n");
    printf("UPPERCASE=non-terminals, lowercase=terminals, #=epsilon\n");
    printf("Enter start symbol: "); scanf(" %c",&start_symbol);
    printf("Enter terminals (e.g: +*()id$): "); char tsym[MAX_T*2]; scanf("%s",tsym);
    t_count=0; for(int i=0;tsym[i];i++) terminals[t_count++]=tsym[i];

    printf("Enter number of productions: "); scanf("%d",&prod_count); getchar();
    nt_count=0;
    printf("Format: A->alpha\n");
    for(int i=0;i<prod_count;i++) {
        char line[MAX_LEN]; printf("Production %d: ",i+1);
        fgets(line,MAX_LEN,stdin); line[strcspn(line,"\n")]='\0';
        prod_lhs[i]=line[0]; strcpy(prod_rhs[i],line+3);
        if(ntIndex(line[0])<0) non_terminals[nt_count++]=line[0];
        for(int j=3;line[j];j++) if(isNT(line[j])&&ntIndex(line[j])<0) non_terminals[nt_count++]=line[j];
    }

    computeFIRST(); computeFOLLOW(); buildTable(); printTable();

    char inp[100]; char cont='y';
    while(cont=='y'||cont=='Y') {
        printf("\nEnter string to parse (without $): "); scanf("%s",inp);
        parseString(inp);
        printf("Parse another? (y/n): "); scanf(" %c",&cont);
    }
    return 0;
}
