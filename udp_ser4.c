/**************************************
udp_ser.c: the source file of the server in udp transmission
**************************************/
#include "headsock.h"

void str_ser(int sockfd);             // transmitting and receiving function

int main(int argc, char *argv[])
{
  int sockfd;
  struct sockaddr_in my_addr;

  /********* initialize *********/

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {      //create socket
    printf("error in socket");
    exit(1);
  }

  my_addr.sin_family = AF_INET;          // Address family,  AF_INET
  my_addr.sin_port = htons(MYUDP_PORT);  // UDP port
  my_addr.sin_addr.s_addr = INADDR_ANY;  // all interfaces
  bzero(&(my_addr.sin_zero), 8); 
  if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {           //bind socket
    printf("error in binding");
    exit(1);
  }

  printf("start receiving\n");

  /********* transmission *********/
  str_ser(sockfd);                        // send and receive

  /********* deinitialize *********/
  close(sockfd);
  exit(0);
}

void str_ser(int sockfd)
{
  char buf[BUFSIZE];
  FILE *fp;
  struct sockaddr_in addr;
  struct ack_so ack;
  struct pack_so packet;
  long end, n = 0;
  long lseek=0;
  socklen_t len;
  int errno;
  end = 0;
  
  len = sizeof (struct sockaddr_in);
  srand(time(NULL)); // seed for random number

  printf("receiving data!\n");

  while(!end)
  {
    /********* receive data *********/
    if ((n= recvfrom(sockfd, &packet, BUFSIZE, 0, (struct sockaddr *)&addr, &len)) == -1)       //receive the packet
    {
      printf("error when receiving: %d\n",errno);
      exit(1);
    }

    else 
    {
      printf("Received %ld bytes\n", n);
    
      /********* store in buffer *********/

      if (packet.data[n-1] == '\0')                 //if it is the end of the file
      {
        printf("End of file!\n");
        end = 1;
        n --;
      }
      memcpy((buf+lseek), packet.data, n);
      lseek += n;
      /********* build ack **********/

      // THE CORRECT ACK
      ack.num = packet.num;
      ack.len = packet.len;
      
      /********* send ack *********/

      if ((n = sendto(sockfd, &ack, 2, 0, (struct sockaddr *)&addr, len))==-1)      //send the ack
      {
          printf("Error sending: %d\n",errno);               
          exit(1);
      }
      printf("Sent ACK\n");
      }
  }

  /********* store to file *********/

  if ((fp = fopen ("myUDPreceive.txt","wt")) == NULL)
  {
    printf("File doesn't exit\n");
    exit(0);
  }
  fwrite (buf , 1 , lseek , fp);          //write data into file
  fclose(fp);
  printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
}

