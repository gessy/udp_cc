Congection Control over UDP
===========================

Client-server application for testing Congestion Control over UDP and using
ECN bits. This is possible to run only at Linux oS. Other OS does not support
`setsockopt()` API.

Installation
------------

This project uses CMake. Thus to compile run following commands:

    mkdir build
    cd ./build
    cmake ../
    make

Testing
-------

Server listens and responds at port 9001. Do not forget to enable this port
on your firewall. To run server type:

   ./server

To run client type:

   ./client server.name.org 
