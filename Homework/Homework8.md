# CS118 Homework 8

Prithvi Kannan
UID: 405110096

## Problem 1

1. Obtain IP address via DHCP

    - PC sends DHCP discover message for everyone on the network to see
    - router will send DHCP offer to the client with an IP address and information about the DNS and first hop router
    - PC sends DHCP accept with a chosen IP address
    - router sends DHCP ACK

2. Get MAC address of first hop router using ARP

    - client broadcasts ARP query with IP of first hop router
    - router replies with the MAC address and saves interface in the switching table

3. Send DNS query

    - if local DNS, recursively/iteratively obtain IP of the desired server of the website
    - if no local DNS, let client handle

4. Route to server

    - if server in same AS, use OSPF routing
    - if server is in a different AS and we are moving in the same AS network
    use iBGP routing
    - if the server is in a different AS network, and we are moving between AS networks,
    use eBGP routing

5. Send HTTP request with TCP packed containing datagram

    - create connection with TCP 3 way handshake
    - client send TCP packet with request
    - server sends TCP packet with response
    - client displays response in browser

## Problem 2

a. 

$P(A=5)=(1-P(A))^4*P(A)$ \
$P(A)=p(1-p)^3$ since $P(A)$ means A transmits but not B, C, or D
$P(A)=(1-p(1-p)^3)^4 *p(1-p)^3$

b. 

$P(A=4)=p(1-p)^3$ \
$P(B=4)=p(1-p)^3$ \
$P(C=4)=p(1-p)^3$ \
$P(D=4)=p(1-p)^3$ 

since mutually exclusive $P=4p(1-p)^3$

## Problem 3

a. Switch 1: 3 times \
Switch 2: 4 times \
Switch 3: 4 times

b. Switch 1
|host|interface|
|---|---|
|A|1|
|G|3|
|D|2|
|F|2|
|J|3| 

Switch 2
|host|interface|
|---|---|
|A|1|
|G|2|
|D|1|
|K|3|
|J|3| 

Switch 3
|host|interface|
|---|---|
|A|1|
|D|1|
|K|2|
|J|2|

c. 11


## Problem 4

a. No, 802.11 protocol will not break down. Each AP has their own MAC and SSID so that each the AP can distinguish at the access points which access point the frame is meant for by its SSID and MAC address. The other AP will not process the data because it was not addressed to it. As a result, two AP's can operate over the same channel. However, the two ISPs are still sharing the same wireless bandwidth, and if the ISP's are different, there may be collisions.

b. Since the channel is different, there will be no collisions

## Problem 5
 
The end to end delays of datagrams with be higher with mobility. This is because datagrams will be forwarded to the home agent and then the mobile device. It's also possible that the delay is shorter if the datagram does not have to be routed through the home agent. 

