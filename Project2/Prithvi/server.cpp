// SERVER

#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <locale>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <cstdlib>
#include <vector>
#include "globals.h"
#include "tcp_packet.h"

using namespace std;

int port = 5000;
long long serverSeqNum = 0;
long long clientSeqNum = 0;

TCP_Packet initWindow[2];


vector<TCP_Packet> packetWindow;

/**
     * This method throws the perror and exits the program
     * @param s           A string that is the error message
     **/
void throwError(string s)
{
  perror(s.c_str());
  exit(1);
}

/**
 * Sends the SYN + ACK back to the client
 * @param sockfd      The socket for the connection
 * @param their_addr  The sockaddr_in struct
 * @return string     String representing the file name that was parsed
 **/
string initiateConnection(int sockfd, struct sockaddr_in &their_addr)
{
  int recvlen;
  uint8_t buf[MSS + 1];
  socklen_t sin_size = sizeof(struct sockaddr_in);
  TCP_Packet p;
  TCP_Packet sendB;
  uint8_t sendBuf[MSS];
  string fileName = "testfile";
  srand(time(NULL));
  // int count = 0;
  while (1)
  {
    recvlen = recvfrom(sockfd, buf, MSS, 0 | MSG_DONTWAIT,
                       (struct sockaddr *)&their_addr, &sin_size);
    if (recvlen > 0)
    {
      buf[recvlen] = 0;
      p.convertBufferToPacket(buf);
      cout << p.RECV() << endl;

      if (p.getSyn()) 
      { // incoming SYN from client
        if (!initWindow[0].isSent()) 
        { // has not already sent SYN ACK
          clientSeqNum = p.getSeqNumber();
          sendB.setFlags(1, 1, 0);
          serverSeqNum = rand() % 10000;
          sendB.setSeqNumber(serverSeqNum % MAXSEQ);
          sendB.setAckNumber(clientSeqNum + 1);
          sendB.convertPacketToBuffer(sendBuf);
          cout << sendB.SEND() << endl;
          if (sendto(sockfd, &sendBuf, MSS, 0, (struct sockaddr *)&their_addr,
                     sizeof(their_addr)) < 0)
          {
            throwError("Could not send to the client");
          }
          sendB.startTimer();
          sendB.setSent();
          initWindow[0] = sendB;
        }
        else 
        { // we already sent SYN ACK
          initWindow[0].convertPacketToBuffer(sendBuf);
          cout << "SEND " << serverSeqNum << " " << WINDOW
               << " Retransmission "
               << " SYN" << endl;
          if (sendto(sockfd, &sendBuf, MSS, 0, (struct sockaddr *)&their_addr,
                     sizeof(their_addr)) < 0)
          {
            throwError("Could not send to the client");
          }
          initWindow[0].startTimer();
        }
      }
      else if (p.getAck() && initWindow[0].isSent() &&
               p.getAckNumber() == (initWindow[0].getSeqNumber()+1))
      { // incoming ACK from client
        initWindow[0].setAcked();
        clientSeqNum = p.getSeqNumber();
        // serverSeqNum += 1;
        // sendB.setFlags(1, 0, 0);
        // sendB.setSeqNumber(serverSeqNum % MAXSEQ);
        // sendB.setAckNumber(clientSeqNum);
        // sendB.convertPacketToBuffer(sendBuf);
        // cout << sendB.SEND() << endl;
        // if (sendto(sockfd, &sendBuf, MSS, 0, (struct sockaddr *)&their_addr,
        //            sizeof(their_addr)) < 0)
        // {
        //   throwError("Could not send to the client");
        // }
        // sendB.setSent();
        // sendB.startTimer();
        // initWindow[1] = sendB;
        return fileName;
      }
      // poll iniwitWindow[0] (not 1, since its just an ack and has no timer)
      if (initWindow[0].isSent() && !initWindow[0].isAcked() &&
          initWindow[0].hasTimedOut(1))
      {
        initWindow[0].convertPacketToBuffer(sendBuf);
        cout << initWindow[0].SEND() << endl;

        if (sendto(sockfd, &sendBuf, MSS, 0, (struct sockaddr *)&their_addr,
                   sizeof(their_addr)) < 0)
        {
          throwError("Could not send to the client");
        }
        initWindow[0].startTimer();
      }
      memset((char *)&buf, 0, MSS + 1);
    }
  }
}

/**
 * Send chunked file to the client
 * @param sockfd      The socket for the connection
 * @param their_addr  The sockaddr_in struct
 * @param fileSize    The size of the file
 * @param fs          The streampos (at the end of the file)
 * @param fileBuffer  The buffer holding the contents of the file
 **/
void sendChunkedFile(int sockfd, struct sockaddr_in &their_addr,
                     long long fileSize, streampos fs, char *fileBuffer)
{
  long numPackets = 0;
  uint8_t sendBuf[MSS];
  uint8_t buf[MSS + 1];
  int recvlen;
  TCP_Packet p;
  TCP_Packet ack;
  numPackets = fs / PACKET_SIZE + 1;
  socklen_t sin_size = sizeof(struct sockaddr_in);
  serverSeqNum++;
  long i = 0;
  while (1)
  {
    if (i < numPackets && packetWindow.size() < WINDOW / MSS)
    {
      p.setFlags(0, 0, 0);
      if (i == numPackets - 1)
      {
        p.setData((uint8_t *)(fileBuffer + i * PACKET_SIZE),
                  (int)(fileSize - PACKET_SIZE * i));
      }
      else
      {
        p.setData((uint8_t *)(fileBuffer + i * PACKET_SIZE), PACKET_SIZE);
      }
      p.setSeqNumber(serverSeqNum % MAXSEQ);
      p.setAckNumber(clientSeqNum + 1);
      p.convertPacketToBuffer(sendBuf);
      packetWindow.push_back(p);
      cout << p.SEND() << endl;
      if (sendto(sockfd, &sendBuf, MSS, 0, (struct sockaddr *)&their_addr,
                 sizeof(their_addr)) < 0)
        throwError("Could not send to the client");
      serverSeqNum += MSS;
      packetWindow.back().startTimer();
      i++;
    }
    recvlen = recvfrom(sockfd, buf, MSS, 0 | MSG_DONTWAIT,
                       (struct sockaddr *)&their_addr, &sin_size);
    if (recvlen > 0)
    {
      buf[recvlen] = 0;
      ack.convertBufferToPacket(buf);
      cout << ack.RECV() << endl;
      if (ack.getSeqNumber() == initWindow[1].getAckNumber())
      {
        initWindow[1].convertPacketToBuffer(sendBuf);
        cout << "SEND " << initWindow[1].getSeqNumber() << " "
             << initWindow[1].getAckNumber() << " Retransmission" << endl;
        if (sendto(sockfd, &sendBuf, MSS, 0, (struct sockaddr *)&their_addr,
                   sizeof(their_addr)) < 0)
          throwError("Could not send to the client");
      }
      else
      {
        clientSeqNum = ack.getSeqNumber();
        for (unsigned long j = 0; j < packetWindow.size(); j++)
          if ((packetWindow[j].getSeqNumber()+packetWindow[j].getLen()) == ack.getAckNumber())
            packetWindow[j].setAcked();
        while (1)
        {
          if (packetWindow.size() > 0 && packetWindow[0].isAcked())
          {
            if (packetWindow.size() > 1)
            {
              for (unsigned long k = 0; k < packetWindow.size() - 1; k++)
              {
                packetWindow[k] = packetWindow[k + 1];
              }
            }
            packetWindow.pop_back();
            if (packetWindow.size() == 0 && i >= numPackets)
              return;
          }
          else
            break;
        }
      }
    }
    for (unsigned long j = 0; j < packetWindow.size(); j++)
      if (packetWindow[j].hasTimedOut(1))
      {
        packetWindow[j].convertPacketToBuffer(sendBuf);
        cout << "SEND " << packetWindow[j].getSeqNumber() << " "
             << packetWindow[j].getAckNumber() << " Retransmission" << endl;
        if (sendto(sockfd, &sendBuf, MSS, 0, (struct sockaddr *)&their_addr,
                   sizeof(their_addr)) < 0)
          throwError("Could not send to the client");
        packetWindow[j].startTimer();
      }
  }
}

/**
 * Sends the FIN to the client
 * @param sockfd      The socket for the connection
 * @param their_addr  The sockaddr_in struct
 **/
void closeConnection(int sockfd, struct sockaddr_in &their_addr)
{
  int recvlen = 0;
  int retryFin = 0;
  uint8_t buf[MSS + 1];
  socklen_t sin_size = sizeof(struct sockaddr_in);
  TCP_Packet ackPacket;
  TCP_Packet receivedPacket;
  TCP_Packet finPacket;
  uint8_t sendBuf[MSS];
  finPacket.setFlags(0, 0, 1);
  serverSeqNum++;
  finPacket.setSeqNumber(serverSeqNum % MAXSEQ);
  finPacket.setAckNumber(clientSeqNum);
  finPacket.convertPacketToBuffer(sendBuf);
  cout << finPacket.SEND() << endl;

  if (sendto(sockfd, &sendBuf, MSS, 0, (struct sockaddr *)&their_addr,
             sizeof(their_addr)) < 0)
    throwError("Could not send to the client");
  finPacket.startTimer();
  ackPacket.startTimer();
  bool hasBeenSent = false;
  while (1)
  {
    recvlen = recvfrom(sockfd, buf, MSS, 0 | MSG_DONTWAIT,
                       (struct sockaddr *)&their_addr, &sin_size);
    if (recvlen > 0)
    {
      buf[recvlen] = 0;
      receivedPacket.convertBufferToPacket(buf);
      cout << receivedPacket.RECV() << endl;
      if (receivedPacket.getAckNumber() == finPacket.getSeqNumber())
      {
        finPacket.setAcked();
      }
      else if (receivedPacket.getFin())
      {
        // We received the client fin, so now we send back an ack and wait
        // for 2 RTO before quitting
        serverSeqNum++;
        ackPacket.setSeqNumber(serverSeqNum % MAXSEQ);
        ackPacket.setAckNumber(receivedPacket.getSeqNumber());
        ackPacket.setFlags(1, 0, 0);
        cout << ackPacket.SEND() << endl;

        ackPacket.convertPacketToBuffer(sendBuf);
        if (sendto(sockfd, &sendBuf, MSS, 0, (struct sockaddr *)&their_addr,
                   sizeof(their_addr)) < 0)
        {
          throwError("Could not send to the server");
        }
        ackPacket.setSent();
        if (!hasBeenSent)
        {
          ackPacket.startTimer(); // only start the timer once
          hasBeenSent = true;
        }
      }
    }
    if (finPacket.hasTimedOut(1) && !finPacket.isAcked() && retryFin < 100)
    {
      // can retransmit this fin packet as many times as we want
      finPacket.convertPacketToBuffer(sendBuf);
      cout << "SEND " << finPacket.getSeqNumber() << " " << finPacket.getAckNumber()
           << " Retransmission FIN" << endl;
      if (sendto(sockfd, &sendBuf, MSS, 0, (struct sockaddr *)&their_addr,
                 sizeof(their_addr)) < 0)
        throwError("Could not send to the client");
      finPacket.startTimer();
      retryFin++;
    }
    else if ((finPacket.isAcked() && ackPacket.hasTimedOut(2)) ||
             (ackPacket.isSent() && ackPacket.hasTimedOut(2)) ||
             (retryFin >= 100))
    {
      // cout << "Server is done transmitting Closing server" << endl;
      break;
    }
  }
}

int main(int argc, char *argv[])
{
  // Just takes the port number as the argument.

  // cout << "Starting server" << endl;

  // Processing the arguments
  if (argc != 2)
  {
    throwError(
        "Please input the 1 argument: portnumber. Exiting the program ...");
  }
  port = atoi(argv[1]);
  if (port < 1024)
  {
    throwError("Could not process int or trying to use privileged port. "
               "Exiting the program ...");
  }

  int sockfd;
  struct sockaddr_in my_addr;
  struct sockaddr_in their_addr;
  socklen_t sin_size;

  // cout << "Starting making connection" << endl;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    throwError("socket");

  memset((char *)&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(port);
  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (::bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) < 0)
    throwError("bind");
  sin_size = sizeof(struct sockaddr_in);

  // Initiate connection and get the fileName
  string fileName = initiateConnection(sockfd, their_addr);
  cout << endl;

  char *fileBuffer = nullptr;
  long long fileSize = 0;
  ifstream inFile;

  struct stat buffer;
  int fileExists = 0;
  fileExists = stat(fileName.c_str(), &buffer);


  if (fileExists == 0)
  {
    // Open and read in file
    inFile.open(fileName.c_str(), ios::in | ios::binary | ios::ate);
    streampos fs;
    if (inFile.is_open())
    {
      fs = inFile.tellg();
      fileSize = (long long)(fs);
      fileBuffer = new char[(long long)(fs) + 1];
      inFile.seekg(0, ios::beg);
      inFile.read(fileBuffer, fs);
      inFile.close();
    }
    sendChunkedFile(sockfd, their_addr, fileSize, fs, fileBuffer);
    delete fileBuffer;
  }

  cout << endl;


  closeConnection(sockfd, their_addr);

  close(sockfd);
  return 0;
}