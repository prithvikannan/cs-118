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
  - transport (TCP, UDP)
  - network (IP)
  - link (WiFi
    - switch is at this layer and is therefore not visible to the network
  - physical (bits on the wire)

## Application architectures
- client-Server model
  - programs to run on different end hosts, asymmetric
    - developer does not need to write any software for the network core
  - server is always on with a permanent IP
  - clients may be dynamic and can only communicate to server directly
- peer-to-peer (P2P)
  - both sides run the same code