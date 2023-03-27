import socket, json


def main():
    while True:
        r_cmd = (soc_s.recv(1024)).decode()
        if r_cmd == "exit":
            break
        elif r_cmd == "regist":
            regist(serversocket=soc_s)
        elif r_cmd == "auth":
            auth(serversocket=soc_s)
    soc_s.close()
    soc_c.close()


def regist(serversocket):
    global Ai
    msg = serversocket.recv(1024)
    data = json.loads(msg.decode())
    Ai = data.get("A")
    print("初回認証情報 受信\nAi = ", Ai, "\n")


def auth(serversocket):
    global Ai
    msg = serversocket.recv(1024)
    data = json.loads(msg.decode())
    a = data.get("a")
    print("受信済: a <<-\na = ", a)
    A_plus = format(int(a, 16) ^ int(Ai, 16), "x")
    b = format(int(A_plus, 16) + int(Ai, 16), "x")
    data = json.dumps({"b": b})
    serversocket.send(data.encode())
    print("送信済: b ->>")
    Ai = A_plus


if __name__ == "__main__":
    localhost = "127.0.0.1"
    soc_c = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    soc_c.bind((localhost, 8080))
    soc_c.listen(5)
    soc_s, s_address = soc_c.accept()
    print("Server ( ", s_address, " ) に接続されました")
    main()
