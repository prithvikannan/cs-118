# CS118 Homework 6

Prithvi Kannan
UID: 405110096

## Problem 1

a. No, since the bus is shared between the different input and output ports. Busses only supports a single packet at a time.

b. No, shared memory will use the system bus, which has the same constraint of a single packet at a time.

c. Yes, a crossbar will allow multiple packets simultaneously.

## Problem 2

for 60 interfaces, need 6 bits \
for 90 interfaces, need 7 bits \
for 8 interfaces, need 3 bits

subnet 1: `224.1.17.128/26` \
subnet 2: `224.1.17.0/25` \
subnet 3: `224.1.17.248/29` 

## Problem 3

a. $\lceil \frac{2400-20}{800-20} \rceil = 4$ fragments

b. fields 

id number = 421 \
header length = 20
total length = 800 for the first 3, 60 for the last \
MF flag = 1 for the first 3, 0 for the last \
offset = 0, 85, 170, 255

## Problem 4

a. According to RFC 791 says, "If the header checksum fails, the internet datagram is discarded at once by the entity which detects the error." \
Since some header fields change (e.g., time to live), this is recomputed and verified at each point that the internet header is processed. 

b. IPv4 has a header checksum to detects errors in the header, and it discards any packets not matching the header checksum, the payload never reaching the transport layer. 

TCP has a checksum that covers the TCP pseudo header and payload. It is optional for UDP on IPv4, but mandatory for UDP on IPv6. Other transport protocols may have error detection, and others may not. If a transport protocols does not have error detection, it is up to the application to check for errors.

## Problem 5

This is not possible under NAT. Since
both P2P users are behind NAT, they cannot establish a direct TCP connection as the NAT will drop the handshake of TCP. This is because the NAT table has not been created for bernard since there were no outgoing connections.