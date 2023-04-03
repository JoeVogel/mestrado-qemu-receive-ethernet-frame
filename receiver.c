#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

/* Return the index of the given device name */
static int32_t iface_to_id(int32_t sk, int8_t *device)
{
   struct ifreq ifr;
   int32_t ret;

   memset(&ifr, 0, sizeof(struct ifreq));
   strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));

   ret = ioctl(sk, SIOCGIFINDEX, &ifr);
   if (ret < 0)
   {
      printf("failed to get index\n");
      return -1;
   }

   return ifr.ifr_ifindex;
}

/* Bint the socket to the given device */
static int32_t iface_bind_to_device(int32_t sk, int32_t ifindex, int32_t protocol)
{
   struct sockaddr_ll   sll;
   int32_t ret;

   memset(&sll, 0x0, sizeof(struct sockaddr_ll));
   sll.sll_family    = AF_PACKET;
   sll.sll_ifindex   = ifindex;
   sll.sll_protocol  = protocol;

   ret = bind(sk, (struct sockaddr *)&sll, sizeof(struct sockaddr_ll));
   if (ret == -1)
   {
      return -1;
   }

   return 0;
}

int32_t main(int32_t argc, int8_t *argv[])
{
   int32_t sock;
   int8_t buf[1522];
   int32_t ifindex;
   int32_t ret;
   int32_t bytes;
   struct packet_mreq mr;

   sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
   if (sock == -1)
   {
      printf("socket open failed\n");
      return 1;
   }

   ifindex = iface_to_id(sock, "eth0");
   if (ifindex < 0)
   {
      close(sock);
      return 1;
   }

   ret = iface_bind_to_device(sock, ifindex, htons(ETH_P_ALL));
   if (ret < 0)
   {
      close(sock);
      printf("interface binding error\n");
      return 1;
   }

   while(1)
   {
      bytes = recvfrom(sock, buf, 1522, 0, NULL, NULL);
      if (bytes < 0)
      {
         printf("error in recvfrom\n");
         exit;
      }
      printf("received bytes = %d\n", bytes);
      
      struct ethhdr *eth = (struct ethhdr *)(buf);
	  printf("\nEthernet Header\n");
	  printf("\t|-Source Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
	  printf("\t|-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);
      printf("\t|-Protocol : %d\n",eth->h_proto);

   }

   close(sock);
   return 0;
}