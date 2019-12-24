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
#include <signal.h>
#include <time.h>
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
/* interrupt handler */
void intHandler(int dummy);
/* send ping request */
void send_ping(int ping_sockfd, struct sockaddr_in *ping_addr, char ping_dom, char *ping_ip, char *rev_host);


int main (int argc, char *argv[]) {
  if (argc!= 2) {
    printf("\nFormat %s <address>\n", argv[0]);
    return 0;
  }

  int sockfd;
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

  // to create socket, need run with 'sudo'
  sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (sockfd < 0) {
    printf("\nSocket file descriptor not received.\n");
    return 0;
  }
  printf("\nSocket file descriptor %d received\n", sockfd);

  // bind handler on interrupt signal (ctrl+c)
  signal(SIGINT, intHandler);

  // send ping
  send_ping(sockfd, &addr_con, reverse_hostname, ip_addr, argv[1]);

  return 0;
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

void intHandler(int dummy) {
  pingloop = 0;
}

void send_ping(int ping_sockfd, struct sockaddr_in *ping_addr, char ping_dom, char *ping_ip, char *rev_host) {
  int ttl_val = 64, msg_count = 0, flag = 1, msg_received_count = 0;
  int i, addr_len;

  struct ping_pkt pkt;
  struct sockaddr_in r_addr;
  struct timespec time_start, time_end, tfs, tfe;

  clock_gettime(CLOCK_MONOTONIC, &tfs);

  if (setsockopt(ping_sockfd, IPPROTO_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0) {
    printf("\nSending socket options to TTL failed...\n");
    return;
  }

  printf("\nSocket set to TTL...\n")

  // programm's loop
  while (pingloop) {

  }
}