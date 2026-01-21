from sys import stderr

usage: str = ""
usage += "\n\n-----------------------------\n"
usage += "Commands:\n"
usage += "PASS [password]\n"
usage += "PING\n"
usage += "PONG\n"
usage += "NICK [nick]\n"
usage += "USER [user]\n"
usage += "QUIT\n"
usage += "-----------------------------\n"

crlf = "\r\n"


def main():
    while True:
        print(usage, file=stderr)
        response = input()
        args = response.split(" ")
        args[0] = args[0].upper()
        argc = len(args)
        if args[0] == "PASS" and argc >= 2:
            print(f"PASS {args[1]}", end=crlf)
        elif args[0] == "PING":
            print("PING localhost", end=crlf)
        elif args[0] == "PONG":
            print("PONG localhost", end=crlf)
        elif args[0] == "NICK" and len(args) >= 2:
            print(f"NICK {args[1]}", end=crlf)
        elif args[0] == "USER" and len(args) >= 2:
            print(f"USER * : {args[1]}", end=crlf)
        elif args[0] == "QUIT" or args[0] == "Q":
            print("QUIT Bye!", end=crlf)
            return


if __name__ == "__main__":
    main()
