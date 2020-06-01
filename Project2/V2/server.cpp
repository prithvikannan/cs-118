#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <thread>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <ctime>
#include <thread>
#include <iostream>
#include <map>

using namespace std;

#define FIN 1
#define SYN 2
#define ACK 4
#define FIN_ACK 5
#define SYN_ACK 6

#define MAX_CONNECTIONS 10

fd_set write_fds;
fd_set read_fds;
struct timeval tv;
struct timeval ts_tv;

unsigned int timeout = 10;
unsigned int packet_size = 524;
unsigned int data_size = 512;

static void sig_handler(int signum)
{
    exit(0);
}

struct header
{
    uint32_t seq;
    uint32_t ack;
    uint16_t id;
    uint16_t flags;
};

typedef struct header header;

struct packet
{
    header pack_header;
    char data[512];
};

typedef struct packet packet;

struct connection_info
{
    packet pack;
    struct sockaddr src_addr;
    socklen_t addrlen;
    ofstream myfile;
    int is_FIN;
    clock_t timestamp;
};

typedef struct connection_info connection_info;

struct stat info;

connection_info all_connection[MAX_CONNECTIONS];

void print_packet(string message, packet buf)
{
    string flag;
    switch (buf.pack_header.flags)
    {
    case 0:
        flag = "";
        break;
    case 1:
        flag = "FIN";
        break;
    case 2:
        flag = "SYN";
        break;
    case 4:
        flag = "ACK";
        break;
    case 5:
        flag = "FIN ACK";
        break;
    case 6:
        flag = "SYN ACK";
        break;
    }
    cout << message << " " << buf.pack_header.seq << " " << buf.pack_header.ack << " " << flag << endl;
}

packet convertByteOrder(connection_info input)
{
    packet buf = input.pack;
    buf.pack_header.seq = htonl(input.pack.pack_header.seq);
    buf.pack_header.ack = htonl(input.pack.pack_header.ack);
    buf.pack_header.id = htons(input.pack.pack_header.id);
    buf.pack_header.flags = htons(input.pack.pack_header.flags);
    return buf;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    signal(SIGQUIT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGINT, sig_handler);

    // create a socket using TCP IP
    string port_num, file_dir;

    port_num = argv[1];

    if (stoi(port_num) <= 1023 && stoi(port_num) >= 0)
    {
        perror("ERROR: incorrect port number");
        exit(1);
    }

    struct addrinfo hints, *server_info, *anchor;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port_num.c_str(), &hints, &server_info) != 0)
    {
        fprintf(stderr, "ERROR:getaddrinfo error");
        exit(1);
    }

    int sockfd;
    int yes = 1;
    for (anchor = server_info; anchor != NULL; anchor = anchor->ai_next)
    {
        sockfd = socket(anchor->ai_family, anchor->ai_socktype, anchor->ai_protocol);
        //fcntl(sockfd, F_SETFL, O_NONBLOCK);
        if (sockfd < 0)
        {
            continue;
        }

        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if (bind(sockfd, anchor->ai_addr, anchor->ai_addrlen) < 0)
        {
            close(sockfd);
            continue;
        }
        break;
    }

    if (anchor == NULL)
    {
        fprintf(stderr, "ERROR:bind fail");
        exit(2);
    }

    freeaddrinfo(server_info);

    struct sockaddr client_addr;
    socklen_t client_addrlen = 16;
    packet buf;

    while (1)
    {
        memset(&buf, 0, sizeof(buf));
        int recv_byte = recvfrom(sockfd, &buf, sizeof(buf), 0, &client_addr, &client_addrlen);
        int data_size = recv_byte - 12;
        // convert network byte order
        buf.pack_header.seq = ntohl(buf.pack_header.seq);
        buf.pack_header.id = ntohs(buf.pack_header.id);
        buf.pack_header.ack = ntohl(buf.pack_header.ack);
        buf.pack_header.flags = ntohs(buf.pack_header.flags);

        // log packet
        print_packet("RECV", buf);

        /*
        SYN received
        */
        if (buf.pack_header.flags == SYN)
        {

            for (int i = 0; i < MAX_CONNECTIONS; i++)
            {
                if (all_connection[i].src_addr.sa_family != client_addr.sa_family)
                {
                    if (all_connection[i].pack.pack_header.id == 0)
                    {
                        // setup file path
                        string file_path = to_string(i + 1) + ".file";
                        all_connection[i].myfile.open(file_path);

                        // set rand seq number, increment ack number, set flags
                        all_connection[i].pack.pack_header.seq = rand() % 25600;
                        all_connection[i].pack.pack_header.ack = buf.pack_header.seq + 1;
                        all_connection[i].pack.pack_header.flags = SYN_ACK;
                        all_connection[i].pack.pack_header.id = i + 1;
                        all_connection[i].src_addr = client_addr;
                        all_connection[i].addrlen = client_addrlen;

                        // convert to network byte order
                        buf.pack_header.seq = htonl(all_connection[i].pack.pack_header.seq);
                        buf.pack_header.ack = htonl(all_connection[i].pack.pack_header.ack);
                        buf.pack_header.id = htons(all_connection[i].pack.pack_header.id);
                        buf.pack_header.flags = htons(all_connection[i].pack.pack_header.flags);

                        // send packet
                        sendto(sockfd, &buf,
                               sizeof(buf), 0,
                               &all_connection[i].src_addr,
                               all_connection[i].addrlen);

                        // log packet
                        print_packet("SEND", all_connection[i].pack);
                        break;
                    }
                }
                else
                {
                    sendto(sockfd, &buf,
                           sizeof(buf), 0,
                           &all_connection[i].src_addr,
                           all_connection[i].addrlen);
                }
            }
        }
        /*
        ACK received
        */
        else if (buf.pack_header.flags == ACK || buf.pack_header.flags == 0)
        {

            for (int i = 0; i < MAX_CONNECTIONS; i++)
            {
                if (buf.pack_header.id == all_connection[i].pack.pack_header.id)
                { // find the connection
                    if (all_connection[i].is_FIN == 1)
                    { // connection has already been marked as FIN
                        all_connection[i].myfile.close();
                        break;
                    }
                    if (buf.pack_header.seq == all_connection[i].pack.pack_header.ack)
                    { // packet is in the correct order
                        all_connection[i].pack.pack_header.ack += data_size;
                        all_connection[i].pack.pack_header.ack %= 102401;

                        if (buf.pack_header.ack == all_connection[i].pack.pack_header.seq)
                        { // packet to client is lost
                        }
                        else if (buf.pack_header.ack == all_connection[i].pack.pack_header.seq + 1)
                        { // packet is in order
                            all_connection[i].pack.pack_header.seq++;
                        }

                        // write data to file
                        all_connection[i].myfile.write(buf.data, data_size);
                        all_connection[i].myfile.flush();
                    }
                    else
                    { // packet is out of order so drop

                        // log packet
                        print_packet("DROP", buf);

                        if (buf.pack_header.ack == all_connection[i].pack.pack_header.seq)
                        { // packet to client is lost
                        }
                        else if (buf.pack_header.ack == all_connection[i].pack.pack_header.seq + 1)
                        { //packet sent to client is in order
                            all_connection[i].pack.pack_header.seq++;
                        }
                    }
                    
                    // set ACK flag
                    all_connection[i].pack.pack_header.flags = ACK;

                    // convert network byte order
                    buf.pack_header.seq = htonl(all_connection[i].pack.pack_header.seq);
                    buf.pack_header.ack = htonl(all_connection[i].pack.pack_header.ack);
                    buf.pack_header.id = htons(all_connection[i].pack.pack_header.id);
                    buf.pack_header.flags = htons(all_connection[i].pack.pack_header.flags);

                    // send packet
                    sendto(sockfd, &buf,
                           sizeof(buf), 0,
                           &all_connection[i].src_addr,
                           all_connection[i].addrlen);

                    // log packet
                    print_packet("SEND", all_connection[i].pack);

                    break;
                }
            }
        }
        /*
        FIN RECEIVED
        */
        else if (buf.pack_header.flags == FIN)
        {
            for (int i = 0; i < MAX_CONNECTIONS; i++)
            {
                if (buf.pack_header.id == all_connection[i].pack.pack_header.id)
                { // find the corresponding connection
                    if (buf.pack_header.seq == all_connection[i].pack.pack_header.ack)
                    { // packet is in order
                        all_connection[i].pack.pack_header.ack++;
                    }
                    else
                    { // packet is out of order, drop
                    }

                    // set ACK flag
                    all_connection[i].pack.pack_header.flags = ACK;

                    // convert network byte order
                    buf.pack_header.seq = htonl(all_connection[i].pack.pack_header.seq);
                    buf.pack_header.ack = htonl(all_connection[i].pack.pack_header.ack);
                    buf.pack_header.id = htons(all_connection[i].pack.pack_header.id);
                    buf.pack_header.flags = htons(all_connection[i].pack.pack_header.flags);

                    // send packet
                    sendto(sockfd, &buf,
                           sizeof(buf), 0,
                           &all_connection[i].src_addr,
                           all_connection[i].addrlen);

                    // log packet
                    print_packet("SEND", all_connection[i].pack);

                    // set FIN flags
                    buf.pack_header.seq = all_connection[i].pack.pack_header.seq;
                    buf.pack_header.ack = 0;
                    buf.pack_header.id = all_connection[i].pack.pack_header.id;
                    buf.pack_header.flags = FIN;

                    // log packet
                    print_packet("SEND", buf);

                    // convert network byte order
                    buf.pack_header.seq = htonl(buf.pack_header.seq);
                    buf.pack_header.ack = htonl(buf.pack_header.ack);
                    buf.pack_header.id = htons(buf.pack_header.id);
                    buf.pack_header.flags = htons(buf.pack_header.flags);

                    // send packet
                    sendto(sockfd, &buf,
                           sizeof(buf), 0,
                           &all_connection[i].src_addr,
                           all_connection[i].addrlen);

                    // mark connectin as closed
                    all_connection[i].is_FIN = 1;
                    break;
                }
            }
        }
    }
}
