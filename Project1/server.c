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

	if (argc != 2) {
		fprintf(stderr,"Error: correct usage is ./server [port]\n");
        exit(1);
	}

	port = atoi(argv[1]);

	sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockFd == -1) {
		fprintf(stderr,"Error: unable to create socket\n");
		exit(1);
    }

	memset((char *)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(port);

	if (bind(sockFd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
		fprintf(stderr,"Error: unable to bind\n");
		exit(1);
    }

	listen(sockFd, 3);

	while (1) {
		newsockFd = accept(sockFd, (struct sockaddr *) &clientAddress, &client);

		if (newsockFd == -1) {
			fprintf(stderr,"Error: could not accept incoming connection\n");
			exit(1);
        }

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
		} else if (strcasecmp(extensionString, "gif") == 0) {
			ext = 'g';
		}

		FILE* f = fopen(pathString, "r");
		if (f == 0) {
			badRequest(newsockFd);
			continue;
		}

		stat(pathString, &s);
		int size = s.st_size;
		int slen = (ceil(log10(size)) + 21);

		dprintf(newsockFd, "HTTP/1.1 200 OK\n");
		dprintf(newsockFd, "Content-Length: %d\n", size);



		switch (ext) {
		case 'h':
			write(newsockFd, "Content-Type: text/html\n\n", 25);
			break;
		case 'j':
			write(newsockFd, "Content-Type: image/jpeg\n\n", 26);
			break;
		case 'g':
			write(newsockFd, "Content-Type: image/gif\n\n", 25);
			break;
		default:
			write(newsockFd, "Content-Type: application/octet-stream\n\n", 40);
			write(newsockFd, "Content-Disposition: attachment\n\n", 33);
			break;
		}
		
		// Max request length
		char tempbuf[REQUEST_MAX];
		int fileFd = fileno(f);

		// Persistent HTTP
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