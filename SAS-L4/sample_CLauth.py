import ctypes, socket, json, time

lib = ctypes.cdll.LoadLibrary('./libSASLotp.so')

class sIFInfo(ctypes.Structure):
    _fields_ = [("Alp", ctypes.c_ulonglong),
                ("Bet", ctypes.c_ulonglong),
                ("Gam", ctypes.c_ulonglong)]

class sCLInfo(ctypes.Structure):
    _fields_ = [("A", ctypes.c_ulonglong),
                ("M", ctypes.c_ulonglong),
                ("LastAlp", ctypes.c_ulonglong)]

# 関数の引数の型を指定する
#lib.example_function.argtypes = [ctypes.c_int, ctypes.c_double]

# int CLgenInfo(sCLInfo *clp, sIFInfo *ifp);
CLgenInfo = lib.CLgenInfo
CLgenInfo.argtypes = [ctypes.POINTER(sCLInfo), ctypes.POINTER(sIFInfo)]
CLgenInfo.restype =ctypes.c_int

# int CLchkAuth(sCLInfo *clp,sIFInfo *ifp);
CLchkAuth = lib.CLchkAuth
CLchkAuth.argtypes = [ctypes.POINTER(sCLInfo), ctypes.POINTER(sIFInfo)]
CLchkAuth.restype =ctypes.c_int


# ullong str2ull(char *str);
str2ull = lib.str2ull
str2ull.argtypes = [ctypes.c_char_p]
str2ull.restype = ctypes.c_ulonglong


# 関数の戻り値の型を指定する
sif = sIFInfo()
clif = sCLInfo()

def main():
    # コマンドを入力して認証処理を行う
    while True:
        # コマンド入力受付
        print("コマンドを入力してください->")
        ans = input()
        # 入力されたコマンドをサーバに送信
        serversocket.send(bytes(ans,"utf-8"))
        #処理が高速なせいか、重複して送信されるため少し待つ
        time.sleep(0.001)
         # 入力コマンドが"auth"の場合
        if ans == "auth":
            # 認証フェーズ
            # 認証メソッドの呼び出し
            # ファイルからの読み込み
            print("IDを入力してください->")
            ID = input()
            data = json.dumps({"ID": ID})
            serversocket.send(data.encode())
            with open('CLinfo_py.txt', 'r') as f:
                clif.A = int(f.readline())
                clif.M = int(f.readline())
                clif.LastAlp = int(f.readline())
            
            msg = serversocket.recv(1024)
            # JSON形式として読み込む
            data = json.loads(msg.decode())
            # 認証情報を保存
            sif.Alp = data.get("a")
            print("認証子αの受け取り:",format(sif.Alp,'x'))
            if CLgenInfo(clif,sif) == 0:
                data = json.dumps({"B": sif.Bet})
                serversocket.send(data.encode())
                print("認証子Betの生成と送信:",format(sif.Bet,'x'))
                msg = serversocket.recv(1024)
                # JSON形式として読み込む
                data = json.loads(msg.decode())
                # 認証情報を保存
                sif.Gam = data.get("g")
                print("相互認証用γの受け取り:",format(sif.Gam,'x'))
                if CLchkAuth(clif,sif) == 0:
                    print("サーバ認証成功\n")
                elif CLchkAuth(clif,sif) == 1:
                    print("サーバ認証失敗\n")
                else:
                    print("同期ズレが検知されたため、もう一度認証を行う\n")
            elif CLgenInfo(clif,sif) == 1:
                print("認証失敗")

            # ファイルへの書き込み
            with open('CLinfo_py.txt', 'w') as f:
                f.write(str(clif.A) + '\n')
                f.write(str(clif.M) + '\n')
                f.write(str(clif.LastAlp) + '\n')
        # 入力コマンドが"quit"の場合
        elif ans == "quit":
            break
        # 入力コマンドが"regist","auth","quit"のいずれでもない場合
        else:
            print("適切なコマンドを入力してください")
            print("regist or auth or quit")
    # ソケットを閉じる
    serversocket.close()

# mainの実行
if __name__ == "__main__":
    # 送信先サーバのIPアドレスの指定
    ip_address = "127.0.0.1"
    #ソケットの作成
    # IPv4, TCPで指定
    serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # IPアドレスとポート番号を指定しソケットの接続
    serversocket.connect(("127.0.0.1", 1235))
    # mainメソッドの呼び出し
    main()