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

using namespace std;

#define max_file_size 100 * 1024 * 1024
#define packet_size 524
#define data_size 512
#define MSS 512
#define MAX_retran 20

#define FIN 1
#define SYN 2
#define ACK 4
#define ACK_FIN 5
#define SYN_ACK 6

fd_set write_fds;

unsigned int SS_THRESH = 10000;
unsigned int CWND = 512;
unsigned int last_seq = 0;
int max_seq_num = 102400;
int timer_flag = 0;

unsigned int seq_num = 0;
unsigned int ack_num = 0;
unsigned int id_num = 0;

struct logger
{
    unsigned int E_ACK;
    unsigned int seq;
    chrono::steady_clock::time_point send_time;
    streampos file_pos;
};

typedef struct logger logger;

vector<logger> CWND_logs; //vector for packet logs

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
    char data[data_size];
};

typedef struct packet packet;

void set_header(packet &pack, uint32_t S, uint32_t A, uint16_t I, uint16_t F)
{
    pack.pack_header.seq = htonl(S);
    pack.pack_header.ack = htonl(A);
    pack.pack_header.id = htons(I);
    pack.pack_header.flags = htons(F);
}

// network to host byte order transfer
void ntoh_reorder(packet &pack)
{
    pack.pack_header.ack = ntohl(pack.pack_header.ack);
    pack.pack_header.seq = ntohl(pack.pack_header.seq);
    pack.pack_header.id = ntohs(pack.pack_header.id);
    pack.pack_header.flags = ntohs(pack.pack_header.flags);
}

//multithread 0.5 sec timer used in SYN and ENDING phase
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

//output message format
void standard_output(char format, uint32_t S, uint32_t A, uint16_t ID, int F, unsigned int window, unsigned int thresh)
{
    string FLAG = "";

    if (format == 'S' || format == 'U')
    {
        S = ntohl(S);
        A = ntohl(A);
        ID = ntohs(ID);
        F = ntohs(F);
    }

    switch (F)
    {
    case 0:
        FLAG = " ";
        break;
    case 1:
        FLAG = "FIN";
        break;
    case 2:
        FLAG = "SYN";
        break;
    case 4:
        FLAG = "ACK";
        break;
    case 5:
        FLAG = "FIN ACK";
        break;
    case 6:
        FLAG = "SYN ACK";
        break;
    }

    if (format == 'R')
        printf("RECV %u %u %u %u %u %s\n", S, A, ID, window, thresh, FLAG.c_str());
    else if (format == 'D')
        printf("DROP %u %u %u %s\n", S, A, ID, FLAG.c_str());
    else if (format == 'S')
        printf("SEND %u %u %u %u %u %s\n", S, A, ID, window, thresh, FLAG.c_str());
    else if (format == 'U')
        printf("SEND %u %u %u %u %u %s DUP\n", S, A, ID, window, thresh, FLAG.c_str());
}

void print_packet(string message, packet buf)
{
    ntoh_reorder(buf);
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

//sigpipe signal handle, used to check network condition
void sigpipe_handler(int s)
{
    if (s == SIGPIPE)
    {
        cerr << "ERROR: Server is down. Abort." << endl;
        exit(EXIT_FAILURE);
    }
}

//data receiving in stop and wait packet transfer, handshake and ending functions, expected ack number as an argument
int recv_pack(int sockfd, packet &recv_pack, struct addrinfo *anchor, uint32_t ack)
{
    memset(&recv_pack, 0, packet_size);

    //invoke the timer
    timer_flag = 0;
    thread timer_thread(timer, 500);

    //as long as timer is not timeout
    while (timer_flag == 0)
    {
        int byte_s = recvfrom(sockfd, &recv_pack, packet_size, 0, anchor->ai_addr, &anchor->ai_addrlen);
        if (byte_s >= 0)
        { //check whether the receive ack is the expected
            ntoh_reorder(recv_pack);

            // outout receive infor
            standard_output('R', recv_pack.pack_header.seq, recv_pack.pack_header.ack, recv_pack.pack_header.id, recv_pack.pack_header.flags, CWND, SS_THRESH);
            if (ack == recv_pack.pack_header.ack || recv_pack.pack_header.flags == FIN) //if found the expected ACK
            {
                // printf("Received the expected ACk packet!\n");
                timer_flag = 1;
                timer_thread.join();

                if (CWND < SS_THRESH)
                    CWND += 512;

                return byte_s;
            }
            else //if ack is received but not the expected one
            {
                continue;
            }
        }
    }
    timer_thread.join();
    return -1; //indicating no expected ACK returns
}

// helper function to compute the time since
int64_t time_since(chrono::steady_clock::time_point start_time)
{
    chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start_time).count();
}

// two way handshake
void handshake(int sockfd, struct addrinfo *anchor)
{
    chrono::steady_clock::time_point start_time;

    // create send and recieve packet
    packet send_packet;
    packet recv_packet;
    memset(&send_packet, 0, sizeof(send_packet));
    memset(&recv_packet, 0, sizeof(recv_packet));

    // set SYN headers
    set_header(send_packet, seq_num, ack_num, id_num, SYN);

    // start the 10 second timer
    start_time = chrono::steady_clock::now();

    while (1)
    { // send SYN and wait for SYN/ACK

        if (time_since(start_time) >= 10)
        { // check 10 second timeout
            cerr << "ERROR: Server receiving timeout! Nothing received for 10 sec" << endl;
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        // send packet
        sendto(sockfd, &send_packet, packet_size, 0, anchor->ai_addr, anchor->ai_addrlen);

        // log packet
        print_packet("SEND", send_packet);

        // recieve incoming packet with expected ack_num=seq_num+1
        int recv_bytes = recv_pack(sockfd, recv_packet, anchor, seq_num + 1);

        if (recv_bytes >= 0)
        {
            // update timer
            start_time = chrono::steady_clock::now();

            if (recv_packet.pack_header.flags == SYN_ACK)
            { // if the received packet is SYN_ACK

                // update seq num, ack num
                seq_num = recv_packet.pack_header.ack;
                ack_num = recv_packet.pack_header.seq + 1;

                // use server's id
                id_num = recv_packet.pack_header.id;
                break;
            }
        }
    }
}

// send packet in sliding window fashion
void slidingWindow_data_trans(int sockfd, struct addrinfo *anchor, string file_name)
{
    unsigned int CWND_space = CWND / 512;
    unsigned int recv_ack = 0;
    unsigned last_seq = 0;
    int file_length = 0;
    int F = 4; //need a ACK for the first send
    chrono::steady_clock::time_point timeout_check;
    timer_flag = 0; //reset timer flag

    ifstream myfile(file_name.c_str(), ios::binary | ios::ate);

    packet send_packet;
    packet recv_packet;

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

    myfile.seekg(0, myfile.beg);
    timer_flag = 0; //flag for timeout

    while (1)
    {
        // check time
        chrono::steady_clock::time_point T = chrono::steady_clock::now();

        // check whether there is a timeout
        if (!CWND_logs.empty())
        {
            // 0.5 ceond timeout check
            if ((chrono::duration_cast<chrono::milliseconds>(T - CWND_logs.front().send_time).count() > 500))
            {
                // resume original pos in file and restore seq number
                //printf("time out detected!\n");
                myfile.clear(); //clear the eof bit
                myfile.seekg(CWND_logs.front().file_pos);

                last_seq = seq_num;
                seq_num = CWND_logs.front().seq;
                CWND_logs.erase(CWND_logs.begin());
                //CWND_logs.clear(); //remove all elements in CWND_logs vector

                // reset CWND window
                SS_THRESH = CWND / 2;
                CWND = 512;
                CWND_space = CWND / 512; //clear CWND space for resend
            }
        }

        // if there is still free space in CWND
        if (CWND_space > 0 && !myfile.eof())
        {
            streampos file_pos = myfile.tellg(); //get cureent packet pos in file before read
            myfile.read(send_packet.data, data_size);

            if (myfile.fail() && !myfile.eof())
            {
                cerr << "ERROR: Error during reading from file" << endl;
                close(sockfd);
                exit(EXIT_FAILURE);
            }

            // set right seq,ack,id and flags for packet to be sent (ACK number is 0, no flags)
            set_header(send_packet, seq_num, ack_num, id_num, F);
            ack_num = 0;
            F = 0; // no flags ever since

            logger send_log;        //create a log for this packet
            send_log.seq = seq_num; //logs its sequence num

            //increment seq number by the sent bytes
            seq_num += myfile.gcount();
            seq_num %= (102400 + 1); // max seq num is 102400

            send_log.E_ACK = seq_num;                         //calculate the expected ack num for this packet
            send_log.file_pos = file_pos;                     //log its pos in file
            send_log.send_time = chrono::steady_clock::now(); //log the time it sent out
            CWND_logs.push_back(send_log);                    //put this log onto vector

            if (timer_flag == 0)
            {
                timeout_check = send_log.send_time; //mark the time for 10 second timeout
                timer_flag = 1;
            }

            // outout send infor
            if (ntohl(last_seq) >= ntohl(seq_num))
            {
                standard_output('U', send_packet.pack_header.seq, send_packet.pack_header.ack, send_packet.pack_header.id, send_packet.pack_header.flags, CWND, SS_THRESH);
                last_seq = seq_num;
            }
            else
                standard_output('S', send_packet.pack_header.seq, send_packet.pack_header.ack, send_packet.pack_header.id, send_packet.pack_header.flags, CWND, SS_THRESH);

            // execute packet sending
            sendto(sockfd, &send_packet, 12 + myfile.gcount(), 0, anchor->ai_addr, anchor->ai_addrlen);

            // decrement CWND space
            CWND_space--;
        }

        // listen for incoming ACK packet
        int byte_s = recvfrom(sockfd, &recv_packet, packet_size, 0, anchor->ai_addr, &anchor->ai_addrlen);
        if (byte_s > 0)
        { //check whether the receive ack is the expected
            ntoh_reorder(recv_packet);
            recv_ack = recv_packet.pack_header.ack;

            // update the timer
            timeout_check = chrono::steady_clock::now();

            // tranverse the queue
            for (vector<int>::size_type i = 0; i != CWND_logs.size(); i++)
            {
                //if find this packet in queue, increment pop them from queue, increment window size
                if (CWND_logs[i].E_ACK == recv_packet.pack_header.ack)
                {
                    // outout received packet infor
                    standard_output('R', recv_packet.pack_header.seq, recv_packet.pack_header.ack, recv_packet.pack_header.id, recv_packet.pack_header.flags, CWND, SS_THRESH);
                    CWND_logs.erase(CWND_logs.begin(), CWND_logs.begin() + i + 1); //erase all slots before and including it

                    //update CWND window and SS_THRESH
                    if (CWND < SS_THRESH)
                    {
                        CWND += 512;
                        CWND_space += 2;
                    }
                    else
                    {
                        unsigned int dummy = CWND;
                        CWND += 512 * 512 / CWND;
                        if ((CWND / 512 - dummy / 512) >= 1)
                            CWND_space += 2;
                        else
                            CWND_space += 1;
                    }
                    break;
                }
                if (i == CWND_logs.size() - 1)
                    standard_output('D', recv_packet.pack_header.seq, recv_packet.pack_header.ack, recv_packet.pack_header.id, recv_packet.pack_header.flags, CWND, SS_THRESH);
            }
        }
        else
        { // if no bytes are received
            // check 10 second timeout
            if (chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - timeout_check).count() >= 10)
            {
                cerr << "ERROR: nothing receive from server for 10 second" << endl;
                close(sockfd);
                exit(EXIT_FAILURE);
            }
        }

        // if reaches to the end of file and all ACKs are received
        if (myfile.tellg() == -1 && CWND_logs.empty())
        {
            break;
        }
    }
}

// client initiates shutdown
void ending(int sockfd, struct addrinfo *anchor)
{
    chrono::steady_clock::time_point start_time;
    chrono::steady_clock::time_point send_time;
    int client_fin = 0;

    // create data and packet
    packet send_packet;
    packet recv_packet;

    // combine header and data into packet
    memset(&send_packet, 0, sizeof(send_packet));
    memset(&recv_packet, 0, sizeof(recv_packet));

    // send FIN message
    ack_num = 0;
    set_header(send_packet, seq_num, ack_num, id_num, FIN);

    // send packet
    sendto(sockfd, &send_packet, packet_size, 0, anchor->ai_addr, anchor->ai_addrlen);
    start_time = chrono::steady_clock::now();
    send_time = chrono::steady_clock::now();

    // log packet
    print_packet("SEND", send_packet);

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
            sendto(sockfd, &send_packet, packet_size, 0, anchor->ai_addr, anchor->ai_addrlen);
            send_time = chrono::steady_clock::now();

            // log packet
            print_packet("SEND", send_packet);
        }

        int recv_bytes = recvfrom(sockfd, &recv_packet, packet_size, 0, anchor->ai_addr, &anchor->ai_addrlen);
        if (recv_bytes > 0)
        {
            // log packet
            print_packet("RECV", recv_packet);

            // convert network to host
            ntoh_reorder(recv_packet);

            // update timer
            start_time = chrono::steady_clock::now();

            if ((recv_packet.pack_header.ack == seq_num + 1 || recv_packet.pack_header.flags == FIN))
            { // server sent ACK for FIN or FIN
                if (recv_packet.pack_header.flags == ACK_FIN || recv_packet.pack_header.flags == FIN)
                { // server sent a FIN
                    client_fin = 1;
                    ack_num = recv_packet.pack_header.seq + 1;
                    set_header(send_packet, seq_num, ack_num, id_num, ACK);

                    // send packet
                    sendto(sockfd, &send_packet, packet_size, 0, anchor->ai_addr, anchor->ai_addrlen); 
                    start_time = chrono::steady_clock::now();                                          

                    // log packet
                    print_packet("SEND", send_packet);
                }

                while (client_fin)
                { // enter the two second wait phase if received FIN/ACK
                    if (time_since(start_time) >= 2)
                    { // if timeout close socket
                        close(sockfd);
                        return;
                    }

                    if (recvfrom(sockfd, &recv_packet, packet_size, 0, anchor->ai_addr, &anchor->ai_addrlen) > 0)
                    { // waiting for server FIN
                        // log packet
                        print_packet("RECV", recv_packet);

                        // convert network to host
                        ntoh_reorder(recv_packet);

                        if (recv_packet.pack_header.flags == FIN)
                        { // server sends FIN
                            ack_num = recv_packet.pack_header.seq + 1;
                            set_header(send_packet, seq_num, ack_num, id_num, ACK);

                            // send packet
                            sendto(sockfd, &send_packet, packet_size, 0, anchor->ai_addr, anchor->ai_addrlen);

                            // log packet
                            print_packet("SEND", send_packet);
                        }
                        else
                        { //received some other packet
                            standard_output('D', recv_packet.pack_header.seq, recv_packet.pack_header.ack, recv_packet.pack_header.id, recv_packet.pack_header.flags, CWND, SS_THRESH);
                        }
                    }
                }
            }
            else //if not ecpected packet
                standard_output('D', recv_packet.pack_header.seq, recv_packet.pack_header.ack, recv_packet.pack_header.id, recv_packet.pack_header.flags, CWND, SS_THRESH);
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

    struct addrinfo hints, *server_info, *anchor;
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
    for (anchor = server_info; anchor != NULL; anchor = anchor->ai_next)
    {
        sockfd = socket(anchor->ai_family, anchor->ai_socktype, anchor->ai_protocol);
        if (sockfd < 0)
            continue;

        break;
    }

    if (anchor == NULL)
    {
        cerr << "ERROR: bind failure" << endl;
        exit(EXIT_FAILURE);
    }

    //set socketfd non-blocking
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    // set random sequence number
    seq_num = rand() % 25600;

    // start TCP handshake
    handshake(sockfd, anchor);

    // send file with pipeline
    slidingWindow_data_trans(sockfd, anchor, file_name);

    // close connection
    ending(sockfd, anchor);
}
