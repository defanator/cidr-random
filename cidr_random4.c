// vim:sw=4:ts=4:et:

/*
 * Copyright (C) 2021 Andrei Belov (@defanator on github)
 *
 * Inspired by https://stackoverflow.com/questions/64542446/choosing-a-random-ip-from-any-specific-cidr-range-in-c,
 * based on original code by Scott Tadman (@tadman on github)
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* number of addresses to generate */
#define MAX_RANDOM_ADDRESSES 30

/* 4 octets in binary form + dots between octets + trailing zero */
#define INET_ADDRBINSTRLEN ((8 * 4) + (1 * 3) + 1UL)

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN sizeof("255.255.255.255")
#endif

char * sprintb(char *dst, void const * const ptr, size_t const size)
{
    int i, j;
    char *dp = dst;
    unsigned char byte;
    unsigned char *b = (unsigned char*) ptr;
    
    for (i = size - 1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            *dp++ = byte ? '1' : '0';
        }
    }

    *dp = 0x0;
    return dst;
}

char * sprintb_addr4(char *dst, struct in_addr *in4) {
    char *dp = dst;
    u_char *p = (u_char *) &in4->s_addr;

    for (int i = 0; i < 4; i++) {
        sprintb(dp, &p[i], sizeof(u_char));
        dp += 8;
        if (i < 3) *dp++ = '.';
    }

    return dst;
}

int main(int argc, char** argv)
{
    char *ipnet;
    uint8_t prefixlen;
    struct in_addr addr4, mask4, rand4;
    uint32_t mask_len, net_ip, rand_ip;
    char straddr4[INET_ADDRSTRLEN];
    char binaddr4[INET_ADDRBINSTRLEN];

    if (argc < 3) {
        printf("usage: cidr_random4 net prefixlen\n");
        printf("  e.g. cidr_random4 192.168.1.0 24\n");
        printf("       cidr_random4 10.10.0.0 16\n");
        printf("       cidr_random4 127.0.0.0 8\n");
        exit(-1);
    }

    ipnet = argv[1];
    prefixlen = atoi(argv[2]);

    if (inet_aton(ipnet, &addr4) < 1) {
        printf("incorrect IPv4 address/net: \"%s\"\n", ipnet);
        return(1);
    }

    mask4.s_addr = htonl((uint32_t) (0xffffffffu << (32 - prefixlen)));

    if (addr4.s_addr != (addr4.s_addr & mask4.s_addr)) {
        addr4.s_addr &= mask4.s_addr;

        printf("WARNING: low address bits of %s/%d are meaningless\n",
               ipnet, prefixlen);
    }

    net_ip = ntohl(addr4.s_addr);
    mask_len = 0xffffffffu - ntohl(mask4.s_addr);

    inet_ntop(AF_INET, &addr4, straddr4, INET_ADDRSTRLEN);
    printf("network: %s/%d\n", straddr4, prefixlen);

    inet_ntop(AF_INET, &mask4, straddr4, INET_ADDRSTRLEN);
    printf("netmask: %s (free bits=%d)\n", straddr4, 32 - prefixlen);

    printf("%s\n", sprintb_addr4(binaddr4, &addr4));
    printf("%s\n", sprintb_addr4(binaddr4, &mask4));
    printf("----\n");

    srand(((unsigned) getpid() << 16) ^ time(NULL));

    for (int i = 0; i < MAX_RANDOM_ADDRESSES; i++) {
        uint32_t rv = rand();

        rand_ip = (net_ip & ~mask_len) | (mask_len & rv);

        rand4.s_addr = htonl(rand_ip);
        printf("%s %s\n", sprintb_addr4(binaddr4, &rand4), inet_ntoa(rand4));
  }

  return(0);
}
