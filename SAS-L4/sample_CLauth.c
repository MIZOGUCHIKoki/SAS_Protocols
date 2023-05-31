#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "SASLotp.h"

int main (void) {
    sIFInfo sif = {0, 0, 0};
    sCLInfo clif = {0, 0, 0}; 
    int sockfd, nbytes;
    char buf[BUFSIZ];
    char *mesg = "user0123\n";
    struct sockaddr_in servaddr;
    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        perror ("socket"); exit (1);
    }
    memset (&servaddr, 0, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons (10000);
    /* IP address of localhost is 127.0.0.1 */
    if (inet_pton (AF_INET, "127.0.0.1", &servaddr.sin_addr) < 0) {
        perror ("inet_pton"); exit (1);
    }
    if (connect (sockfd, (struct sockaddr *)&servaddr, sizeof (servaddr))
        < 0) {
        perror ("connect"); exit (1);
    }

    ullong tmp[2] = {0,0}; 
    FILE *fp_in, *fp_out;
    //認証フェーズ
    for(int au = 0; au < 10; au++){
        printf("\n-----%d回目-----\n\n",au+1);
        //ID送信
        printf("IDを送信し認証要求-> %s\n",mesg);
        nbytes = write (sockfd, mesg, strlen (mesg));

        fp_in = fopen("CLinfo.txt", "r");
        fscanf(fp_in, "%llu %llu %llu\n", &clif.A, &clif.M, &clif.LastAlp);
        fclose(fp_in);
        
        //認証情報αの受け取り
        printf("A: %llu\n", clif.A);
        printf("M: %llu\n",clif.M);
        tmp[0] = clif.A;
        tmp[1] = clif.M;
        printf("認証情報受信α-> ");
        if (recv(sockfd, &sif.Alp, sizeof(&sif.Alp), 0) < 0) {
            perror("Error receiving data");
        }
        printf("α:%llu\n",sif.Alp);

        //βの生成と送信
        if(CLgenInfo(&clif, &sif) == 0){
            printf("認証情報生成&送信-> ");
            printf("β:%llu\n",sif.Bet);
            send (sockfd, &sif.Bet, sizeof(&sif.Bet), 0);
            if (recv(sockfd, &sif.Gam, sizeof(&sif.Gam), 0) < 0) {
                perror("Error receiving data");
            }
            
            //サーバ認証
            printf("γ:%llu\n",sif.Gam);
            if (CLchkAuth(&clif, &sif) == 0){
                printf("認証成功!\n");
                if(au == 0){
                    printf("次回同期ズレを試します\n");
                    clif.A = tmp[0];
                    clif.M = tmp[1];
                }else if(au == 3){
                    printf("次回同期ズレを試します\n");
                    clif.A = tmp[0];
                    clif.M = tmp[1];
                }else if(au == 7){
                    printf("次回同期ズレを試します\n");
                    clif.A = tmp[0];
                    clif.M = tmp[1];
                }
            }else if(CLchkAuth(&clif, &sif) == 2){
                printf("リトライ要求を受け付けました.\nもう一度認証を行います.\n");
            }else{
                printf("認証失敗\n");
            }
        }else if (CLgenInfo(&clif, &sif) == 1){
            perror("前回認証情報と同じ");
        }
        // 構造体をファイルに書き込む
        fp_out = fopen("CLinfo.txt", "w");
        fprintf(fp_out, "%llu %llu %llu\n", clif.A, clif.M, clif.LastAlp);
        fclose(fp_out);
    }
    close (sockfd);
    return 0;
}