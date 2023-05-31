#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "SASLotp.h"

int main (void) {
    sIFInfo sif = {0, 0, 0};
	sSVInfo svif = {0, 0, 0, 0, 0, 0, 0};
    int listenfd, connfd;
    char buf[BUFSIZ];
    struct sockaddr_in servaddr;
    if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        perror ("socket"); exit (1);
    }
    memset (&servaddr, 0, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons (10000); /* echo port 7 is reserved */
    servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
    if (bind (listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr))
        < 0) {
        perror ("bind"); exit (1);
    }
    if (listen (listenfd, 5) < 0) {
        perror ("listen"); exit (1);
    }
    if ((connfd = accept (listenfd, (struct sockaddr *)NULL, NULL)) < 0) {
        perror ("accept"); exit (1);
    }
    fprintf (stderr, "accept!\n");
    //SAS-L認証を行う
    //認証フェーズ
    SVinitSASL();
    char *str = "user0123";
    ullong id =  str2ull(str);
    FILE *fp_in, *fp_out;

    for (int au = 0; au < 10; au++) {
        printf("\n-----%d回目-----\n\n",au+1);
        //IDの受け取り
        printf("\nIDの受け取り-> ");
        if ((recv(connfd, buf, sizeof (buf), 0)) < 0) {
            perror("Error receiving data");
        }
        // ファイルから構造体を読み込む
        fp_in = fopen("SVinfo.txt", "r");
        fscanf(fp_in, "%llu %llu %llu %llu %llu %llu %llu\n", &svif.ID, &svif.A, &svif.M, &svif.B, &svif.N, &svif.Alp, &svif.Bet);
        fclose(fp_in);

        printf("%s\n",buf);
        printf("A: %llu\n", svif.A);
        printf("M: %llu\n", svif.M);

        //認証情報αの生成と送信
        printf("認証情報生成&送信-> ");
        SVgenInfo(&svif, &sif);
        printf("α:%llu\n",sif.Alp);
        send (connfd, &sif.Alp, sizeof(&sif.Alp), 0);

        //βの受け取り
        printf("認証情報受信-> ");
        if (recv(connfd, &sif.Bet, sizeof(&sif.Bet), 0) < 0) {
            perror("Error receiving data");
        }
        printf("β:%llu\n",sif.Bet);
        printf("比較用B:%llu\n",svif.Bet);

        //クライアント認証
        if(SVchkAuth(&svif, &sif) == 0){
            printf("認証成功!相互認証を行います!-> ");
            printf("γ:%llu\n",sif.Gam);
            send (connfd, &sif.Gam, sizeof(&sif.Gam), 0);
        }else if(SVchkAuth(&svif, &sif) == 1){
            printf("認証失敗\n");
        }else{
            printf("同期ズレを検知.\n認証値の再設定\n");
            printf("A: %llu\n", svif.A);
            printf("M: %llu\n", svif.M);
            printf("Gam: %llu\n", sif.Gam);
            send (connfd, &sif.Gam, sizeof(&sif.Gam), 0);
        }

        // 構造体をファイルに書き込む
        fp_out = fopen("SVinfo.txt", "w");
        fprintf(fp_out, "%llu %llu %llu %llu %llu %llu %llu\n", svif.ID, svif.A, svif.M, svif.B, svif.N, svif.Alp, svif.Bet);
        fclose(fp_out);
    }
    close (connfd);
    fprintf (stderr, "close\n");
}