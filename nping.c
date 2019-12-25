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
#include <unistd.h>
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
void send_ping(int ping_sockfd, struct sockaddr_in *ping_addr, char *ping_dom, char *ping_ip, char *rev_host);
/* calculate checksum */
unsigned short checksum(void *b, int len);


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

void send_ping(int ping_sockfd, struct sockaddr_in *ping_addr, char *ping_dom, char *ping_ip, char *rev_host) {
  int ttl_val = 64, msg_count = 0, flag = 1, msg_received_count = 0;
  int i;
  unsigned int addr_len;

  struct ping_pkt pkt;
  struct sockaddr_in r_addr;
  struct timespec time_start, time_end, tfs, tfe;
  struct timeval tv_out;
  long double rtt_msec = 0, total_msec = 0;

  tv_out.tv_sec = RECV_TIMEOUT;
  tv_out.tv_usec = 0;

  clock_gettime(CLOCK_MONOTONIC, &tfs);

  if (setsockopt(ping_sockfd, IPPROTO_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0) {
    printf("\nSending socket options to TTL failed...\n");
    return;
  }

  printf("\nSocket set to TTL...\n");

  setsockopt(ping_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv_out, sizeof(tv_out));

  // programm's infinite loop
  while (pingloop) {
    flag = 1;

    // set up packet
    bzero(&pkt, sizeof(pkt));
    pkt.hdr.icmp_type = ICMP_ECHO;
    pkt.hdr.icmp_id = getpid();
    for (i = 0; i < sizeof(pkt.msg) - 1; ++i) {
      pkt.msg[i] = i + '0';
    }
    pkt.msg[i] = 0;
    pkt.hdr.icmp_cksum = checksum(&pkt, sizeof(pkt));
    pkt.hdr.icmp_seq = msg_count++;

    usleep(PING_SLEEP_RATE);

    // ready to send packet
    clock_gettime(CLOCK_MONOTONIC, &time_start);
    if (sendto(ping_sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *) ping_addr, sizeof(*ping_addr)) <= 0) {
      printf("\nPacket sending failed...\n");
      flag = 0;
    }

    addr_len = sizeof(r_addr);

    // todo I've got a problem here...
    if (recvfrom(ping_sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *) &r_addr, &addr_len) <= 0 && msg_count > 1) {
      printf("\nPacket receive failed...\n");
    } else {
      clock_gettime(CLOCK_MONOTONIC, &time_end);
      double time_elapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec)) / 100000.0;
      rtt_msec = (time_end.tv_sec - time_start.tv_sec) * 1000.0 + time_elapsed;

      if (flag) {
        if (pkt.hdr.icmp_code == 0 && pkt.hdr.icmp_type == 69) {
          printf(
            "%d bytes from %s (h: %s) (%s) msg_seq=%d ttl=%d rtt=%Lf ms.\n", 
            PING_PKT_S, ping_dom, rev_host, ping_ip, msg_count, ttl_val, rtt_msec
          );
          msg_received_count++;
        } else {
          printf("Error...packet received with ICMP type %d code %d\n", pkt.hdr.icmp_type, pkt.hdr.icmp_code);
        }
      }
    }
  }
  clock_gettime(CLOCK_MONOTONIC, &tfe); 
  double time_elapsed = ((double)(tfe.tv_nsec - tfs.tv_nsec)) / 1000000.0; 
    
  total_msec = (tfe.tv_sec - tfs.tv_sec) * 1000.0 + time_elapsed;
                    
  printf("\n===%s ping statistics===\n", ping_ip); 
  printf(
    "\n%d packets sent, %d packets received, %f percent packet loss. Total time: %Lf ms.\n\n",  
    msg_count, msg_received_count, ((msg_count - msg_received_count) / msg_count) * 100.0, total_msec
  );  
}

unsigned short checksum(void *b, int len) {
  unsigned short *buf = b;
  unsigned int sum = 0;
  unsigned short result;

  for (sum = 0; len > 1; len -= 2) {
    sum += *buf++;
  }

  if (len == 1) {
    sum += *(unsigned char *) buf;
  }

  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  result = ~sum; 

  return result;
}