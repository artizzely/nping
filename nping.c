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
/* resolve the reverse lookup of the hostname */
char *reverse_dns_lookup(char *ip_addr);


int main (int argc, char *argv[]) {
  if (argc!= 2) {
    printf("\nFormat %s <address>\n", argv[0]);
    return 0;
  }

  char *ip_addr, *reverse_hostname;
  struct sockaddr_in addr_con;

  ip_addr = dns_lookup(argv[1], &addr_con);
  if (ip_addr == NULL) {
    printf("\nDNS lookup failed: couldn't resolve hostname.\n");
    return 0;
  }

  reverse_hostname = reverse_dns_lookup(ip_addr);
  printf("\nTrying to connect to '%s' IP: %s\n", argv[1], ip_addr);
  printf("\nReverse Lookup domain: %s\n", reverse_hostname);
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

char *reverse_dns_lookup(char *ip_addr) {
  struct sockaddr_in tmp_addr;
  socklen_t len;
  char buf[NI_MAXHOST], *ret_buf;

  tmp_addr.sin_family = AF_INET;
  tmp_addr.sin_addr.s_addr = inet_addr(ip_addr);
  len = sizeof(struct sockaddr_in);

  if (getnameinfo((struct sockaddr *) &tmp_addr, len, buf, sizeof(buf), NULL, 0, NI_NAMEREQD)) {
    printf("\nError: couldn't resolve reverse lookup of hostname\n");
    return NULL;
  }

  ret_buf = (char*) malloc((strlen(buf) + 1) * sizeof(char));
  strcpy(ret_buf, buf);
  return ret_buf;
}