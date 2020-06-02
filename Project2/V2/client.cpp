#include <iostream>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
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
#include <thread>
#include <chrono>
#include <ctime>
#include <vector>
#include <unordered_set>

#include "packet.h"

using namespace std;

fd_set write_fds;

unsigned int SS_THRESH = 10000;
unsigned int CWND = 5120;
unsigned int last_seq = 0;
int timer_flag = 0;

unsigned int seq_num = 0;
unsigned int ack_num = 0;
unsigned int id_num = 0;

void throwerror(string msg)
{
    perror(msg.c_str());
    exit(1);
}

struct logger
{
    unsigned int E_ACK;
    unsigned int seq;
    chrono::steady_clock::time_point send_time;
    streampos file_pos;
};

typedef struct logger logger;

vector<logger> send_window; //vector for packet logs

unordered_set<uint32_t> already_sent;
unordered_set<uint32_t> already_acked;

// 0.5 sec timer used in SYN and ENDING phase
void timer(int ms)
{
    //timeout for 0.5 second
    //timer evaluate the flag every 1 ms
    //if change due to ack received, timer terminates
    chrono::steady_clock::time_point time_out = chrono::steady_clock::now();

    while (timer_flag != 1)
    {
        if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - time_out).count() >= ms)
        {
            timer_flag = 1;
            break;
        }
    }
}

void print_packet(string message, packet buf)
{
    // if message is type send
    if (!message.compare("SEND"))
    {
        ntoh_reorder(buf);
        if (already_sent.find(buf.pack_header.seq) != already_sent.end())
        {
            message = "RESEND";
        }
        else
        {
            if (buf.pack_header.flags != FIN)
            {
                already_sent.insert(buf.pack_header.seq);
            }
        }
    }

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
    }

    cout << message << " " << buf.pack_header.seq << " " << buf.pack_header.ack << " " << flag << endl;
}

// sigpipe signal handle, used to check network condition
void sigpipe_handler(int s)
{
    if (s == SIGPIPE)
    {
        cerr << "ERROR: Server is down. Abort." << endl;
        exit(EXIT_FAILURE);
    }
}

// data receiving in stop and wait packet transfer, handshake and ending functions, expected ack number as an argument
int recv_pack(int sockfd, packet &recv_pack, struct addrinfo *base, uint32_t ack)
{
    memset(&recv_pack, 0, packet_size);

    // start timer the
    timer_flag = 0;
    thread timer_thread(timer, 500);

    //as long as timer is not timeout
    while (!timer_flag)
    {
        int byte_s = recvfrom(sockfd, &recv_pack, packet_size, 0, base->ai_addr, &base->ai_addrlen);
        if (byte_s >= 0)
        { //check whether the receive ack is the expected
            ntoh_reorder(recv_pack);

            // log packet
            print_packet("RECV", recv_pack);

            if (ack == recv_pack.pack_header.ack || recv_pack.pack_header.flags == FIN)
            { // if found the expected ACK
                timer_flag = 1;
                timer_thread.join();
                return byte_s;
            }
        }
    }
    timer_thread.join();
    return -1;
}

// helper function to compute the time since
int64_t time_since(chrono::steady_clock::time_point start_time)
{
    chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start_time).count();
}

void send_packet(int sockfd, packet &buf, struct addrinfo *base)
{
    sendto(sockfd, &buf,
           sizeof(buf), 0, base->ai_addr, base->ai_addrlen);
}

void send_packet(int sockfd, packet &buf, int len, struct addrinfo *base)
{
    sendto(sockfd, &buf,
           len, 0, base->ai_addr, base->ai_addrlen);
}

// two way handshake
void handshake(int sockfd, struct addrinfo *base)
{
    // start the 10 second timer
    chrono::steady_clock::time_point start_time;
    start_time = chrono::steady_clock::now();

    // create send and recieve packet
    packet sending_packet;
    memset(&sending_packet, 0, sizeof(sending_packet));
    packet receiving_packet;
    memset(&receiving_packet, 0, sizeof(receiving_packet));

    // set SYN headers
    set_header(sending_packet, seq_num, ack_num, id_num, SYN);

    while (1)
    { // send SYN and wait for SYN/ACK

        if (time_since(start_time) >= 10)
        { // check 10 second timeout
            cerr << "ERROR: Server receiving timeout! Nothing received for 10 sec" << endl;
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        // send packet
        send_packet(sockfd, sending_packet, base);

        // log packet
        print_packet("SEND", sending_packet);

        // recieve incoming packet with expected ack_num=seq_num+1
        int recv_bytes = recv_pack(sockfd, receiving_packet, base, seq_num + 1);

        if (recv_bytes >= 0)
        {
            // update timer
            start_time = chrono::steady_clock::now();

            if (receiving_packet.pack_header.flags == SYN_ACK)
            { // if the received packet is SYN_ACK

                // update seq num, ack num
                seq_num = receiving_packet.pack_header.ack;
                ack_num = receiving_packet.pack_header.seq + 1;

                // use server's id
                id_num = receiving_packet.pack_header.id;
                break;
            }
        }
    }
}

int64_t check_timeout(chrono::steady_clock::time_point T)
{
    chrono::duration_cast<chrono::milliseconds>(T - send_window.front().send_time).count();
}

void prepare_log(logger &send_log, int seq_num, streampos file_pos)
{
    send_log.E_ACK = seq_num;                         //calculate the expected ack num for this packet
    send_log.file_pos = file_pos;                     //log its pos in file
    send_log.send_time = chrono::steady_clock::now(); //log the time it sent out
    send_window.push_back(send_log);                  //put this log onto vector
}
// send packet in sliding window fashion
void slidingWindow_data_trans(int sockfd, struct addrinfo *base, string file_name)
{
    unsigned int CWND_space = CWND / 512;
    unsigned int recv_ack = 0;
    unsigned last_seq = 0;
    int file_length = 0;
    int F = ACK;
    chrono::steady_clock::time_point timeout_check;
    timer_flag = 0;

    ifstream myfile(file_name.c_str(), ios::binary | ios::ate);

    packet sending_packet;
    packet receiving_packet;

    // check if file is okay
    if (myfile.good())
    {
        file_length = myfile.tellg();
        if (file_length > max_file_size)
        {
            close(sockfd);
            cerr << "ERROR: File too large to be sent. Abort" << endl;
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        cerr << "ERROR: fail to open the file" << endl;
        exit(EXIT_FAILURE);
    }

    // set pointer to start of file
    myfile.seekg(0, myfile.beg);
    timer_flag = 0;

    while (1)
    {
        // check time
        chrono::steady_clock::time_point T = chrono::steady_clock::now();

        if (!send_window.empty())
        { // check whether there is a timeout
            // 0.5 second timeout check
            if (check_timeout(T) > 500)
            {
                // reset file position to first packet
                myfile.clear(); //clear the eof bit
                myfile.seekg(send_window.front().file_pos);

                // restore sequence number
                last_seq = seq_num;
                seq_num = send_window.front().seq;

                cout << "TIMEOUT " << seq_num << endl;
                // erase first element
                send_window.erase(send_window.begin());
                CWND_space = CWND / 512;
            }
        }

        // if there is still free space in CWND
        if (CWND_space > 0 && !myfile.eof())
        {
            // read data into packet
            streampos file_pos = myfile.tellg();
            myfile.read(sending_packet.data, data_size);

            if (myfile.fail() && !myfile.eof())
            { // error check for reading
                close(sockfd);
                throwerror("ERROR: Error during reading from file");
            }

            // prepare headers in packet
            set_header(sending_packet, seq_num, ack_num, id_num, F);
            ack_num = 0;
            F = NONE; // no flags ever since

            // logging
            logger send_log;
            send_log.seq = seq_num;

            // increment seq number by the sent bytes
            seq_num += myfile.gcount();
            seq_num %= (MAX_SEQ_NUMBER + 1); // max seq num is 25600

            prepare_log(send_log, seq_num, file_pos);

            if (timer_flag == 0)
            {
                timeout_check = send_log.send_time; //mark the time for 10 second timeout
                timer_flag = 1;
            }

            // log packet
            if (ntohl(last_seq) >= ntohl(seq_num))
            {
                // dup packet
                print_packet("SEND", sending_packet);
                last_seq = seq_num;
            }
            else
            {
                print_packet("SEND", sending_packet);
            }

            // send packet
            send_packet(sockfd, sending_packet, 12 + myfile.gcount(), base);

            // decrement CWND space
            CWND_space--;
        }

        // listen for incoming ACK packet
        int byte_s;
        if ((byte_s = recvfrom(sockfd, &receiving_packet, packet_size, 0, base->ai_addr, &base->ai_addrlen)) < 0)
        {
            if (time_since(timeout_check) >= 10)
            {
                close(sockfd);
                throwerror("ERROR: nothing receive from server for 10 second");
            }
        }

        // convert to host byte order
        ntoh_reorder(receiving_packet);

        // check whether the receive ack is the expected
        recv_ack = receiving_packet.pack_header.ack;

        // update the timer
        timeout_check = chrono::steady_clock::now();

        // traverse the queue
        vector<int>::size_type i = 0;
        while (i != send_window.size())
        {
            //if find this packet in queue, increment pop them from queue, increment window size
            if (send_window[i].E_ACK == receiving_packet.pack_header.ack)
            {
                // log packet
                print_packet("RECV", receiving_packet);

                // erase all previous packets
                send_window.erase(send_window.begin(), send_window.begin() + i + 1);
                CWND_space += 2;
                break;
            }
            i++;
        }

        // if reaches to the end of file and all ACKs are received
        if (myfile.tellg() == -1 && send_window.empty())
        {
            break;
        }
    }
}

// client initiates shutdown
void shutdown(int sockfd, struct addrinfo *base)
{
    int client_fin = 0;
    ack_num = 0;

    chrono::steady_clock::time_point start_time;
    chrono::steady_clock::time_point send_time;

    // create data and packet
    packet sending_packet;
    memset(&sending_packet, 0, sizeof(sending_packet));
    packet receiving_packet;
    memset(&receiving_packet, 0, sizeof(receiving_packet));

    // send FIN message
    set_header(sending_packet, seq_num, ack_num, id_num, FIN);

    // log packet
    print_packet("SEND", sending_packet);

    // send packet
    send_packet(sockfd, sending_packet, base);
    start_time = chrono::steady_clock::now();
    send_time = chrono::steady_clock::now();

    // wait for FIN/ACK
    while (1)
    {

        if (time_since(start_time) >= 10)
        { // check 10 second timeout
            cerr << "ERROR: Server FINACK receiving timeout! Nothing received for 10 sec" << endl;
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        if (time_since(send_time) >= 0.5)
        { // if send 0.5 second timeout
            // send packet
            send_packet(sockfd, sending_packet, base);
            send_time = chrono::steady_clock::now();

            // log packet
            print_packet("SEND", sending_packet);
        }

        int recv_bytes;
        if ((recv_bytes = recvfrom(sockfd, &receiving_packet, packet_size, 0, base->ai_addr, &base->ai_addrlen)) <= 0)
        {
            continue;
        }

        // convert network to host
        ntoh_reorder(receiving_packet);

        // log packet
        print_packet("RECV", receiving_packet);

        // update timer
        start_time = chrono::steady_clock::now();

        if ((receiving_packet.pack_header.ack == seq_num + 1 || receiving_packet.pack_header.flags == FIN))
        { // server sent ACK for FIN or FIN
            if (receiving_packet.pack_header.flags == FIN)
            { // server sent a FIN
                client_fin = 1;
                ack_num = receiving_packet.pack_header.seq + 1;
                set_header(sending_packet, seq_num, ack_num, id_num, ACK);

                // send packet
                send_packet(sockfd, sending_packet, base);
                start_time = chrono::steady_clock::now();

                // log packet
                print_packet("SEND", sending_packet);
            }

            while (client_fin)
            { // enter the two second wait phase if received FIN/ACK
                if (time_since(start_time) >= 2)
                { // if timeout close socket
                    close(sockfd);
                    return;
                }

                if (recvfrom(sockfd, &receiving_packet, packet_size, 0, base->ai_addr, &base->ai_addrlen) > 0)
                { // waiting for server FIN
                    // log packet
                    print_packet("RECV", receiving_packet);

                    // convert network to host
                    ntoh_reorder(receiving_packet);

                    if (receiving_packet.pack_header.flags == FIN)
                    { // server sends FIN
                        ack_num = receiving_packet.pack_header.seq + 1;
                        set_header(sending_packet, seq_num, ack_num, id_num, ACK);

                        // send packet
                        send_packet(sockfd, sending_packet, base);

                        // log packet
                        print_packet("SEND", sending_packet);
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    signal(SIGPIPE, sigpipe_handler);

    //check client argument number
    if (argc != 4)
    {
        perror("ERROR: incorrect number of arguments!");
        exit(EXIT_FAILURE);
    }

    string host_name_ip = argv[1];
    string port_num = argv[2];
    string file_name = argv[3];

    //check port number
    if (stoi(port_num) <= 1023 && stoi(port_num) >= 0)
    {
        perror("ERROR: incorrect port num, please specify a number in the range [1024, 65535]");
        exit(EXIT_FAILURE);
    }

    struct addrinfo hints, *server_info, *base;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(host_name_ip.c_str(), port_num.c_str(), &hints, &server_info) != 0)
    {
        cerr << "ERROR: getaddrinfo error" << endl;
        exit(EXIT_FAILURE);
    }

    int sockfd;
    base = server_info;

    if ((sockfd = socket(base->ai_family, base->ai_socktype, base->ai_protocol)) < 0)
    {
        throwerror("ERROR: unable to make socket");
    }

    // set socketfd non-blocking
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    // TODO: fix sequence number wraparound bug, current solution is hacky
    // set random sequence number
    seq_num = rand() % (MAX_SEQ_NUMBER / 4);

    // start TCP handshake
    handshake(sockfd, base);

    // send file with pipeline
    slidingWindow_data_trans(sockfd, base, file_name);

    // close connection
    shutdown(sockfd, base);
}
