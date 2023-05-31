import ctypes,socket, json

lib = ctypes.cdll.LoadLibrary('./libSASLotp.so')

class sIFInfo(ctypes.Structure):
    _fields_ = [("Alp", ctypes.c_ulonglong),
                ("Bet", ctypes.c_ulonglong),
                ("Gam", ctypes.c_ulonglong)]
    
class sSVInfo(ctypes.Structure):
    _fields_ = [("ID", ctypes.c_ulonglong),
                ("A", ctypes.c_ulonglong),
                ("M", ctypes.c_ulonglong),
                ("B", ctypes.c_ulonglong),
                ("N", ctypes.c_ulonglong),
                ("Alp", ctypes.c_ulonglong),
                ("Bet", ctypes.c_ulonglong),]
# 関数の引数の型を指定する
#lib.example_function.argtypes = [ctypes.c_int, ctypes.c_double]

# 構造体のポインタを受け取る関数を定義
# void SVgenReg(ullong id, sSVInfo *svp);
SVgenReg = lib.SVgenReg
SVgenReg.argtypes = [ctypes.c_ulonglong, ctypes.POINTER(sSVInfo)]
SVgenReg.restype = None

# void SVgenInfo(sSVInfo *svp, sIFInfo *ifp);
SVgenInfo = lib.SVgenInfo
SVgenInfo.argtypes = [ctypes.POINTER(sSVInfo), ctypes.POINTER(sIFInfo)]
SVgenInfo.restype = None

# int SVchkAuth(sSVInfo *svp, sIFInfo *ifp);
SVchkAuth = lib.SVchkAuth
SVchkAuth.argtypes = [ctypes.POINTER(sSVInfo), ctypes.POINTER(sIFInfo)]
SVchkAuth.restype =ctypes.c_int

# ullong str2ull(char *str);
str2ull = lib.str2ull
str2ull.argtypes = [ctypes.c_char_p]
str2ull.restype = ctypes.c_ulonglong


# 関数の戻り値の型を指定する
sif = sIFInfo()
svif = sSVInfo()

def main():
    # クライアントからコマンドを受け取り認証処理を行う
    while True:
        # クライアントからコマンドを受け取る
        print("コマンドを受け付けます")
        ans = (clientsocket.recv(1024)).decode()
        print("受信コマンド->",ans)
        # 受信コマンドが"auth"の場合
        if ans == "auth":
            # ファイルからの読み込み
            with open('SVinfo_py.txt', 'r') as f:
                svif.ID = int(f.readline())
                svif.A = int(f.readline())
                svif.M = int(f.readline())
                svif.B = int(f.readline())
                svif.N = int(f.readline())

            msg = clientsocket.recv(1024)
            # JSON形式として読み込む
            data = json.loads(msg.decode())
            # 認証情報を保存
            ID = data.get("ID")
            print("ID:",str2ull(bytes(ID.encode())),"\n")
            print("ID",svif.ID,"\n")
            if svif.ID != str2ull(bytes(ID.encode())):
                print("IDが違います")
                break
            SVgenInfo(svif,sif)
            print("認証子αの生成と送信->α:",format(sif.Alp,'x'))
            data = json.dumps({"a": sif.Alp})
            clientsocket.send(data.encode())

            msg = clientsocket.recv(1024)
            # JSON形式として読み込む
            data = json.loads(msg.decode())
            # 認証情報を保存
            sif.Bet = data.get("B")
            print("サーバで生成した認証子β:",format(svif.Bet,'x'))
            print("受信した認証子Bet:",format(sif.Bet,'x'))
            if SVchkAuth(svif,sif)== 0:
                print("クライアント認証成功")
            elif SVchkAuth(svif,sif)== 1:
                print("クライアント認証失敗")
            else:
                print("同期ズレ検知")
                print("γ:",format(sif.Gam,'x'))
            data = json.dumps({"g": sif.Gam})
            clientsocket.send(data.encode())
            print("相互認証用認証子γの生成と送信->:",format(sif.Gam,'x'))

            with open('SVinfo_py.txt', 'w') as f:
                f.write(str(svif.ID) + '\n')
                f.write(str(svif.A) + '\n')
                f.write(str(svif.M) + '\n')
                f.write(str(svif.B) + '\n')
                f.write(str(svif.N) + '\n')
        # 受信コマンドが"quit"の場合
        elif ans == "quit":
            break
        # 入力コマンドが"regist","auth","quit"のいずれでもない場合
        else:
            print("適切なコマンドを入力してください")
            print("auth or quit")
    # ソケットを閉じる
    clientsocket.close()


# mainの実行
if __name__ == "__main__":
    # サーバとして公開するIPアドレスの指定
    ip_address = "127.0.0.1"
    # ソケットの作成
    # IPv4, TCPで指定
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # IPアドレスとポート番号をバインドする
    s.bind((ip_address, 1235))
    # ソケットに接続するクライアントの待ちキューの作成(今回の処理にはあまり関係ない)
    s.listen(5)
    # 接続したクライアントの情報を保存する
    clientsocket, address= s.accept()
    print(address,"に接続しました")
    # メインメソッドの呼び出し
    main()