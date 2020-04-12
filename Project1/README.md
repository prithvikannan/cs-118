# Project 1: Web Server Implementation

Name: Prithvi Kannan

Email: prithvi.kannan@gmail.com

UID: 405110096

## Usage
Run `make` to compile `server.c` into an executable called `server`. 

To start the server, run `./server [port]`.  To make a request, using a web browser, type `[host]:[port]/[file.ext]` and the server will send that file to the browser. Requests will be logged to the server.

## Design

## Challenges
When testing my code, I kept running to `404 error` when I used Chrome as the client, due to Chrome forcing requests to be in HTTPS form, rather than HTTP. I solved this issue by using Firefox instead.

Since I am working on a Windows system, I have used a Beaglebone as a Linux server. I ssh into the board to start the server and view the log. When testing the client, I have to use the IP address of the board, `192.168.7.2`, as the host, rather than localhost as described in the spec. 

## References
