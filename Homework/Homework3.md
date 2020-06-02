# CS118 Homework 3

Prithvi Kannan
UID: 405110096

## Problem 1
Yes, the protocol will still work. According to the specifications of rdt3.0, if the packet is lost (or does not return ACK in time), the packet will be resent, so the sender side is fine. The reciever will also be okay, because it will treat a lost packet same as an error packet, which is fine. The performance will be significantly impacted if the timeout parameter is too short. This would result in an extra packet for all subsequent packets, and this problem gets worst as n approaches infinity. 

## Problem 2
NAK-only protocol is not preferred to a protocol using ACK. If a packet is lost, the reciever can only detect errors after the next packet arrives. For example, we can only detect that packet n was lost once we saw n-1 and n+1. If n+1 is sent way after n, this can be a long time to detect n was missing. 

If the sender is sending data often, then NAK-only is better than ACK. This is because the recovery described above works quickly when packets are constantly being sent. Also, packets are more likely recieved than lost/corrupted, so there would be less NAKs to send than ACKs. 

## Problem 3

a. Assume the sender has recieved packet k-1 and sent the ACKs for the packets before. If all the ACKs were recieved, then the window is [k, k+5]. If no ACKs were recieved, then the window is [k-6, k-1]. Combining these two cases, the window will be of size 6 and the first element will be in the range [k-6, k]

b. The possible ACK values will be in the range of k-7 to k-1. If the reciever is waiting for k, then it has already ACKed k-1 through k-6. If none of those ACKs were recieved then those may still be in transit. At this time the sender has already recieved an ACK for k-7, so the reciever will not send any ACK for less than k-7. 

## Problem 4

a. The sender will not be kept busy. 

$Utilization = \frac{\frac{5*32000}{8*10^6}}{0.4+\frac{32*10^3}{8*10^6}}=0.45$

$EffectiveThroughput=Utilization*8=3.6 Megabits/s$

b. The minimum window size is 11 and this would use 4 bits.

let $w$ be the window size

$1=\frac{w*\frac{5*32000}{8*10^6}}{0.04+\frac{5*32000}{8*10^6}}$

solving this for $w$ gives 11.

## Problem 5

a. True. Assume the sender has a window size of 4 and sends packets 1, 2, 3, 4 at some time. At then the receiver ACKs 1-4. Shorly after, the timer runs out, so the sender
resends 1-4. Then the reciever notices the duplicates so sends reACKs for each packet. Then the sender recives the ACK and moves the window up to 5-8. After this, the sender can recieve the other set of ACKs for 1-4, which are outside the current window.

b. True. Go back N can have the same issue as described for Selective Repeat.

c. True. Selective repeat and stop and wait are similar, and the exact same is window size is 1.

d. True. The selective repeat buffer can out of order packets within the window. GBN is going to send a bunch of packets again, which is not efficeint and will have significant network communication and memory cost. 