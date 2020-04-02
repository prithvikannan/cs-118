# CS118 Homework 1
Prithvi Kannan
UID: 405110096

## Problem 1
a. $100 Mbps / 10 Mbps = 10 users$

b. 25 %

c. $P(x = n) = {100 \choose n} * 0.25^n * (1-0.25)^{100-n}$

d. $P(x\geq 21) = {\sum_{n=21}^{100} {100 \choose n} * 0.25^n * (1-0.25)^{100-n}}$

## Problem 2

a. There is no delay for the first packet. The 2nd packet has L/R, followed by packets of delay 2L/R, 3L/R, and so on.

$\overline{delay} = \frac{L + 2L + 3L + ... + (N-1)L}{R}$

$\overline{delay} = \frac{L(1 + 2 + 3 + ... + (N-1)}{R}$

$\overline{delay} = \frac{L(N-1)}{2R}$

b. The answer is the same as part a. Each transmission takes $\frac{LN}{R}$ time so the buffer will be empty when N packets arrive.


## Problem 3

$delay = time_{toll} + time_{travel}$

a. 5 cars in caravan 

$time_{toll}=5\space cars*3\space booths*12\space seconds=3\space mins$

$time_{travel}=2*\frac{50\space km/h}{100\space km} = 60\space mins$

$time_{total}=63\space mins$

b. 8 cars in caravan 

$time_{toll}=8\space cars*3\space booths*12\space seconds=4.8\space mins$

$time_{travel}=2*\frac{50\space km/h}{100\space km} = 60\space mins$

$time_{total}=64.8\space mins$

## Problem 4

$time_{processing} = \frac{1s}{64 Kbps} * \frac{8 bit}{1 byte} * 56\space bytes = 0.007\space sec$

$time_{trans} = \frac{1s}{2 Mbps} * \frac{8 bit}{1 byte} * 56\space bytes = 0.00022\space sec$

$time_{prop} = 0.01\space sec$

$time_{total} = 0.007 + 0.00022 + 0.01 = 0.017\space sec$
## Problem 5

$time_{link} = 50 TB * \frac{8*10^{12}b}{TB} * \frac{1s}{2 Gbps} = 56\space hours$

$time_{overnight} = 24\space hours$

Overnight delivery is the preferred option since it is much faster.