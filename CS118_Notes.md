# CS118

## Introduction

- Become an _internet developer_
- 4 quizzes (60%)
  - covering a few chapters each
  - weight tbd
- 2 projects (20%)
  1. Web server
  2. TCP transport protocal
  - C/C++ in linux env
- weekly homework (20%)

## Overview of internet

- web is geographically divided across different internet service providers (ISPs)
- 3 types of hardware components
  - host: laptop, smartphone, server
  - communication links: fiber, radio
  - routers: hardware components for internet, packet switching
- network strucutre
  - edge: hosts, servers
  - access networks: wired/wireless links
  - core: connected routers

## Network edge

- different access networks
  - downstream speeds more important than upstream speeds
  - DSL
    - reuses telephone line
  - cable
    - shared coax cable
  - ethernet
    - high speed network for enterprise
    - connected by ethernet switches
- wireless networks
  - LAN
    - within a building
  - wide area
    - LTE, cellular services

## Network core

- mesh of interconnected routers
- routing: determines source-destination route taken by packets
- forwarding: moving packets from a router's input to another router's output
- packet-switching
  - break messages into packets
  - forward packets from routers to the next using routes computed
  - transmit packets at full speed (back to back)
  - it takes $L/R$ seconds to send $L$ bits at $R$ bps
  - end-end delay is $2L/R$ since it must go through the router
- circuit switching
  - used by telephone networks
  - establish a dedicated circuit (reserve the resource) for the duration of the usage
  - frequency division multiplexer (FDM): each user is partitioned on different frequencies
  - time division multiplexer (TDM): at any given time, a user can use all frequencies
- packet-switching is preferred to circuit-switching
  - more robust; will still work if parts of the network go down
  - support more users can be on the the network by sharing resources; better for bursts of data
  - however can create network congestion and lose packets
  - "reserved resources" for circuit switching
  - "on demand" for packet switching
- computing probability for x active out of N users
  - $P(n,x) = {n \choose x} p^x (1-p)^{N-x}$
  - summation of P(N,x) for 0 up to x to get total
  - use a confidence interval such as <0.1%
  - example: 1 Mb/s link, 100kb/s per user and active 10% of the time
    - circuit switching can support 10 users
    - packet switching can support ~35 users (calculated using probability that 10 of them are active at the same time)
- connecting networks
  - decentralized: naive way is $O(n^2)$
  - centralized: global ISP is $O(n)$ but creates a bottleneck
  - hierarchical: shared network infrastructure with multiple ISP
    - IXP is a third party to connect ISP
    - peering links are "deals" between ISPs
    - regional networks between access nets and ISP as a layer of indirection
    - content provider networks (eg. Google, Microsoft) are connected to tier 1 ISP
    - between access nets and ISP are regional and IXP

## Delay and Loss

- packet delay = time to transmit packet = $L/R$
- end-end delay = $2L/R$
  - router must wait for the entire packet to arrive before forwarding
- queuing occurs if arrival rates to link exceeds transmission rate
  - packets sit in buffer queue when waiting
  - packets can be lost if the buffer fills up
- 4 sources of packet delay
  - processing: router must check the packet, lookup table
  - queuing: wait for queue to be served, depends on congestion
    - traffic intensity $La/R$ where $L$ is packet length, $R$ is bandwidth, $a$ is average packet arrival rate
    - $La/R$ < 0 then delay small, and increases to infinity
  - transmission: $L/R$
    - "putting the packets on the link"
  - propagation: $d/s$ where $d$ is length of link, $s$ is propagation speed (limited by physics)
    - important for large physical distance
- `traceroute` tracks delay along routers until reaching the destination
  - sends 3 packets to each router iteratively
  - high variability
- packet loss
  - can be due to corruption
  - can be dropped by a full queue
    - may be retransmitted by previous node or just lost

## Protocols

- format, order of message, action
- building block for network software
- developed in layered stack
  - ensures modularity and separation between layers
  - more layers is more overhead as each adds header bits
- 5 conventional layers
  - application layer (http)
    - some are open and defined in RFC
    - proprietary for specific applications like Skype
  - transport (TCP, UDP)
  - network (IP)
  - link (WiFi)
    - switch is at this layer and is therefore not visible to the network
  - physical (bits on the wire)

## Application architectures

- client-server model
  - programs to run on different end hosts, asymmetric
    - developer does not need to write any software for the network core
  - server is always on with a permanent IP
    - must be designed to scale (ie large data centers)
  - clients may be dynamic and can only communicate to server directly
  - on the same host, processes can use inter-process communication
  - across hosts, processes exchange messages using socket
- peer-to-peer (P2P)
  - both sides run the same code
  - end systems can directly communicate
  - self-scalability - each peer brings service capacity and demand
  - connections and IP can change

## Addressing processes

- hosts have unique 32-bit IP address to identify
  - ex. 172.217.1.36
- different processes on a host are indicated by port numbers
  - ex. port 80 for http server, port 25 for mail server
- OS knows when messages come into a port, it can send the message to the corresponding process

## TCP vs UDP

- depends on application's needs
  - data integrity; some apps can tolerate loss
  - timing/latency
  - throughput
  - security
- TCP tries to provide everything
  - reliable transport, flow control, congestion throttle
  - requires client/server to setup connection
  - does not provide security
- UDP is generally worse than TCP
  - does not provide the benefits that TCP has
  - also does not provide security
  - but it is faster because less overhead

## Web and HTTP

- client-server model
- the browser is the client
  - webpage is made up of objects such as HTML, image, applet, video
  - sends URL in form [host][path] to the web server
- web server is hosted at port 80 always
- non-persistent HTTP (1.0)
  - algorithm
    - 1 request to setup TCP connection
    - 1 request for each referenced object (by URL)
    - closes connection after recieving object
    - parses html response and decides if subsequent requests are neeeded
  - time is $2*time_{round}$
  - can be parallelized at the cost of OS resources
- persistent HTTP (1.1)
  - keeps the TCP connection open for multiple messages
  - closes connection if inactive
- example: webpage with 10 refence object
  - $2T$ _is used because every message needs to go to server and back_
  - non-persistent HTTP $2T * 2*T*10 = 22T$
  - persistent HTTP $2T * T*10 = 12T$
  - paralled non-persistent HTTP $2T * 2*T$ (for the first 5) $+2*T$ (for the next 5) $= 6T$
- HTTP message
  - request
    - request line with GET/POST and the desired object
    - header lines with parameters such as browser, host, encoding, idle time
    - body
    - encoded into smaller form
  - response
    - status line (200, 400, 404, 505, etc)
    - header lines with parameters such as local time, server OS, last modified, length, type
    - requested data in body
  - HTTP 1.0
    - GET, POST (sends input data), HEAD (doesn't need object)
  - HTTP 1.1
    - all of HTTP 1.0
    - PUT (sends entire file to server)
    - DELETE
  - HTTP 2.0
    - designed for throughput connections
    - multiplexing streams over one connection
    - request-response pipelining
- stateless HTTP
  - server cannot infer whether the client visits before the request is sent
  - modern applications like Amazon have workarounds such as cookies
    - store ids in the local web browser and send in request
    - Amazon stores your id in their database
    - cost is user privacy
- web caches to improve response time
  - maintained by ISPs
  - proxy server closer to client, reduces propagation delay
  - cache information for popular webpages
  - conditional get
    - server only sends new object is it has been updated
    - use last modified time

## Email

- user agents (clients)
  - composing/reading email
- mail server
  - maintains a mailbox for each user
  - message queue of outgoing mails
- SMTP
  - "push" rather than HTTP pull
  - ASCII for everything
  - persistent connections
- mail access protocol
  - POP: post office protocol
  - IMAP: internet mail access protocol
  - HTTP: gmail, office365

## Domain Name System (DNS)

- given the host name find the IP address
- DNS services
  - users need readable names
  - routers need IP addresses
  - host aliasing, mail server aliasing, load distribution
- must be a distributed database
  - traffic volume extremely high
  - centralized system doesn't scale
- hierarchy design concept
  - host names must be hierarchical
    - `kiwi.cs.ucla.edu`
    - hierarchy from right to left
  - each name server handles part of hierarchy
    - abstracted from other layers
- example `www.amazon.com`
  - query root server to find com DNS
  - query .com DNS server to get amazon.com
  - query amazon.com DNS server to get IP address for www.amazon.com
- different DNS servers
  - root name servers
    - only 13 total
    - returns mapping to local name server
  - authorative DNS server
    - maintained by the organization and provide their own hostname to IP mappings
  - local DNS server/default name server
    - ex. CS department server
    - if response is cached, it may return translation
    - otherwise forwards request through the hierarchy
- iterated query (default)
  - "i don't know the name, but ask this server"
  - local DNS server takes burden
- recursive query
  - higher-level servers provide answers on behalf
  - puts heavy load on upper levels
- DNS caches
  - at every level, cache some results
  - cache entries timeout and are removed
  - update/notify mechanisms are in place to indicate outdated cache entries
- DNS protocol uses UDP for both query and reply messages (sync via message ID)

## P2P applications

- end systems can directly communicate
- file distribution client-server
  - $NF/u_s$ is the time to send N copies sequentially
  - $F/d_{min}$ is the time for a client to download
  - $D_{c-s}\geq max\{NF/u_s,F/d_{min}\}$
  - distribution time is proportional to number of clients
- file distribution p2p
  - $NF/(u_s+Su_i)$ for the client to download $NF$ bits total from peers with added service capacity $Su_i$
  - $D_{p2p}\geq max\{F/u_s,F/d_{min},NF/(u_s+Su_i)\}$
  - distribution time grows slower as N increases
- bittorrent

## Content distribution networks

- challenges of scale and heterogeneity
- video formats
  - transmitted as a sequence of images
  - coding uses redundancy within and between images to compress image (MPEG4)
- dynamic adaptive streaming over HTTP (DASH)
  - server divides video into chunks, encoded and stored at different rates
  - manifest file contains URL for each chunk
  - client measures bandwidth and requests appropriate chunk, consulting the manifest
  - puts more intelligence on the client side
- store copies of the videos at different sites around the world
  - client is directed to a nearby CDN
  - video hosting website will redirect you to the IP address of the CDN

# Transport layer

## Transport layer services

- logical communication between processes
- send segments, not packets

## MUX and DeMUX

- multiplexing combines data from multiple sockets at sender
  - add headers to identify process
- demultiplexing is done on the reciever side
  - IP datagram has source and dest IPs
  - port number identifies the process
  - connectionless demultiplexing
    - all UDP segments with the same destination port number will be combined
    - only 1 application process to serve all requests
  - connection oriented demux
    - TCP socket identified by tuple of source/dest IP and ports
    - server can support many simultaneous TCP sockets

## Connectionless transport: UDP

- low overhead, barebones protocol
- segments may be lost or delivered out of order
- each segment is handled independently
- checksum to detect transmission errors
  - convert contents to ints and add with 1's complement
  - reciever recomputes checksum and compares
  - checksum may let errors pass through
- usage
  - streaming, DNS, SNMP

## Reliable data transfer

- making reliable services over an unreliable channel
- formalized using finite state machines
- rdt1.0: channel is reliable
- rdt2.0: channel can have errors
  - use checksum to determine if the packet was corrupted
  - reciever gives sender ACK if good
  - reciever give sender NAK if errors, sender retransmits
- rdt2.1: sequence numbers
  - what if the ACK/NAK message is corrupted? should sender retransmit or drop?
  - add unique (sequence #) id on the sender side so the reciever know whether they are sent a duplicate or next packet
  - 2 sequence numbers is enough to handle corrupted bits
    - rotate between 0 and 1
    - interpretation of ACK/NAK depends on previous state
- rdt2.2: only ACK
  - reciever will send ACK for the last correctly recieved packet
  - duplicate ACK = NAK
    - if server sent pck1 but reciever sends back ack0, that means NAK
- rdt3.0: channels with errors and loss
  - timer to wait for ack
  - if packet is lost, then makes sense to retransmit
  - if packet is delayed, duplicate will be sent but seq #'s handles
  - optimal timer
    - large timer works but is slow, small timer can create interference
    - ideal timeout is slightly larger than past RTT
- pipeline
  - send multiple "in-flight" packets
  - amortize RTT across multiple packets
  - go-back-N
    - sliding window moves when ack recieved
    - reciever drops after missing packet
      - sender will resend N packets after missing packet
    - uses 1 timer, but stores timestamps of all packets
    - discard out of order packets
  - selective repeat
    - send individual ack
    - sender window buffer
      - supports out of order packets
      - cost is more memory
    - selective recovery

## TCP

- basics
  - one sender one reciever
  - reliable, in order byte stream
  - pipelining
  - bi-directional data flow
  - requires handshaking
  - flow controlled
- sequence numbers is byte stream index of the first byte of the segment data
- acknowledgement number is the sequential number expected from the other side
  - using cumulative ACK
- timeout is set by probabalistic function of RTT
  - $estRTT=(1-\alpha)*estRTT+\alpha*SampleRTT$ over a sample
  - $devRTT = (1-\beta)*devRTT+\beta*(|SampleRTT-estRTT|)$
  - $timeout = estRTT + 4*DevRTT$
- sender events
  - make a segment with sequence number and start a timer
  - if timeout, retransmit and restart timer
  - if ack, update what is know to be acked, timer for unACKed segments
- fast retransmit
  - if sender recieves 3 dup ACKs, resent unACKed segment with smallest seq #
- flow control ensures buffers do not overflow
  - reciever indicates how much free buffer they have in `rwnd` field and sender limits "in flight" data accordingly
- handshake to establish parameters for communication
  - 3 way handshake ensure both x and y can ack messages using `SYNbit`
    - listen: SYN from x to y
    - synset: SYNACK to ack SYN from y to x
    - estab: ACK for SYNACK with client to server data
  - close connection using `FINbit`
    - client closes socket but can still recieve data, sends `FINbit=1` to server who ACKs
    - server sends `FINbit=1` back to client who ACKs
    - timeout of $2*maxSegmentLife$

## Congestion control

- prevent overflowing the _network_ (flow control was about the _reciever_)
- can result in lost packets or long delays
- TCP congestion control follows implicit feedback, self clocking
- congestion window `cwnd`
  - increased slowly for new bandwidth, decreased quickly for congestion
  - sender limits transmission to $min($`cwnd,rwnd`$)$
  - upon loss event (timeout or triple ACK), reduce `cwnd`
- additive increase, multiplicative decrease (AIMD)
  - TCP fairness is equal bandwidth to each connection
  - additive increase maintains slope of 1
  - multiplicative decrease bring closer to the line $y=x$
    - equal bandwidth share line
    - cutting half applies differently to connection 1 and 2

## TCP implementation

- slow start - _jumpstarts a new connection_
  - when connection begins, set `cwnd=1MSS`
  - aggressive increase via exponential
    - `cwnd` doubles per RTT
    - `cwnd+=1MSS` for every ACK
    - ex. if 4 ACKs recieved, then `cwnd+=4MSS`
  - stop at slow-start-threshold
    - `ssthresh` initialization is set in OS (guess)
    - on a loss event, set `ssthresh` to half of `cwnd` before the loss event
- congestion avoidance
  - linear increase when `cwnd>ssthresh` 
    - `cwnd++` per RTT
    - `cwnd+=(MSS/cwnd)*MSS` for every ACK
- fast retransmit
  - infer packet loss with 3 dup ACKs 
    - 1-2 dup accounts for out-of-order case
  - `ssthresh=max(cwnd/2, 2MSS)`
  - `cwnd=ssthresh+3MSS`
    - additional `3MSS` accounts for the triple ACK "delay factor"
- fast recovery
  - `cwnd+=1MSS` for every duplicate ACK
  - indicates that the reciever got another packet
- loss event
  - half cwnd 
  - via duplicate ACK -> fast retransmit/fast recovery
  - via retransmission timeout -> slow start

# Network Layer: Data Plane

## Overview of network layer
- between transport and link layer
- on the sender side encapsulates segments into datagrams
- on the receiving side delivers segments to transport layer
- forwarding - move packets between routers
- routing - determining path from source to destination

## Routers
- routing processor software for routing
- high speed switching fabric hardware 
- use header fields to lookup output port in fowarding table
- destination based forwarding
  - forwarding table of address range to link interface
  - use longest prefix match of destination address
- switching fabric
  - memory
    - copies packet and reads for output
    - limited by memory speed
  - bus
    - send data over a shared bus
    - limited by bus bandwidth
  - interconnected network
    - complex-shaped networks (banyan, crossbar, etc.)
- queuing delay
  - input: head of line blocking - datagram at front of queue prevents others in queue
  - output: buffering to hold packets
    - $buf=\frac{RTT*C}{\sqrt{N}}$
- scheduling
  - FIFO, priority, RR

## IP protocol
- forwarding table
- IP datagram: headers (inc. source and dest IP) and data
  - IP protocol number (IPv4 or IPv6)
  - TTL is max number of remaining hops
    - drop packet when 0
  - upper layer protocol (TCP or UDP)
- lot of overhead
  - \>20 bytes of IP headers
  - 20 bytes of TCP headers
- IP fragmentation
  - split datagrams within net
  - reassemble at the final destination 
- IP address
  - 32 bit identifier
  - 4 8 bit parts separated by a period
- subnets can reach each other without the router
  - upper bits match 
- classless interdomain routing (CIDR)
  - `a.b.c.d/x`
  - x is the subnet length (bits)
- getting IP addresses
  - hosts - dynamic host configuration protocol (DHCP)
    - host dynamically gets IP address when connecting to network
    - only hold address when on
    - connection steps
      - discover, offer, request, ACK
  - networks
    - portion of ISP address space 
    - hierarchical addressing for efficiency
  - ISP - ICANN 
    - manages DNS
- network address translation (NAT)
  - datagrams leaving local network have same NAT IP
  - datagrams within network have a local IP
  - local network uses only one IP from the perspective of rest of network
  - IPv4
    - 24 bit `10.0.0.0`
    - 20 bit `172.16.0.0`    
    - 16 bit `192.168.0.0`
- IPv6
  - all addresses in 32-bit address sapce will be allocated
  - fixed 40 byte header
  - no fragmentation
  - no checksum
  - IPv6 carried as a payload in IPv4 routers

# Network Layer: Control Plane

## Overview

- goal is to determine a "good" path from sender to reciever through routers
  - "good" is in terms of cost, speed, congestion
- graph representations
  - edges have costs representing bandwidth or congestion
  - algorithm finds the least cost path

## Routing protocols

- link state - Dijkstra's algorithm
  - link costs are known to all nodes (via a broadcast)
  - compute least cost path from one node to all other nodes iteratively
  - runtime is $O(n^2)$ or $O(nlogn)$ depending on implementation
- distance vector - Bellman Ford
  - dynamic programming
  - $d_x(y) = min \{c(x,v) + d_v(y) \}$
  - iterative: each local iteration is caused by local link cost or $d_v$ update
  - distributed: each node notifies neighbors when $d_v$ changes
- LS complexity and runtime is $O(nE)$ but DV varies
- router malfunctions give bad link cost in LS, but bad path cost in DV

## OSPF

- group routers into _autonomous systems_ (AS)
  - within the AS, same routing protocol used
  - forwarding table is updated by intra-AS and inter-AS protocols
- open shortest path first (OSPF)
  - uses link state algorithm
  - router sends advertisements to all routers in AS

## Border Gateway Protocol (BGP)

- eBGP: obtain subnet reachability information neighboring ASs 
- iBGP: propagate reachability to AS-internal routers
- messages between BGP routers in a session
  - OPEN: opens TCP connection
  - UPDATE: advertises new path (or withdraws old)
  - KEEPALIVE: keeps connection alive if no update
  - NOTIFICATION: reports errors, close connection
- path vector protocol that advertises paths
- path attributes
  - prefix + attributes = route
    - `AS-PATH` contains ASs through which the prefix passed
    - `NEXT-HOP` says which internal AS router the next hop is
- routing based on policy
  1. local preference
  2. shortest `AS-PATH`
  3. closest `NEXT-HOP`

## Internet control message protocol (ICMP)