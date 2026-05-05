/*
 * Lab 13 - Implementation of DAG
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_NODES 100
#define MAX_VARS  50
#define MAX_INSTRS 50

/* ---- DAG Node ---- */
typedef struct {
    char op[5];         /* operator or empty for leaf */
    int  left;          /* index of left child (-1 = none) */
    int  right;         /* index of right child (-1 = none) */
    char labels[10][10];/* variable names that hold this value */
    int  label_cnt;
    int  is_leaf;
    char value[10];     /* for leaf nodes: variable or constant name */
} DAGNode;

DAGNode dag[MAX_NODES]; int dag_cnt = 0;

/* Variable -> node index mapping */
typedef struct { char name[10]; int node; } VarMap;
VarMap vmap[MAX_VARS]; int vmap_cnt = 0;

/* ---- TAC ---- */
typedef struct { char res[10]; char op[5]; char arg1[10]; char arg2[10]; } TAC;
TAC instrs[MAX_INSTRS]; int instr_cnt = 0;

int getVarNode(char *var) {
    for(int i=0;i<vmap_cnt;i++) if(strcmp(vmap[i].name,var)==0) return vmap[i].node;
    return -1;
}

void setVarNode(char *var, int node) {
    for(int i=0;i<vmap_cnt;i++) {
        if(strcmp(vmap[i].name,var)==0) { vmap[i].node=node; return; }
    }
    strcpy(vmap[vmap_cnt].name,var); vmap[vmap_cnt].node=node; vmap_cnt++;
}

void addLabel(int node, char *label) {
    for(int i=0;i<dag[node].label_cnt;i++) if(strcmp(dag[node].labels[i],label)==0) return;
    strcpy(dag[node].labels[dag[node].label_cnt++],label);
}

/* Find existing interior node with same op and children */
int findInterior(char *op, int left, int right) {
    for(int i=0;i<dag_cnt;i++) {
        if(!dag[i].is_leaf &&
           strcmp(dag[i].op,op)==0 &&
           dag[i].left==left &&
           dag[i].right==right) return i;
    }
    return -1;
}

/* Get or create leaf node for a variable/constant */
int getLeaf(char *name) {
    int n = getVarNode(name);
    if(n >= 0) return n;
    /* Check if there's already a leaf with this value */
    for(int i=0;i<dag_cnt;i++) {
        if(dag[i].is_leaf && strcmp(dag[i].value,name)==0) {
            setVarNode(name,i); return i;
        }
    }
    /* Create new leaf */
    int idx = dag_cnt++;
    dag[idx].is_leaf=1; dag[idx].left=-1; dag[idx].right=-1;
    dag[idx].label_cnt=0; dag[idx].op[0]='\0';
    strcpy(dag[idx].value,name);
    addLabel(idx,name);
    setVarNode(name,idx);
    return idx;
}

void processInstr(TAC *t) {
    if(strlen(t->op)==0 || strcmp(t->op,"=")==0) {
        /* Assignment: res = arg1 */
        int n = getLeaf(t->arg1);
        setVarNode(t->res, n);
        addLabel(n, t->res);
        return;
    }

    /* Binary op */
    int n1 = getLeaf(t->arg1);
    int n2 = getLeaf(t->arg2);

    /* Check for existing node */
    int n = findInterior(t->op, n1, n2);
    if(n < 0) {
        /* Create new */
        n = dag_cnt++;
        dag[n].is_leaf=0; dag[n].label_cnt=0;
        strcpy(dag[n].op,t->op);
        dag[n].left=n1; dag[n].right=n2;
        dag[n].value[0]='\0';
    }
    setVarNode(t->res, n);
    addLabel(n, t->res);
}

void printDAG() {
    printf("\n=== DAG Nodes ===\n");
    printf("%-6s %-8s %-8s %-8s %-20s\n","Node","Op/Val","Left","Right","Labels");
    printf("--------------------------------------------------------------\n");
    for(int i=0;i<dag_cnt;i++) {
        char lstr[100]=""; 
        for(int j=0;j<dag[i].label_cnt;j++) {
            strcat(lstr,dag[i].labels[j]); strcat(lstr," ");
        }
        if(dag[i].is_leaf) {
            printf("N%-5d %-8s %-8s %-8s %s\n",i,dag[i].value,"leaf","leaf",lstr);
        } else {
            printf("N%-5d %-8s N%-7d N%-7d %s\n",i,dag[i].op,dag[i].left,dag[i].right,lstr);
        }
    }

    /* Identify common subexpressions */
    printf("\n=== Common Subexpressions (shared nodes) ===\n");
    int found=0;
    for(int i=0;i<dag_cnt;i++) {
        if(!dag[i].is_leaf && dag[i].label_cnt > 1) {
            printf("  N%d (%s) is reused: ",i,dag[i].op);
            for(int j=0;j<dag[i].label_cnt;j++) printf("%s ",dag[i].labels[j]);
            printf("\n"); found=1;
        }
    }
    if(!found) printf("  No common subexpressions found.\n");
}

/* Simplified dot notation */
void printDotGraph() {
    printf("\n=== DOT Notation (paste at graphviz.org) ===\n");
    printf("digraph DAG {\n");
    printf("  rankdir=BT;\n");
    for(int i=0;i<dag_cnt;i++) {
        if(dag[i].is_leaf) {
            char labs[100]=""; for(int j=0;j<dag[i].label_cnt;j++) { strcat(labs,dag[i].labels[j]); if(j<dag[i].label_cnt-1) strcat(labs,","); }
            printf("  N%d [label=\"%s\\n(%s)\", shape=ellipse];\n",i,dag[i].value,labs);
        } else {
            char labs[100]=""; for(int j=0;j<dag[i].label_cnt;j++) { strcat(labs,dag[i].labels[j]); if(j<dag[i].label_cnt-1) strcat(labs,","); }
            printf("  N%d [label=\"%s\\n(%s)\", shape=box];\n",i,dag[i].op,labs);
            printf("  N%d -> N%d [label=\"L\"];\n",i,dag[i].left);
            if(dag[i].right>=0) printf("  N%d -> N%d [label=\"R\"];\n",i,dag[i].right);
        }
    }
    printf("}\n");
}

/* Regenerate optimized TAC from DAG */
void regenerateCode() {
    printf("\n=== Optimized Code (after CSE elimination) ===\n");
    for(int i=0;i<dag_cnt;i++) {
        if(dag[i].is_leaf || dag[i].label_cnt==0) continue;
        char *primary = dag[i].labels[0];
        if(!dag[i].is_leaf) {
            char lname[10], rname[10];
            strcpy(lname, dag[dag[i].left].label_cnt>0 ? dag[dag[i].left].labels[0] : dag[dag[i].left].value);
            if(dag[i].right>=0) strcpy(rname, dag[dag[i].right].label_cnt>0 ? dag[dag[i].right].labels[0] : dag[dag[i].right].value);
            if(dag[i].right>=0) printf("  %s = %s %s %s\n",primary,lname,dag[i].op,rname);
            else printf("  %s = %s %s\n",primary,dag[i].op,lname);
        }
        for(int j=1;j<dag[i].label_cnt;j++) printf("  %s = %s\n",dag[i].labels[j],primary);
    }
}

void sampleBlock() {
    /* Basic block:
       t1 = a + b
       t2 = a + b    <- same as t1
       t3 = t1 * c
       t4 = t2 * c   <- same as t3
       d  = t4
    */
    instr_cnt=0;
    struct { char r[10],o[5],a[10],b[10]; } tmp[]={
        {"t1","+","a","b"},
        {"t2","+","a","b"},
        {"t3","*","t1","c"},
        {"t4","*","t2","c"},
        {"d", "=","t4",""}
    };
    for(int i=0;i<5;i++) {
        strcpy(instrs[instr_cnt].res,tmp[i].r); strcpy(instrs[instr_cnt].op,tmp[i].o);
        strcpy(instrs[instr_cnt].arg1,tmp[i].a); strcpy(instrs[instr_cnt].arg2,tmp[i].b);
        instr_cnt++;
    }
}

int main() {
    printf("=== DAG Construction for Basic Block Optimization ===\n");
    printf("1. Use sample basic block\n2. Enter custom TAC\nChoice: ");
    int ch; scanf("%d",&ch); getchar();

    if(ch==1) {
        sampleBlock();
        printf("\n=== Input Basic Block ===\n");
        for(int i=0;i<instr_cnt;i++) {
            if(strlen(instrs[i].arg2)>0)
                printf("  %s = %s %s %s\n",instrs[i].res,instrs[i].arg1,instrs[i].op,instrs[i].arg2);
            else
                printf("  %s = %s\n",instrs[i].res,instrs[i].arg1);
        }
    } else {
        printf("Enter TAC (format: res = arg1 op arg2 or res = arg1, 'end' to stop):\n");
        instr_cnt=0;
        char line[100];
        while(1) {
            printf("Instr %d: ",instr_cnt+1); fgets(line,100,stdin); line[strcspn(line,"\n")]='\0';
            if(strcmp(line,"end")==0) break;
            char res[10],eq[3],arg1[10],op[5],arg2[10];
            int n=sscanf(line,"%s %s %s %s %s",res,eq,arg1,op,arg2);
            strcpy(instrs[instr_cnt].res,res); strcpy(instrs[instr_cnt].arg1,arg1);
            if(n>=5) { strcpy(instrs[instr_cnt].op,op); strcpy(instrs[instr_cnt].arg2,arg2); }
            else     { strcpy(instrs[instr_cnt].op,"="); instrs[instr_cnt].arg2[0]='\0'; }
            instr_cnt++;
        }
    }

    dag_cnt=0; vmap_cnt=0;
    for(int i=0;i<instr_cnt;i++) processInstr(&instrs[i]);

    printDAG();
    regenerateCode();
    printDotGraph();
    return 0;
}
