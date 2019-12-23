#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "nping.h"

int pingloop = 1;

/* ping packet */
struct ping_pkt
{
  struct icmp hdr;
  char msg[PING_PKT_S-sizeof(struct icmp)];
};

/* perform a DNS lookup */
char *dns_lookup(char *addr_host, struct sockaddr_in *addr_con);


int main (int argc, char *argv[]) {
  if (argc!= 2) {
    printf("\nFormat %s <address>\n", argv[0]);
    return 0;
  }

  char *ip_addr;
  struct sockaddr_in addr_con;

  ip_addr = dns_lookup(argv[1], &addr_con);
  if (ip_addr == NULL) {
    printf("\nDNS lookup failed: couldn't resolve hostname.\n");
    return 0;
  }

  printf("IP: %s\n", ip_addr);
}

char *dns_lookup(char *addr_host, struct sockaddr_in *addr_con) {
  printf("\nResolve DNS...\n");
  struct hostent *host_entity;
  char *ip = (char*)malloc(NI_MAXHOST*sizeof(char));

  if((host_entity = gethostbyname(addr_host)) == NULL) {
    printf("/nERROR: no ip found for %s/n", addr_host);
    return NULL;
  }
  // filling up address
  strcpy(ip, inet_ntoa(*(struct in_addr *) host_entity -> h_addr));
  (*addr_con).sin_family = host_entity -> h_addrtype;
  (*addr_con).sin_addr.s_addr = *(long *) host_entity -> h_addr;
  (*addr_con).sin_port = htons(PORT_NO);

  return ip;
}