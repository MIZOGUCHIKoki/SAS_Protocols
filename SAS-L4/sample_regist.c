#include <stdio.h>
#include "SASLotp.h"

int main (void) {
    sIFInfo sif = {0, 0, 0};
    sCLInfo clif = {0, 0, 0}; 
	sSVInfo svif = {0, 0, 0, 0, 0, 0, 0};
    int listenfd, connfd;
    char *str = "user0123";
    ullong id =  str2ull(str);
    FILE *fp_out;
    
    //登録フェーズ
    SVinitSASL();
    SVgenReg(id,&svif);
    clif.A = svif.A;
    clif.M = svif.M;
    printf("登録フェーズ\nA: %llu\n", svif.A);
    printf("M: %llu\n", svif.M);
    
    // 構造体をファイルに書き込む
    fp_out = fopen("SVinfo.txt", "w");
    fprintf(fp_out, "%llu %llu %llu %llu %llu %llu %llu\n", svif.ID, svif.A, svif.M, svif.B, svif.N, svif.Alp, svif.Bet);
    fclose(fp_out);

    // 構造体をファイルに書き込む
    fp_out = fopen("CLinfo.txt", "w");
    fprintf(fp_out, "%llu %llu %llu\n", clif.A, clif.M, clif.LastAlp);
    fclose(fp_out);
}