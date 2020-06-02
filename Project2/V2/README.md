# Project 2: Simple Window-Based Reliable Data Transfer

Name: Prithvi Kannan

Email: prithvi.kannan@gmail.com

UID: 405110096

## Usage

Run `make` to compile `server.cpp` into an executable called `server` and `client.cpp` into an executable called `client`. Both `server.cpp` and `client.cpp` include header file `packet.h`.

To start the server, run `./server [port]`. In a separate terminal window, start a client with the following command `./client [hostname_or_ip] [port] [filename]`. The server supports multiple connections at once or consecutively, and will log the file each subsequent client sends as `1.file`, `2.file`, etc. 

## Design

The client and server start the connection with the TCP 3-way handshake, beginning with the client's SYN, then the server's SYNACK, and then the client's first message and ACK. The client reads as many bytes from the file as can fit into a packet at a time, and sends to the server. Upon recieving the packet, the server sends an ACK back to the client and stores the contents of the packet in a buffer. The client sends messages using pipelining, so that up to 10 packets can be in flight at a time. Once the client has reached the end of the file, it initiates the 4-way closing sequence, sending a FIN packet to the server. The server responds with an ACK and a FIN of it's own. All packets are monitored with a timer such that any packet that is not acknowledged in time is considered loss. Lost packets are handled using go-back-n (GBN) protocol, so any packets in flight after a lost packet will also be resent. The client program exits upon closing sequence but the server program stays running to accept new clients. 

## Challenges

For project 1, I did all of my development on my Beaglebone as a Linux server. However, for this project, I set up WSL on my laptop so I could actually program in an Ubuntu environment. This worked very well for normal things like compiling with `g++` or running `diff` to check my program outputs, but I was unable to set up `tc` in WSL. As a result, I was stalled on my packet loss code for a while, but thanks the the TA's I was able to use clumsy for this instead.

A less technical challenge that I encountered was programming in C++, which I had not done since CS32 last year. All of my recent programming has been in C, JavaScript, or Python, so I had to frequently look up documentation about C++ syntax and data structures.

## References

1. UDP tutorial https://www.geeksforgeeks.org/udp-server-client-implementation-c/
2. `unordered_set` documentation http://www.cplusplus.com/reference/unordered_set/unordered_set/find/
3. `rand` documentation http://www.cplusplus.com/reference/cstdlib/rand/
4. `compare` documentation http://www.cplusplus.com/reference/string/string/compare/
5. `ifstream` documentation http://www.cplusplus.com/reference/fstream/ifstream/
6. `perror` documentation http://www.cplusplus.com/reference/cstdio/perror/
7. VSCode debugger help https://stackoverflow.com/questions/39030053/debugging-c-built-with-a-makefile-in-visual-studio-code
8. TCP sequence numbers https://packetlife.net/blog/2010/jun/7/understanding-tcp-sequence-acknowledgment-numbers/
9. TCP pipelining algorithm https://www.cs.csustan.edu/~john/classes/previous_semesters/CS3000_Communication_Networks/2008_01_Winter/Notes/chap25.html
