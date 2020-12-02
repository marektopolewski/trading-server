A simple C++-based application that models a risk trading server.

The implementation consists of a server and a client.

The communcation is via TCP sockets. It is bidrectional and one-to-many (one server, mutiple clients).

To build:
```
makedir build
cd build
cmake ..
make all
```

To run test:
```
./test/test
```

To run the server:
```
./server/server
```

To run the client:
```
./client/client
```
