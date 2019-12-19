import socket
import threading
import os


class Server:
    _sock = None
    _running = False
    _connections = []

    def __init__(self):
        self._sock = socket.socket()
        self._sock.bind(("0.0.0.0", 8777))
        self._sock.listen(1)

        self._running = True

    def Run(self):
        # Run thread for checking if there is an incoming connection
        conThread = threading.Thread(target=self.ConnectionHandler)
        conThread.daemon = True
        conThread.start()

        # Have the master thread monitor the console.
        self.Console()

    def ConnectionHandler(self):
        while self._running:
            client, addr = self._sock.accept()
            cThread = threading.Thread(target=self.ClientHandler,
                                       args=(client, addr))
            cThread.daemon = True
            cThread.start()
            self._connections.append(client)
            print(str(addr[0]) + ":", str(addr[1]), "connected")

    def ClientHandler(self, client, addr):
        inc = 0
        args = []

        while self._running:
            try:
                data = client.recv(1024)
                if data:
                    if inc == 0:
                        args.append(data.decode("utf-8"))
                    elif inc == 1:
                        args.append(int.from_bytes(data, "big"))

                    print(args[inc])
                    inc += 1


                    # TODO send back server config settings.
                    # client.send(bytes("hello", "utf-8"))
                    # client.send(bytes(data))
                else:
                    print(str(addr[0]) + ":", str(addr[1]), "disconnected")
                    self._connections.remove(client)
                    client.close()
                    return
            except:
                print(str(addr[0]) + ":", str(addr[1]), "disconnected")
                self._connections.remove(client)
                client.close()
                return

    def Console(self):
        while self._running:
            con = input()
            if con == "exit":
                self._running = False


if __name__ == "__main__":
    try:
        server = Server()
        server.Run()
    except (KeyboardInterrupt, SystemExit):
        os._exit(0)
