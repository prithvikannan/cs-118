#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <netinet/in.h> 
#include <sys/types.h>   
#include <sys/socket.h>  
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

const int REQUEST_MAX = 8192;
const int PATH_MAX = 4096;
const int EXTENSION_MAX = 5;

// error handling for when a bad request is sent
void badRequest (int newsockFd) {
	dprintf(newsockFd, "HTTP/1.1 404 Not Found\nContent-Length: 13\nContent-Type: text/html\n\n<h1>404 Not Found</h1>");
	close(newsockFd);
}

int main(int argc, char *argv[])
{
	int sockFd, newsockFd;
    int port;
	socklen_t client;
	struct sockaddr_in serverAddress, clientAddress;
	struct stat s;

	// confirms that the program is started with only a port argument
	if (argc != 2) {
		fprintf(stderr,"Error: correct usage is ./server [port]\n");
        exit(1);
	}

	port = atoi(argv[1]);

	// create a socket using ipv4
	sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockFd == -1) {
		fprintf(stderr,"Error: unable to create socket\n");
		exit(1);
    }

	// sets serverAddress fields
	memset((char *)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(port);

	// binds socket to localhost:port
	if (bind(sockFd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
		fprintf(stderr,"Error: unable to bind\n");
		exit(1);
    }

	// listens for up to 3 connections at once
	listen(sockFd, 3);

	// server runs forever
	while (1) {

		// accepts connection request as a new file descriptor
		newsockFd = accept(sockFd, (struct sockaddr *) &clientAddress, &client);
		if (newsockFd == -1) {
			fprintf(stderr,"Error: could not accept incoming connection\n");
			exit(1);
        }

		// buffer to hold the HTTP request
		char request[REQUEST_MAX];
		memset(request, 0, REQUEST_MAX);

		int r = read(newsockFd, request, REQUEST_MAX);
		if (r == -1) {
			fprintf(stderr,"Error: Failed to read from client.\n");
			exit(1);
        }

		fprintf(stdout, "%s", request);

		char pathString[PATH_MAX];
		memset(pathString, 0, sizeof(pathString));
		int pathIdx = 0;
		char extensionString[EXTENSION_MAX];
		memset(extensionString, 0, sizeof(extensionString));
		int extensionIdx = 0;
		int extensionBool = 0;
		int k = 5;

		// parse the request to get the desired file name and extension
		while (k < 4096) {
			if (request[k] == ' ') { 
				break; 
			} else if (request[k] == '.') { 
				extensionBool = 1; 
			} else if (extensionBool) {
				extensionString[extensionIdx] = request[k];
				extensionIdx++;
			}
			if (strncmp(&request[k], "%20", 3) == 0) {
				pathString[pathIdx] = ' ';
				pathIdx++;
				k = k + 2;
			} else {
				pathString[pathIdx++] = request[k];
			}

			k++;
		}

		pathString[pathIdx++] = '\0';
		extensionString[extensionIdx++] = '\0';

		char ext = 0;
		int html = 0, jpg = 0, gif = 0;
		if ((strcasecmp(extensionString, "html") == 0) || (strcasecmp(extensionString, "htm") == 0)) {
			ext = 'h';
		} else if ((strcasecmp(extensionString, "jpeg") == 0) || (strcasecmp(extensionString, "jpg") == 0)) {
			ext = 'j';
		} else if (strcasecmp(extensionString, "png") == 0) {
			ext = 'p';
		} else if (strcasecmp(extensionString, "txt") == 0) {
			ext = 't';
		}

		// open desired file
		FILE* f = fopen(pathString, "r");
		if (f == 0) {
			badRequest(newsockFd);
			continue;
		}

		stat(pathString, &s);
		int size = s.st_size;
		int slen = (ceil(log10(size)) + 21);

		// send output to the client with appropriate headers
		dprintf(newsockFd, "HTTP/1.1 200 OK\n");
		dprintf(newsockFd, "Content-Length: %d\n", size);
		switch (ext) {
		case 'h':
			dprintf(newsockFd, "Content-Type: text/html\r\n\0");
			break;
		case 't':
			dprintf(newsockFd, "Content-Type: text/plain\r\n\0");
			break;
		case 'j':
			dprintf(newsockFd, "Content-Type: image/jpeg\r\n\0");
			break;
		case 'p':
			dprintf(newsockFd, "Content-Type: image/png\r\n\0");
			break;
		default:
			dprintf(newsockFd, "Content-Type: application/octet-stream\r\n\0");
			break;
		}

		dprintf(newsockFd, "\r\n\0");
		
		// send the contents of the file
		char tempbuf[REQUEST_MAX];
		int fileFd = fileno(f);

		while (1) {
			int x = read(fileFd, tempbuf, sizeof tempbuf);
			switch (x) {
			case 0:
				break;
			case -1:
				fprintf(stderr,"Error: unable to send file\n");
				exit(1);
				break;
			default:
				send(newsockFd, tempbuf, x, 0);
				break;
			}
			if (x == 0) break;
		}
		close(newsockFd);
	}
	close(sockFd);
	return 0;
}