/*******************************
udp_client.c: the source file of the client in udp
********************************/

#include "headsock.h"

float str_cli(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, int *len);
void tv_sub(struct  timeval *out, struct timeval *in);
int main(int argc, char *argv[])
{
  int sockfd;
  float ti, rt;
  int len;
  struct sockaddr_in ser_addr;
  char **pptr;
  struct hostent *sh;
  struct in_addr **addrs;
  
  FILE *fp;

  /********* initialize *********/

  if (argc!= 2)
  {
    printf("parameters not match.");
    exit(0);
  }

  if ((sh=gethostbyname(argv[1]))==NULL) {             //get host's information
    printf("error when gethostbyname");
    exit(0);
  }

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);             //create socket b(udp)
  if (sockfd<0)
  {
    printf("error in socket");
    exit(1);
  }

  addrs = (struct in_addr **)sh->h_addr_list;       //get the server(receiver)'s ip address
  printf("canonical name: %s\n", sh->h_name);
  for (pptr=sh->h_aliases; *pptr != NULL; pptr++)
    printf("the aliases name is: %s\n", *pptr);     //printf socket information
  switch(sh->h_addrtype)
  {
    case AF_INET:
      printf("AF_INET\n");
    break;
    default:
      printf("unknown addrtype\n");
    break;
  }

  ser_addr.sin_family = AF_INET;        // Address family, AF_INET
  ser_addr.sin_port = htons(MYUDP_PORT);
  memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
  bzero(&(ser_addr.sin_zero), 8);

  /********* open file *********/

  if((fp = fopen ("myfile.txt","r+t")) == NULL)
  {
    printf("File doesn't exit\n");
    exit(0);
  }

  /********* transmission *********/

  ti = str_cli(fp, sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr_in), &len);  

  /********* calculations *********/

  rt = (len/(float)ti);
  printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s)\n", ti, (int)len, rt);
  
  /********* deinitialise *********/

  close(sockfd);
  fclose(fp);
  exit(0);
}

/*
 * Transmit data from file through socket and return
 * total transmission time
 *
 */
float str_cli(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, int *len)
{
  char *buf;
  long lsize, ci;
  struct pack_so packet;
  struct ack_so ack;
  int n;
  int errno;
  float time_inv = 0.0;
  struct timeval sendt, recvt;
  int current_packet_number = 1;
  int prev_packet_acked = 1;
  
  ci = 0;

  fseek (fp , 0 , SEEK_END);
  lsize = ftell (fp);
  rewind (fp);
  printf("The file length is %d bytes\n", (int)lsize);
  printf("the packet length is %d bytes\n",DATALEN);

  /********* initialise *********/

  // allocate memory to contain the whole file.
  buf = (char *) malloc (lsize);
  if (buf == NULL) exit (2);

  // copy the file into the buffer.
  fread (buf,1,lsize,fp); // the whole file is loaded in the buffer. 
  buf[lsize] ='\0';                 //append the end byte

  /********* start transmission *********/
  gettimeofday(&sendt, NULL);             //get the current time
  while(ci<= lsize)
  {
    /********* send new data *********/
    if (prev_packet_acked) {
      if ((lsize+1-ci) <= DATALEN)
        packet.len = lsize+1-ci;
      else 
        packet.len = DATALEN;
      packet.num = current_packet_number;
      memcpy(packet.data, (buf+ci), packet.len);
      n = sendto(sockfd, &packet, packet.len, 0, addr, addrlen);       //send the packet to server
      if(n == -1) {
        printf("send error: %d \n", errno);                
        exit(1);
      }
      prev_packet_acked = 0;
      printf("Sent %i bytes, waiting for ACK\n", packet.len);
    }
    /********* receive ack *********/
    if ((n= recvfrom(sockfd, &ack, 2, 0, (struct sockaddr *)&addr, (socklen_t *) len)) == -1)       //receive the packet
    {
      printf("error when receiving: %d\n", errno);
      exit(1);
    }
    printf("Received ACK\n");

    /********* evaluate ack to either advance or resend*********/
    if (ack.num != current_packet_number || ack.len != packet.len) {
      /********* retransmission *********/
      printf("Expected ACK for %i with len %i but receive %i - %i\n", current_packet_number, packet.len, ack.num, ack.len);
      printf("Incorrect ack. Resend\n");
      n = sendto(sockfd, &packet, packet.len, 0, addr, addrlen);       //resend the packet
      if(n == -1) {
        printf("send error: %d \n", errno);                
        exit(1);
      }
    } else {
      /********* advance *********/
      printf("Correct. Send next.\n");
      ci += packet.len;
      current_packet_number++;
      prev_packet_acked = 1;
    }

    
  } 
  gettimeofday(&recvt, NULL);
  *len= ci;                                                 //get current time
  tv_sub(&recvt, &sendt);                                // get the whole trans time
  time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
  return(time_inv);
}

void tv_sub(struct  timeval *out, struct timeval *in)
{
  if ((out->tv_usec -= in->tv_usec) <0)
  {
    --out ->tv_sec;
    out ->tv_usec += 1000000;
  }
  out->tv_sec -= in->tv_sec;
}

// void str_cli1(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, int *len)
// {
//   char packet.data[MAXSIZE];

//   printf("Please input a string (less than 50 characters):\n");
//   if (fgets(packet.data, MAXSIZE, fp) == NULL) {
//     printf("error input\n");
//   }

//   sendto(sockfd, &packet.data, strlen(packet.data), 0, addr, addrlen);                         //send the packet to server
//   printf("send out!!\n");
// }
