//
// Created by 董子豪 on 1/16/22.
//
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
struct header{
  int size;
  int seq;
  char payload[2048];
};

int main(int argc, char *argv[]){
  if (argc!=6){
    fprintf(stderr, "usage /myclient <server ip> <server port> <mtu> <infile path> <outfile path>");
    exit(-1);
  }

  char *ip_address = argv[1];
  int port_number = atol(argv[2]);
  if (port_number<1024||port_number>65535){
    fprintf(stderr, "port need to be between 1024 and 65535");
    exit(-1);
  }
  int mtu = atol(argv[3]);
  if (mtu>2048){
    fprintf(stderr, "mtu should be less than 2048");
    exit(-1);
  }
  char *input_path = argv[4];
  char *output_path = argv[5];
  int socketfd = socket(AF_INET,SOCK_DGRAM,0);
  if (socketfd<0){
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

  struct sockaddr_in server_address;
  bzero(&server_address,sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port= htons(port_number);
  server_address.sin_addr.s_addr = inet_addr(ip_address);
  socklen_t length = sizeof (server_address);

  int in = open(input_path,O_RDWR,0644);
  if (in==-1){
    fprintf(stderr,"can't open");
    exit(-1);
  }

  struct stat info;
  fstat(in,&info);
  int filesize = info.st_size;



  int i = 0;
  int count= 0 ;

  long packagenumber = ceil(1.0*filesize/mtu);
  fprintf(stderr,"%ld\n",packagenumber);
  struct sockaddr_in rece_address;

  struct header *result = malloc(packagenumber*sizeof(struct header));
  for (int j = 0; j < packagenumber; ++j) {
    result[i].size=0;
    result->seq = -1;
  }
  struct header test_header = {-1,-1,"test"};
  sendto(socketfd,&test_header,mtu,0,(struct sockaddr *)&server_address,length);
  struct header rec_test;
  int receive_size = recvfrom(socketfd,&rec_test,sizeof(rec_test),0,(struct sockaddr *)&rec_test,&length);
  if (receive_size<0){
    fprintf(stderr,"Can't find server\n");
    exit(-1);
  }
  while (i<filesize){
    char temp[mtu];
    int read_size = read(in,temp,mtu);

    struct header h;
    h.seq = count;
//    h.payload = temp;
    memcpy(h.payload, temp, read_size+1);
//    strcpy(h.payload,temp);
    h.size = read_size;
//    printf("send : seq : %d read :%d\n",h.seq,h.size);
    sendto(socketfd,&h,mtu+8,0,(struct sockaddr *)&server_address,length);
    i+= read_size;
    count++;
    //
    struct header rece_header;
    int receive_size = recvfrom(socketfd,&rece_header,sizeof(rece_header),0,(struct sockaddr *)&rece_address,&length);
    if(receive_size==-1){
      fprintf(stderr,"time out\n");
      continue;
    }


    result[rece_header.seq] = rece_header;
  }

  for (int j = 0; j < packagenumber; ++j) {
    if (result[j].size>mtu||result[j].size<=0){
      fprintf(stderr,"Package Lost");
      exit(-1);
    }
  }
  int output = open(output_path,O_CREAT|O_WRONLY|O_TRUNC,S_IREAD|S_IWRITE);
  if (output==-1){
    fprintf(stderr,"can't open output file");
    exit(-1);
  }
  for (int j = 0; j < packagenumber; ++j) {
    if (write(output,result[j].payload,result[j].size)!=result[j].size){
      fprintf(stderr, "error writing\n");
      continue;
    }
  }
  close(in);
  close(output);
  close(socketfd);
}
