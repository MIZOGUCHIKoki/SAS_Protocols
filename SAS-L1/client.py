import socket, json, random, hashlib
# 文字コードを ASCII に変換
def to_ascii(text): 
    ascii_values=[ord(character) for character in text] # リスト内包表記
    return sum(ascii_values)

def main():
    while True:
        ans = input("Cmd>> ")
        soc_s.send(bytes(ans,"utf-8"))
        if ans == "exit":
            break
        elif ans == "regist":
            register(ID,S, soc_s)
            continue
        elif ans == "auth":
            auth(ID,S,soc_s)
    soc_s.close()

def register(ID,S,soc_s):
    global Ai, N
    N = random.getrandbits(256)
    S2 = to_ascii(S)
    Ai = hashlib.sha256((ID + str(S2 ^ N)).encode()).hexdigest() # hexdigest で16進数へ
    print("初回認証情報の送信")
    print("A: ",Ai, "\n")    
    data = json.dumps({"A": Ai})
    soc_s.send(data.encode())

def auth(ID, S, soc_s):
    global Ai, N_plus, A_plus, a, b
    N_plus = random.getrandbits(256)
    S2 = to_ascii(S)
    A_plus = hashlib.sha256((ID + str(S2 ^ N_plus)).encode()).hexdigest()
    print("Ai", Ai)
    print("Ai+1", A_plus)
    a = format((int(Ai,16) ^ int(A_plus,16)) + int(Ai,16), "x")
    b = format(int(Ai,16) + int(A_plus,16), "x")
    print()
    print("a: ", a)
    print("b: ", b)
    data = json.dumps({"a": a, "b": b})
    soc_s.send(data.encode())
    Ai = A_plus
# main の実行
if __name__ == "__main__":
    localhost = "127.0.0.1"
    # ソケットの作成
    soc_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # 接続
    soc_s.connect((localhost, 8080))
    ID = "User"
    S = "Pass"
    main()