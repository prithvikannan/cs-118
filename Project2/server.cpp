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
#include <unordered_set>

#include "packet.h"

using namespace std;

fd_set write_fds;
fd_set read_fds;
struct timeval tv;
struct timeval ts_tv;

unordered_set<uint32_t> already_acked;
unordered_set<uint32_t> already_sent;

void throwerror(string msg)
{
    perror(msg.c_str());
    exit(1);
}

static void sig_handler(int signum)
{
    exit(0);
}

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

    if (!message.compare("SEND"))
    {
        if (already_sent.find(buf.pack_header.ack) != already_sent.end())
        {
            message = "RESEND";
        }
        else
        {
            if (buf.pack_header.flags != ACK)
            {
                already_sent.insert(buf.pack_header.ack);
            }
        }
    }

    string flag = "";
    switch (buf.pack_header.flags)
    {
    case 1:
        flag = "FIN";
        break;
    case 2:
        flag = "SYN";
        break;
    case 4:
        flag = "ACK";
        if (already_acked.find(buf.pack_header.ack) != already_acked.end())
        {
            flag = "DUP-ACK";
        }
        else
        {
            already_acked.insert(buf.pack_header.ack);
        }
        break;
    case 5:
        flag = "FIN ACK";
        break;
    case 6:
        flag = "SYN ACK";
        break;
    default:
        break;
    }

    cout << message << " " << buf.pack_header.seq << " " << buf.pack_header.ack << " " << flag << endl;
}

int create_socket(string port_num)
{
    int sockfd;
    int temp = 1;

    if (stoi(port_num) <= 1023 && stoi(port_num) >= 0)
    {
        throwerror("ERROR: bad port number");
    }

    struct addrinfo hints, *server_info, *base;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port_num.c_str(), &hints, &server_info) < 0)
    {
        throwerror("ERROR: can't get address info");
    }

    base = server_info;

    if ((sockfd = socket(base->ai_family, base->ai_socktype, base->ai_protocol)) < 0)
    {
        perror("ERROR: unable to make socket");
    }

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(int));
    if (bind(sockfd, base->ai_addr, base->ai_addrlen) < 0)
    {
        perror("ERROR: unable to bind socket");
        close(sockfd);
    }

    freeaddrinfo(server_info);

    return sockfd;
}

void setup_connection(int i, uint32_t buf_seq, struct sockaddr client_addr, socklen_t client_addrlen)
{
    all_connection[i].pack.pack_header.seq = rand() % MAX_SEQ_NUMBER;
    all_connection[i].pack.pack_header.ack = buf_seq + 1;
    all_connection[i].pack.pack_header.flags = SYN_ACK;
    all_connection[i].pack.pack_header.id = i + 1;
    all_connection[i].src_addr = client_addr;
    all_connection[i].addrlen = client_addrlen;
}

void send_packet(int sockfd, packet &buf, int i)
{
    sendto(sockfd, &buf,
           sizeof(buf), 0,
           &all_connection[i].src_addr,
           all_connection[i].addrlen);
}

// helper function when SYN is sent
void handle_handshake(packet &buf, int sockfd, struct sockaddr client_addr, socklen_t client_addrlen)
{
    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        if (all_connection[i].src_addr.sa_family != client_addr.sa_family)
        {
            if (!all_connection[i].pack.pack_header.id)
            {
                // setup file path
                string file_path = to_string(i + 1) + ".file";
                all_connection[i].myfile.open(file_path);

                // set rand seq number, increment ack number, set flags
                uint32_t buf_seq = buf.pack_header.seq;
                setup_connection(i, buf_seq, client_addr, client_addrlen);

                // prepare packet
                buf.pack_header = all_connection[i].pack.pack_header;

                // convert to network byte order
                hton_reorder(buf);

                // send packet
                send_packet(sockfd, buf, i);

                // log packet
                print_packet("SEND", all_connection[i].pack);
                break;
            }
        }
    }
}

bool compare_id(packet &buf, packet &connection)
{
    return buf.pack_header.id == connection.pack_header.id;
}

bool compare_seq_to_ack(packet &buf, packet &connection)
{
    return buf.pack_header.seq == connection.pack_header.ack;
}

bool compare_ack_to_seq(packet &buf, packet &connection)
{
    return buf.pack_header.ack == connection.pack_header.seq + 1;
}

void write_to_file(int i, packet &buf, int recv_data)
{
    all_connection[i].myfile.write(buf.data, recv_data);
    all_connection[i].myfile.flush();
}

// helper function when ACK is sent
void handle_ack(packet &buf, int sockfd, int recv_data)
{
    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        if (compare_id(buf, all_connection[i].pack))
        { // find the connection
            if (all_connection[i].is_FIN)
            { // connection has already been marked as FIN
                all_connection[i].myfile.close();
                break;
            }
            if (compare_seq_to_ack(buf, all_connection[i].pack))
            { // packet is in the correct order
                all_connection[i].pack.pack_header.ack += recv_data;
                all_connection[i].pack.pack_header.ack %= MAX_SEQ_NUMBER;

                if (compare_ack_to_seq(buf, all_connection[i].pack))
                { // packet is in order
                    all_connection[i].pack.pack_header.seq++;
                }
                // write data to file
                write_to_file(i, buf, recv_data);
            }
            else if (compare_ack_to_seq(buf, all_connection[i].pack))
            { //packet sent to client is in order
                all_connection[i].pack.pack_header.seq++;
            }

            // set ACK flag
            all_connection[i].pack.pack_header.flags = ACK;

            // prepare packet
            buf.pack_header = all_connection[i].pack.pack_header;

            // convert network byte order
            hton_reorder(buf);

            // send packet
            send_packet(sockfd, buf, i);

            // log packet
            print_packet("SEND", all_connection[i].pack);

            break;
        }
    }
}

// helper function when FIN is sent
void handle_fin(packet &buf, int sockfd)
{
    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        if (compare_id(buf, all_connection[i].pack))
        { // find the corresponding connection
            if (compare_seq_to_ack(buf, all_connection[i].pack))
            { // packet is in order
                all_connection[i].pack.pack_header.ack++;
            }

            // set ACK flag
            all_connection[i].pack.pack_header.flags = ACK;

            // prepare packet
            buf.pack_header = all_connection[i].pack.pack_header;

            // convert network byte order
            hton_reorder(buf);

            // send packet
            send_packet(sockfd, buf, i);

            // log packet
            print_packet("SEND", all_connection[i].pack);

            // set FIN flags
            buf.pack_header = all_connection[i].pack.pack_header;
            buf.pack_header.flags = FIN;

            // log packet
            print_packet("SEND", buf);

            // convert network byte order
            hton_reorder(buf);

            // send packet
            send_packet(sockfd, buf, i);

            // mark connectin as closed
            all_connection[i].is_FIN = 1;
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    signal(SIGQUIT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGINT, sig_handler);

    string port_num, file_dir;
    struct sockaddr client_addr;
    socklen_t client_addrlen = 16;
    packet buf;

    if (argc != 2)
    {
        perror("ERROR: incorrect number of arguments!");
        exit(EXIT_FAILURE);
    }

    port_num = argv[1];

    // create socket
    int sockfd = create_socket(port_num);

    while (1)
    {
        memset(&buf, 0, sizeof(buf));
        int recv_byte = recvfrom(sockfd, &buf, sizeof(buf), 0, &client_addr, &client_addrlen);
        int recv_data = recv_byte - 12;

        // convert network to host byte order
        ntoh_reorder(buf);

        // log packet
        print_packet("RECV", buf);

        if (buf.pack_header.flags == SYN)
        { // SYN received
            handle_handshake(buf, sockfd, client_addr, client_addrlen);
        }
        else if (buf.pack_header.flags == ACK || buf.pack_header.flags == 0)
        { // ACK received
            handle_ack(buf, sockfd, recv_data);
        }
        else if (buf.pack_header.flags == FIN)
        { // FIN received
            handle_fin(buf, sockfd);
        }
    }
}
