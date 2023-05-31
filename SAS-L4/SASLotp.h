#ifndef SASLOTP_H
#define SASLOTP_H

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned long long int ullong;

//クライアントとサーバ間の送受信に使用する鍵情報を保持
typedef struct{
	ullong Alp;
	ullong Bet;
	ullong Gam;
} sIFInfo;

//クライアントの認証情報を保持
typedef struct{
	ullong A; //認証値A
	ullong M; //認証値M
	ullong LastAlp; // 前回のα
} sCLInfo;

//サーバの認証情報を保持します。
typedef struct{ 
	ullong ID;// クライアント識別子
	ullong A; // 認証情報 A
	ullong M; // マスク値M
	ullong B; // 認証情報 (前回の A)
	ullong N;  // マスク値 (前回の M)
	ullong Alp;  // 計算結果の α 
	ullong Bet; // 計算結果の β
} sSVInfo;

//関数のプロトタイプ宣言

//サーバ側関数
void SVinitSASL(void);
void SVgenReg(ullong id, sSVInfo *svp);
void SVgenInfo(sSVInfo *svp, sIFInfo *ifp);
int SVchkAuth(sSVInfo *svp, sIFInfo *ifp);
//クライアント側関数
int CLgenInfo(sCLInfo *clp, sIFInfo *ifp);
int CLchkAuth(sCLInfo *clp,sIFInfo *ifp);

//共通関数
ullong str2ull(char *str);
void ull2str(ullong num, uchar *str);

#endif