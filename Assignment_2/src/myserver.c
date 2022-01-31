//
// Created by 董子豪 on 1/16/22.
//
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
struct header{
  int size;
  int seq;
  char payload[2048];
};
int main(int argc, char *argv[]){
//  char *ip_address = "127.0.0.1";
  if (argc!=2){
    fprintf(stderr,"usage ./myserver <port number>\n");
    exit(-1);
  }
  int port_number = atol(argv[1]);
  if (port_number<1024||port_number>65535){
    fprintf(stderr, "port need to be between 1024 and 65535");
    exit(-1);
  }
  //printf("%d\n",port_number);

  int socketfd = socket(AF_INET,SOCK_DGRAM,0);
  if(socketfd<0){
    fprintf(stderr,"Can't create sock");
    exit(-1);
  }
  struct timeval timout;
  timout.tv_sec = 1;
  timout.tv_usec = 0;
  if(setsockopt(socketfd,SOL_SOCKET,SO_RCVTIMEO,&timout, sizeof(timout))<0){
    fprintf(stderr,"can't set time out\n");
    exit(1);
  }
  struct sockaddr_in address;
  bzero(&address,sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port= htons(port_number);
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  int bind_result = bind(socketfd,(struct sockaddr *)&address,sizeof(address));
  if (bind_result<0){
    fprintf(stderr,"Can't blind sock");
    exit(-1);
  }
  socklen_t length = sizeof (address);
//  char buffer[1024] = {0};

  while (1) {
    struct sockaddr_in send;
    struct header h;
    recvfrom(socketfd, &h, sizeof(h), 0, (struct sockaddr *)&send, &length);//接受
    sendto(socketfd,&h,sizeof(h),0,(struct sockaddr *)&send, length);//送回
  }

}
