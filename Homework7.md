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

b.  

Message Received from Host: `MSG <10.0.0.6:5000, 172.217.11.78:80>` \
Message Sent from Router: `MSG <128.97.27.37:8001, 172.217.11.78:80>`

Message Received from Host: `MSG <10.0.0.10:6000, 172.217.11.78:80>` \
Message Sent from Router: `MSG <128.97.27.37:8002, 172.217.11.78:80>`

## Problem 2

## Problem 3

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