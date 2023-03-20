import socket, json

def main():
    flag = 0
    while True:
        # recv 受け取りサイズ(1024bit)
        ans = (soc_c.recv(1024)).decode()
        if ans == "exit":
            break
        elif ans == "regist":
            register(soc_c)
            continue
        elif ans == "auth":
            auth(soc_c)
    soc_c.close()
    soc_s.close()

def register(soc_c):
    global Ai
    # 初回認証情報
    msg = soc_c.recv(1024)
    data = json.loads(msg.decode())
    Ai = data.get("A")
    print("初回登録情報の受信")
    print("A: ", Ai, "\n")

def auth(soc_c):
    global Ai
    msg = soc_c.recv(1024)
    data = json.loads(msg.decode())
    a = data.get("a")
    b = data.get("b")
    print("認証情報と認証子の受信")
    print("a: ", a)
    print("b: ", b)
    A_plus = format((int(a,16) - int(Ai,16)) ^ int(Ai,16), 'x')
    g = format(int(Ai,16) + int(A_plus, 16), 'x')
    print("b: ", b)
    print("g: ", g)
    if b == g:
        print("Success")
        Ai = A_plus
    else:
        print("Failure")

if __name__ == "__main__":
    localhost = "127.0.0.1"
    soc_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    soc_s.bind((localhost, 8080))
    soc_s.listen(5)
    soc_c, c_address = soc_s.accept()
    print(c_address, "に接続しました")
    main()
