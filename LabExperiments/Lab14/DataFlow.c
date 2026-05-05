/*
 * Lab 14 - Implementation of Global Data Flow Analysis
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_BLOCKS 10
#define MAX_DEFS   30
#define MAX_VARS   15
#define MAX_SUCCS  3

/* ---- Definition ---- */
typedef struct { int block; char var[10]; int def_id; } Def;
Def all_defs[MAX_DEFS]; int def_count=0;

/* ---- Basic Block ---- */
typedef struct {
    int id;
    char succs[MAX_SUCCS]; int succ_cnt;
    char preds[MAX_SUCCS]; int pred_cnt;

    /* Reaching Definitions */
    int GEN[MAX_DEFS];  int gen_cnt;
    int KILL[MAX_DEFS]; int kill_cnt;
    int IN_RD[MAX_DEFS];  int in_rd_cnt;
    int OUT_RD[MAX_DEFS]; int out_rd_cnt;

    /* Live Variables */
    char USE[MAX_VARS][10]; int use_cnt;
    char DEF[MAX_VARS][10]; int def_vars_cnt;
    char IN_LV[MAX_VARS][10];  int in_lv_cnt;
    char OUT_LV[MAX_VARS][10]; int out_lv_cnt;
} Block;

Block blocks[MAX_BLOCKS]; int block_cnt=0;
int n_blocks;

int inSet_i(int *set, int cnt, int v) { for(int i=0;i<cnt;i++) if(set[i]==v) return 1; return 0; }
int addSet_i(int *set, int *cnt, int v) { if(!inSet_i(set,*cnt,v)) { set[(*cnt)++]=v; return 1; } return 0; }

int inSet_s(char set[][10], int cnt, char *v) { for(int i=0;i<cnt;i++) if(strcmp(set[i],v)==0) return 1; return 0; }
int addSet_s(char set[][10], int *cnt, char *v) { if(!inSet_s(set,*cnt,v)) { strcpy(set[(*cnt)++],v); return 1; } return 0; }

/* ---- Sample CFG ---- */
void buildSampleCFG() {
    /*
     * CFG:
     *   B1: d1: a=1, d2: b=2           -> B2
     *   B2: d3: c=a+b, d4: a=c         -> B3, B4
     *   B3: d5: b=a-1                  -> B5
     *   B4: d6: c=b*2                  -> B5
     *   B5: d7: a=b+c                  -> (end)
     */
    n_blocks=5;
    for(int i=0;i<n_blocks;i++) {
        blocks[i].id=i; blocks[i].succ_cnt=0; blocks[i].pred_cnt=0;
        blocks[i].gen_cnt=0; blocks[i].kill_cnt=0;
        blocks[i].in_rd_cnt=0; blocks[i].out_rd_cnt=0;
        blocks[i].use_cnt=0; blocks[i].def_vars_cnt=0;
        blocks[i].in_lv_cnt=0; blocks[i].out_lv_cnt=0;
    }

    /* Definitions: (block, var, def_id) */
    struct { int b; char v[5]; } defs[]={
        {0,"a"},{0,"b"},{1,"c"},{1,"a"},{2,"b"},{3,"c"},{4,"a"}};
    def_count=7;
    for(int i=0;i<def_count;i++) {
        all_defs[i].block=defs[i].b;
        strcpy(all_defs[i].var,defs[i].v);
        all_defs[i].def_id=i;
    }

    /* GEN and KILL for reaching definitions */
    /* GEN[B] = defs in B not killed by later defs in B (simplified: last def of each var in B) */
    /* KILL[B] = all defs of same var outside B */

    for(int b=0;b<n_blocks;b++) {
        /* Find defs in this block */
        for(int d=0;d<def_count;d++) {
            if(all_defs[d].block!=b) continue;
            /* GEN: add if this def is the last one for this var in this block */
            int is_last=1;
            for(int d2=d+1;d2<def_count;d2++)
                if(all_defs[d2].block==b && strcmp(all_defs[d2].var,all_defs[d].var)==0) { is_last=0; break; }
            if(is_last) addSet_i(blocks[b].GEN,&blocks[b].gen_cnt,d);

            /* KILL: all defs of same var not in this block */
            for(int d2=0;d2<def_count;d2++)
                if(all_defs[d2].block!=b && strcmp(all_defs[d2].var,all_defs[d].var)==0)
                    addSet_i(blocks[b].KILL,&blocks[b].kill_cnt,d2);
        }
    }

    /* USE and DEF for live variables */
    /* B0: a=1, b=2     -> DEF:{a,b}, USE:{} */
    /* B1: c=a+b, a=c   -> USE:{a,b}, DEF:{c,a} */
    /* B2: b=a-1        -> USE:{a}, DEF:{b} */
    /* B3: c=b*2        -> USE:{b}, DEF:{c} */
    /* B4: a=b+c        -> USE:{b,c}, DEF:{a} */
    struct { int b; char dv[5]; char uv[5]; } lu[]={
        {0,"a",""},{0,"b",""},
        {1,"c","a"},{1,"a","b"},{1,"c",""},  /* simplified */
        {2,"b","a"},
        {3,"c","b"},
        {4,"a","b"},{4,"a","c"}
    };
    for(int i=0;i<9;i++) {
        if(lu[i].dv[0]) addSet_s(blocks[lu[i].b].DEF,&blocks[lu[i].b].def_vars_cnt,lu[i].dv);
        if(lu[i].uv[0]) addSet_s(blocks[lu[i].b].USE,&blocks[lu[i].b].use_cnt,lu[i].uv);
    }

    /* Successors / Predecessors */
    blocks[0].succs[blocks[0].succ_cnt++]=1;
    blocks[1].succs[blocks[1].succ_cnt++]=2; blocks[1].succs[blocks[1].succ_cnt++]=3;
    blocks[2].succs[blocks[2].succ_cnt++]=4;
    blocks[3].succs[blocks[3].succ_cnt++]=4;

    /* Build predecessors */
    for(int b=0;b<n_blocks;b++)
        for(int s=0;s<blocks[b].succ_cnt;s++)
            blocks[blocks[b].succs[s]].preds[blocks[blocks[b].succs[s]].pred_cnt++]=b;
}

/* ---- Reaching Definitions (forward) ---- */
void reachingDefinitions() {
    /* Initialize: IN[0]=∅, OUT[b]=GEN[b] */
    for(int b=0;b<n_blocks;b++) {
        blocks[b].in_rd_cnt=0; blocks[b].out_rd_cnt=0;
        for(int i=0;i<blocks[b].gen_cnt;i++) addSet_i(blocks[b].OUT_RD,&blocks[b].out_rd_cnt,blocks[b].GEN[i]);
    }

    int changed=1;
    while(changed) {
        changed=0;
        for(int b=0;b<n_blocks;b++) {
            /* IN[b] = union of OUT[pred] for all pred */
            for(int p=0;p<blocks[b].pred_cnt;p++) {
                int pred=blocks[b].preds[p];
                for(int i=0;i<blocks[pred].out_rd_cnt;i++)
                    if(addSet_i(blocks[b].IN_RD,&blocks[b].in_rd_cnt,blocks[pred].OUT_RD[i])) changed=1;
            }
            /* OUT[b] = GEN[b] ∪ (IN[b] - KILL[b]) */
            int new_out[MAX_DEFS]; int nc=0;
            for(int i=0;i<blocks[b].gen_cnt;i++) addSet_i(new_out,&nc,blocks[b].GEN[i]);
            for(int i=0;i<blocks[b].in_rd_cnt;i++) {
                if(!inSet_i(blocks[b].KILL,blocks[b].kill_cnt,blocks[b].IN_RD[i]))
                    if(addSet_i(new_out,&nc,blocks[b].IN_RD[i])) changed=1;
            }
            for(int i=0;i<nc;i++) if(addSet_i(blocks[b].OUT_RD,&blocks[b].out_rd_cnt,new_out[i])) changed=1;
        }
    }
}

/* ---- Live Variable Analysis (backward) ---- */
void liveVariables() {
    /* Initialize: OUT[exit]=∅ */
    int changed=1;
    while(changed) {
        changed=0;
        /* Process in reverse order */
        for(int b=n_blocks-1;b>=0;b--) {
            /* OUT[b] = union of IN[succ] */
            for(int s=0;s<blocks[b].succ_cnt;s++) {
                int succ=blocks[b].succs[s];
                for(int i=0;i<blocks[succ].in_lv_cnt;i++)
                    if(addSet_s(blocks[b].OUT_LV,&blocks[b].out_lv_cnt,blocks[succ].IN_LV[i])) changed=1;
            }
            /* IN[b] = USE[b] ∪ (OUT[b] - DEF[b]) */
            for(int i=0;i<blocks[b].use_cnt;i++)
                if(addSet_s(blocks[b].IN_LV,&blocks[b].in_lv_cnt,blocks[b].USE[i])) changed=1;
            for(int i=0;i<blocks[b].out_lv_cnt;i++) {
                if(!inSet_s(blocks[b].DEF,blocks[b].def_vars_cnt,blocks[b].OUT_LV[i]))
                    if(addSet_s(blocks[b].IN_LV,&blocks[b].in_lv_cnt,blocks[b].OUT_LV[i])) changed=1;
            }
        }
    }
}

void printDefSet(int *set, int cnt) {
    printf("{");
    for(int i=0;i<cnt;i++) {
        printf("d%d(%s)",all_defs[set[i]].def_id,all_defs[set[i]].var);
        if(i<cnt-1) printf(",");
    }
    printf("}");
}

void printVarSet(char set[][10], int cnt) {
    printf("{");
    for(int i=0;i<cnt;i++) { printf("%s",set[i]); if(i<cnt-1) printf(","); }
    printf("}");
}

void printResults() {
    printf("\n=== Reaching Definitions ===\n");
    printf("%-6s %-20s %-20s %-20s %-20s\n","Block","GEN","KILL","IN","OUT");
    printf("------------------------------------------------------------------------------------\n");
    for(int b=0;b<n_blocks;b++) {
        printf("B%-5d ",b);
        printDefSet(blocks[b].GEN,blocks[b].gen_cnt); printf("\t");
        printDefSet(blocks[b].KILL,blocks[b].kill_cnt); printf("\t");
        printDefSet(blocks[b].IN_RD,blocks[b].in_rd_cnt); printf("\t");
        printDefSet(blocks[b].OUT_RD,blocks[b].out_rd_cnt); printf("\n");
    }

    printf("\n=== Live Variables ===\n");
    printf("%-6s %-15s %-15s %-15s %-15s\n","Block","USE","DEF","IN","OUT");
    printf("-------------------------------------------------------------------\n");
    for(int b=0;b<n_blocks;b++) {
        printf("B%-5d ",b);
        printVarSet(blocks[b].USE,blocks[b].use_cnt); printf("\t");
        printVarSet(blocks[b].DEF,blocks[b].def_vars_cnt); printf("\t");
        printVarSet(blocks[b].IN_LV,blocks[b].in_lv_cnt); printf("\t");
        printVarSet(blocks[b].OUT_LV,blocks[b].out_lv_cnt); printf("\n");
    }
}

int main() {
    printf("=== Global Data Flow Analysis ===\n\n");
    printf("Using a sample 5-block CFG:\n");
    printf("  B0: a=1; b=2           -> B1\n");
    printf("  B1: c=a+b; a=c         -> B2, B3\n");
    printf("  B2: b=a-1              -> B4\n");
    printf("  B3: c=b*2              -> B4\n");
    printf("  B4: a=b+c              -> (end)\n\n");

    buildSampleCFG();
    reachingDefinitions();
    liveVariables();
    printResults();

    printf("\n=== Definitions ===\n");
    for(int i=0;i<def_count;i++) printf("  d%d: %s defined in B%d\n",i,all_defs[i].var,all_defs[i].block);
    return 0;
}
