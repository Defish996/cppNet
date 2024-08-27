#pragma once 
#define CONV(addr_ptr) ((struct sockaddr*)addr_ptr)
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
enum{
    Usage_Err = 1,
    Socket_Err,
    Bind_Err,
    Listen_Err
};