#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "SASLotp.h"

//S関数
uchar sbox(uchar data){
	return ( data << 2 | data >> 6 );
}

//鍵スケジューリング
void makekey(uchar key[8],uchar KS[32]){
	uchar	t[4],b[4],d[4];
	int i,j;
	for ( i = 0 ; i < 4 ; i++ ){
    	d[i] = 0;
  	}
	for ( i = 0 ; i < 8 ; i++ ) {
		for ( j = 0 ; j < 4 ; j++ ){
			t[j] = key[j+4];
			b[j] = key[j+4] ^ d[j];
			d[j] = key[j];
		}
		key[5] = key[1] ^ key[0];
		key[6] = key[2] ^ key[3];
		key[5] = sbox(key[5] + (key[6] ^ b[0]) + 1);
		key[6] = sbox(key[6] + (key[5] ^ b[1]));
		key[4] = sbox(key[0] + (key[5] ^ b[2]));
		key[7] = sbox(key[3] + (key[6] ^ b[3]) + 1);
		for ( j = 0 ; j < 4 ; j++ ){
      		key[j] = t[j];
    	}
		for ( j = 0 ; j < 4 ; j++ ){
      		KS[4*i+j] = key[j+4];
    	}
	}
}

//暗号化
ullong cipher(uchar KS[32]){
	//平文を適当なパラメータにして暗号化
	uchar block[8] = "        ";
	uchar t[4];
	ullong hash = 0;
	int i,j,k;
	k = 16;
	for ( i = 0 ; i < 8 ; i++ ){
		block[i] = block[i] ^ KS[k++];
  	}
	for ( i = 0 ; i < 4 ; i++ ){
    	block[i+4] = block[i+4] ^ block[i];
  	}
	k = 0;
	for ( i = 0 ; i < 8 ; i++ ) {
		for ( j = 0 ; j < 4 ; j++ ){
      		t[j] = block[j+4];
    	}
		block[5] = block[5] ^ block[4] ^ KS[k];
		block[6] = block[6] ^ block[7] ^ KS[k+1];
		block[5] = sbox(block[5] + block[6] + 1);
		block[6] = sbox(block[6] + block[5]);
		block[4] = sbox(block[4] + block[5]);
		block[7] = sbox(block[7] + block[6] + 1);
		for ( j = 0 ; j < 4 ; j++ ){
      		block[j+4] = block[j+4] ^ block[j];
    	}
		for ( j = 0 ; j < 4 ; j++ ){
     		block[j] = t[j];
    	}
		k += 2;
	}
	for ( i = 0 ; i < 4 ; i++ ){
    	t[i] = block[i+4];
  	}
	for ( i = 0 ; i < 4 ; i++ ){
    	block[i+4] = block[i] ^ block[i+4];
  	}
	for ( i = 0 ; i < 4 ; i++ ){
    	block[i] = t[i];
  	}
	k = 24;
	for ( i = 0 ; i < 8 ; i++ ){
    	block[i] = block[i] ^ KS[k++];
 	}
	for ( i = 0; i < 8; i++ ) {
        hash <<= 8;
        hash |= (uchar) block[i];
    }
	return hash;
}

//FEAL一方向性関数
ullong onewayFEAL(uchar key[8]){
	ullong hash;
  	uchar KS[32];
	makekey(key,KS);
	hash = cipher(KS);
	return hash;
}

//サーバ側の初期化関数. 他の関数を使用する前に一度実行.
void SVinitSASL(void){
	//時間ベースの乱数生成のため、シード値を初期化
	ullong seed = time(NULL);
    srand(seed);
}

//サーバ側で初期認証情報を生成する関数.
void SVgenReg(ullong id, sSVInfo *svp){
	//乱数Nの生成
	uint random_num1 = rand();
    uint random_num2 = rand();
    ullong N = ((ullong)random_num1 << 32) | random_num2;
	//認証子Aの生成
	ullong S = id ^ N;
	uchar key[8]; // 格納する配列
	ull2str(S, key);
	ullong A = onewayFEAL(key);
	//乱数Mの生成
	random_num1 = rand();
    random_num2 = rand();
	ullong M = ((ullong)random_num1 << 32) | random_num2;
	//構造体へ保存
	svp->ID = id;
	svp->A = A;
	svp->M = M;
}

//クライアントへ送信する認証情報を生成する関数.
void SVgenInfo(sSVInfo *svp, sIFInfo *ifp){
	//乱数Ni+1の生成
	uint random_num1 = rand();
    uint random_num2 = rand();
    ullong N_plus = ((ullong)random_num1 << 32) | random_num2;
	//次回用認証子の生成
	ullong S = svp->ID ^ N_plus;
	uchar key[8]; // 格納する配列
	ull2str(S, key);
	ullong A_plus = onewayFEAL(key);
	//認証情報αとβの生成
	ullong a = svp->A ^ A_plus ^ svp->M;
	ullong b = svp->A + A_plus;
	//構造体への保存
	svp->Alp = a;
	svp->Bet = b;
	//共有の構造体へ保存
	ifp->Alp = a;
}

//クライアントから受信した認証情報βを元にサーバで認証し、認証情報γを生成する関数.
//戻り値として 0=認証OK
//1=認証NG
//2=リトライ要求を返す.
int SVchkAuth(sSVInfo *svp, sIFInfo *ifp){
	int ret;
	//サーバが生成した認証情報b
	ullong b = svp->Bet;
	//クライアントが生成した認証情報B
	ullong B = ifp->Bet;
	//認証情報bとBで比較し、クライアントを認証する
	if ( b == B ){
		//Ai→Ai+1, Mi→Mi+1のために生成
		ullong A = svp->A;
		ullong a = svp->Alp;
		ullong M = svp->M;
		ullong A_plus = A ^ a ^ M;
		ullong M_plus = A + M;
		//相互認証情報γの生成
		ullong g = b ^ M;
		//前回情報を保存
		svp->B = A;
		svp->N = M;
		//認証情報の更新
		svp->A = A_plus;
		svp->M = M_plus;
		ifp->Gam = g;
		//認証成功のため、0を返す
		ret = 0;
	//クライアントの認証情報が更新されていない場合の同期ズレ
	}else if( B == ( ( svp->Alp ^ svp->B ^ svp->N ) + svp->B ) ){
		//前回の認証情報に更新する
		svp->A = svp->B;
		svp->M = svp->N;
		ifp->Gam = svp->A - svp->M;
		ret = 2;
	}else{
		//認証失敗のため、1を返す
		ret = 1;
	}
	return ret;
}


//サーバから受信した認証子αを元に認証情報βを生成する関数.
//戻り値として、0 = OK
//1 = NG ・・・αが前回と同じ場合(使い捨て認証情報を流し込まれた場合)
int CLgenInfo(sCLInfo *clp, sIFInfo *ifp){
	int ret;
	ullong a = ifp->Alp;
	ullong A = clp->A;
	ullong M = clp->M;
	ullong LastAlp = clp->LastAlp;
	if( a == LastAlp ){
		ret = 1;
	}else{
		ullong C = A ^ a ^ M;
		ullong B = A + C;
		ifp->Bet = B;
		ret = 0;
	}
	return ret;
}

//サーバから受信した認証情報γを元にクライアントで認証確認する関数.
//戻り値として 0=認証OK
//1=認証NGを返す.
//2=リトライを返す.
int CLchkAuth(sCLInfo *clp,sIFInfo *ifp){
	int ret;
	//サーバが生成した認証情報γ
	ullong g = ifp->Gam;
	ullong G = ifp->Bet ^ clp->M;
	ullong B = (clp->LastAlp ^ clp->A ^ clp->M);//Ai+1
	//認証情報gとGで比較し、クライアントを認証する
	if ( g == G ){
		//Ai→Ai+1, Mi→Mi+1のために生成
		ullong A = clp->A;
		ullong a = ifp->Alp;
		ullong M = clp->M;
		ullong A_plus = A ^ a ^ M;
		ullong M_plus = A + M;
		//前回情報を保存
		clp->LastAlp = a;
		//認証情報の更新
		clp->A = A_plus;
		clp->M = M_plus;
		//認証成功のため、0を返す
		ret = 0;
	}else if ( ifp->Gam == ( clp->A - clp->M ) ){
		//同期ずれが発生しているため、2を返す
		ullong a = ifp->Alp;
		clp->LastAlp = a;
		ret = 2;
	}else{
		//認証失敗のため、1を返す
		ret = 1;
	}
	return ret;
}

//8バイトを文字列として受け取り、その文字列を数値に変換する関数.
ullong str2ull(char *str){
	ullong result = 0;
	int i;
    for ( i = 0; i < 8; i++ ) {
        result <<= 8;
        result |= (uchar) str[i];
    }
    return result;
}

//8バイトを文字列として受け取り、その文字列を数値に変換する関数.
void ull2str(ullong num, uchar *str){
    int i;
    for ( i = 7; i >= 0; i-- ) {
        str[i] = num & 0xFF;
        num >>= 8;
    }
}