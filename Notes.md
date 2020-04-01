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
  - it takes L/R seconds to send L bits at R bps
  - end-end delay is 2L/R since it must go through the router
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
  - $P(n,x) = {n \choose x}p^x (1-p)^{N-x}$
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