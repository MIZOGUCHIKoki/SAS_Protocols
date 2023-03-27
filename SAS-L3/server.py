import socket, random, hashlib, json


def to_ascii(text):
    ascii_values = [ord(character) for character in text]
    return sum(ascii_values)


def main():
    while True:
        cmd = input("cmd>> ")
        soc_s.send(bytes(cmd, "utf-8"))
        if cmd == "regist":
            regist(soc_s)
        elif cmd == "auth":
            auth(soc_s)
        elif cmd == "exit":
            break
    soc_s.close()


def regist(serversocket):
    global Ai, N
    N = random.getrandbits(256)
    Ai = hashlib.sha256(str(N).encode()).hexdigest()
    print("初回認証情報 送信\nAi = ", Ai, "\n")
    data = json.dumps({"A": Ai})
    serversocket.send(data.encode())


def auth(serversocket):
    global Ai
    N_plus = random.getrandbits(256)
    A_plus = hashlib.sha256(str(N_plus).encode()).hexdigest()
    print("次回認証情報 生成\nA_p = ", A_plus, "\n")
    a = format(int(Ai, 16) ^ int(A_plus, 16), "x")
    b = format(int(Ai, 16) + int(A_plus, 16), "x")
    print("認証子 生成\na: ", a, "\nb: ", b)
    data = json.dumps({"a": a})
    serversocket.send(data.encode())
    print("送信済: a ->>")
    msg = serversocket.recv(1024)
    data = json.loads(msg.decode())
    r_b = data.get("b")
    print("受信済: b <<-\nb = ", r_b)
    if b == r_b:
        print("\nSuccess\n")
    else:
        print("\nFailuren\n")
    Ai = A_plus


if __name__ == "__main__":
    localhost = "127.0.0.1"
    soc_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    soc_s.connect((localhost, 8080))
    main()
