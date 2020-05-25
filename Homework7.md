# CS118 Homework 7

Prithvi Kannan
UID: 405110096

## Problem 1

a. NAT table
| ip port inside | ip port outside |
|---|---|
|10.0.0.5:5000|128.97.27.37:8000|
|10.0.0.6:5000|128.97.27.37:8001|
|10.0.0.10:6000|128.97.27.37:8002|
|10.0.0.101:6001|128.97.27.37:8003|
|10.0.0.7:7000|128.97.27.37:8004|
|204.79.197.200:80|128.97.27.37:8005|


b.  

Message Received from Host: `MSG <10.0.0.6:5000, 172.217.11.78:80>` \
Message Sent from Router: `MSG <128.97.27.37:8001, 172.217.11.78:80>`

Message Received from Host: `MSG <10.0.0.10:6000, 172.217.11.78:80>` \
Message Sent from Router: `MSG <128.97.27.37:8002, 172.217.11.78:80>`

## Problem 2

a. The 8 bit protocol field in IP datagram has the information so the network knows to pass the segment via TCP or UDP. 

b. If you have n network interface cards on your computer, then you can have n IP addresses. Also, if using a virtual machine (such as VirtualBox or VMWare) you have a virtual adapter which is the equivalent of a NIC, so you can have n IP addresses for each VM running.

c. Skype uses UDP hole punching to circumvent the challenges of hosts behind two NAT firewalls. The NAT forwards packets when it is convinced that the packet is _outgoing_. Skype uses UDP so the firewall only sees the addresses and ports of source and desination, and if an incoming UDP packet matches a NAT table entry, it will pass it on. This allows clients to set up p2p UDP conneciton behind NATs. 

d. NAT will probably not be needed for it's current purpose if IPv6 is globally deployed. NAT allows connections to appear to use less public IP addresses by adding a layer of indirection, which is important since IPv4 supports only 2^32 ≈ 4.7 billion addresses. On the contrary, IPv6 supports 2^128 addresses, so NAT will probably not be needed.

However, NAT does offer some security benefits that should at least be discussed, such as not giving out unnecessary information about network topology.

## Problem 3
|Step |N' |D(t),p(t) |D(u),p(u) |D(v),p(v) |D(w),p(w) |D(x),p(x)| D(y),p(y)|
|---|---|---|---|---|---|---|---|
|0|z |∞ |∞ |∞ |∞ |8,z |12,z |
|1|zx |∞ |∞ |11,x |14,x | | 12,z|
|2|zxv |15,v |14,v | |14,x | | 12,z|
|3|zxvy |15,v |14,v | |14,x | | |
|4|zxvyu |15,v | | |14,x | | |
|5|zxvyuw |15,v | | | | | |
|6|zxvyuwt | | | | | | |

## Problem 4

a. eBGP \
b. iBGP \
c. eBGP \
d. iBGP

## Problem 5

c1 -> c2: 

$H\rightarrow I\rightarrow exchange \rightarrow F\rightarrow D\rightarrow C\rightarrow B$ \
$= 5+5+5+10+35+20+5=85$ ms

c2 -> c1:

$B\rightarrow C\rightarrow A\rightarrow G\rightarrow H\rightarrow J$ \
$= 5+10+5+10+5$ ms

These routes are not symmetric