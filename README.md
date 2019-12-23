# nping
NPing is a custom base tool that allows user to verify that particular IP address is alive and can requests.

# Overview
Like the standart ping, the NPing sending a small packet of information in **ECHO_REQUEST** to a target computer, which then sends an **ECHO_REPLY** packet via **ICMP** ([Internet Control Message Protocol](https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol)).

# Environment
* Mac OS
* GCC (Apple clang version 11.0.0)

# Build and Run
``` 
$ git clone git@github.com:smile-h/nping.git
$ cd nping
$ make
$ ./nping <address>
```

# How to use
```
$ ./nping google.com

Resolve DNS...

Trying to connectto 'google.com' IP: 64.233.165.113

Reverse Lookup domain: lg-in-f113.1e100.net
```

# todo list
*
