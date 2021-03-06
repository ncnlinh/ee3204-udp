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
  uint32_t current_packet_number = 1;
  socklen_t len;
  int errno;
  end = 0;
  
  len = sizeof (struct sockaddr_in);
  srand(time(NULL)); // seed for random number

  printf("receiving data!\n");

  while(!end)
  {
    /********* receive data *********/
    if ((n= recvfrom(sockfd, &packet, sizeof(struct pack_so), 0, (struct sockaddr *)&addr, &len)) == -1)       //receive the packet
    {
      printf("error when receiving: %d\n",errno);
      exit(1);
    }

    else 
    {
      printf("Received %ld bytes, including %u bytes of data\n", n, packet.len);
    
      
      
      /********* build ack **********/
      
      if (rand() % 100 < CORRUPTED_ACK_RATE )
      {
        ack.num = packet.num - 1;
        ack.len = packet.len - 1;
        printf("Corrupted ACK ");
      } 
      else 
      {
        // THE CORRECT ACK
        ack.num = packet.num;
        ack.len = packet.len;
        printf("ACK ");
      }
      /********* send ack *********/

      if ((n = sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&addr, len))==-1)      //send the ack
      {
          printf("Error sending: %d\n",errno);               
          exit(1);
      }
      printf("sent\n");
    }

    /********* save packet *******/
    if (packet.num == current_packet_number)  // if not a duplicate
    { 
      if (packet.data[packet.len-1] == '\0')                 //if it is the end of the file
      {
        printf("End of file!\n");
        end = 1;
        packet.len --; //To remove the NUL character
      }
      
      memcpy((buf+lseek), packet.data, packet.len);
      lseek += packet.len;
    }
    current_packet_number = packet.num +1;
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

