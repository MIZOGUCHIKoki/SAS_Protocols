import socket, json, random, hashlib


def main():
    while True:
        cmd = input("cmd >>")


if __name__ == "__main__":
    localhost = "127.0.0.1"
    soc_c = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    soc_c.connect((localhost, 8080))
    ID = "user"
    S = "password"
    main()
