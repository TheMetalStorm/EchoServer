# Experimenting with Sockets

## Echo Server & Client
TCP Echo Server and Client, written in C. Server can handle multiple client connections concurrently by using epoll(). 

### Build
- ```cd ./EchoServer```
- run ```make```

### Usage
- ```cd ./EchoServer```
- run ```./server```
- run (up to 128) ```./client``` instances
  
Enjoy the echo

## Chat Server & Client (WIP)
TCP Chat Server and Client, written in C. Server can handle multiple client connections concurrently by using epoll(). 
Use the included Client to have a nice chat experience, alternatively netcat works as well.

### Build
- ```cd ./Chat```
- run ```make```

### Usage
- ```cd ./Chat```
- run ```./server <PORT>```
- run (up to 128) ```./client <SERVER-IP> <PORT>``` instances (or use ```nc <SERVER-IP> <PORT>```)
  

# Disclaimer!
This code is probably unsafe and should not be used by anyone ever, except to learn the basics of using Sockets in Linux.
