# CS118 Homework 2
Prithvi Kannan
UID: 405110096

## Problem 1

Amazon identifies users when shopping through cookies. Cookies are stored locally in the web browser as an ID that Amazon can use to personalize the shopping experience for that user/device/account. The web sever does this by creating a cookie within Amazon's database the first time the user visits the website. The cookie is sent back to the client in the response so that it can be stored in the client's browser. The next time the client visits Amazon, it will embed the cookie in the request, and Amazon can lookup the cookie and identify the user. 

## Problem 2

a. non-persistent HTTP 

$time_{IP}=RTT_1+RTT_2+...+RTT_n$

$time_{setup} = 2*RTT_0$

$time_{objects}=9*2*RTT_0$

$time_{total}=RTT_1+RTT_2+...+RTT_n + 20*RTT_0$

b. non-persistent HTTP parallel up to 5

$time_{IP}=RTT_1+RTT_2+...+RTT_n$

$time_{setup} = 2*RTT_0$

$time_{objects}=2*2*RTT_0$

$time_{total}=RTT_1+RTT_2+...+RTT_n + 6*RTT_0$

c. persistent HTTP 

$time_{IP}=RTT_1+RTT_2+...+RTT_n$

$time_{setup} = 2*RTT_0$

$time_{objects}=9*RTT_0$

$time_{total}=RTT_1+RTT_2+...+RTT_n + 11*RTT_0$

d. persistent HTTP arbitrary parallel

$time_{IP}=RTT_1+RTT_2+...+RTT_n$

$time_{setup} = 2*RTT_0$

$time_{objects}=RTT_0$

$time_{total}=RTT_1+RTT_2+...+RTT_n + 3*RTT_0$

## Problem 3

SMTP uses "\r\n.\r\n." sequence to incidate the end of a message body. 

HTTP uses the `content-length` header field to indicate where the end of file is. There is no specific character for end of message body. 

HTTP can't use the SMTP method because HTTP can transmit any data format, not just ASCII.

## Problem 4
