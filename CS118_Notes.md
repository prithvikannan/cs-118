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