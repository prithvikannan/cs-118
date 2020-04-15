# Project 1: Web Server Implementation

Name: Prithvi Kannan

Email: prithvi.kannan@gmail.com

UID: 405110096

## Usage

Run `make` to compile `server.c` into an executable called `server`.

To start the server, run `./server [port]`. To make a request, using a web browser, type `[host]:[port]/[file.ext]` and the server will send that file to the browser. Requests will be logged to the server.

The server will send files that are stored on the server-side the following forms: `*.html, *.txt, *.jpeg, *.jpg, *.gif, binary`.

## Design

The code can be broken up into two parts.

Setup:

- Parameter Checking: Ensure that the server inputs are correct by verifying that a port is given.
- Setup: Create a socket using the `socket` system call and attach to input port with `bind`.

Running:

- Listening: Wait for incoming requests to the socket file descriptor. When a request comes in, accept it.
- Parsing request: Look through the request and identify the file name and file type.
- Sending request: If the file is valid, open the file. Send the appropriate header containing the a `200 OK`, the file length, and type. Then send the contents to the client. 

## Challenges

When testing my code, I kept running to `404 error` when I used Chrome as the client, due to Chrome forcing requests to be in HTTPS form, rather than HTTP. I solved this issue by using Firefox instead.

Since I am working on a Windows system, I have used a Beaglebone as a Linux server. I ssh into the board to start the server and view the log. When testing the client, I have to use the IP address of the board, `192.168.7.2`, as the host, rather than localhost as described in the spec.

## References

1. Sockets tutorial http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
2. HTTP header formatting https://www.w3.org/Protocols/HTTP/HTRQ_Headers.html
3. `dprinf(3)` documentation https://linux.die.net/man/3/dprintf
4. `stat(2)` documentation http://man7.org/linux/man-pages/man2/stat.2.html
5. `read(2)` documentation http://man7.org/linux/man-pages/man2/read.2.html
6. jpg vs. jpeg https://kinsta.com/blog/jpg-vs-jpeg/