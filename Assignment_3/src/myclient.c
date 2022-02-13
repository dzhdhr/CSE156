//
// Created by 董子豪 on 1/30/22.
//
#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#define ACK 1
#define DATA 2
#define ERROR 3
#define FIN 4
#define TEST 5
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

struct header{
  int size;
  int seq;
  int type;
  char payload[2048];
  int window_size;
  char filename[64];
  int base_seq;
  int count;
};

List genfromfile(int mtu, int fd,int file_size) {
  List ret = newList();
  int count = 0;
  int i = 0;
  while (count<file_size){
    struct header *test_header = malloc(sizeof(struct header));
    int read_size = read(fd,test_header->payload,mtu);
    test_header->size = read_size;
    test_header->window_size = -1;
    test_header->type=DATA;
    test_header->seq = i;
    test_header->base_seq = -1;
    test_header->count = 0;
    i++;
    append(ret,test_header);
    count+=read_size;
  }
  return ret;
}

int main(int argc, char *argv[]){
  if (argc!=7){
    fprintf(stderr,"Usage : ./myclient <server_ip> <server_port> <mtu> <winsz> <infile_path> <outfile_path>\n");
    exit(-1);
  }
  char *ip_address = argv[1];
  int port_number =atol(argv[2]);
  if (port_number<1024||port_number>65535){
    fprintf(stderr, "port need to be between 1024 and 65535\n");
    exit(-1);
  }
  int mtu = atol(argv[3]);
  if (mtu<124||mtu>=2048){
    fprintf(stderr, "mtu need to be between 125 and 2047\n");
    exit(-1);
  }
  int window_size = atol(argv[4]);
  if (window_size<0||window_size>499){
    fprintf(stderr, "window size need to be between 1 and 499\n");
    exit(-1);
  }
  char *in_filename = argv[5];
  char *out_filename = argv[6];
  /* create socket */
  int socketfd = socket(AF_INET,SOCK_DGRAM,0);
  if (socketfd<0){
    fprintf(stderr,"Can't create sock");
    exit(-1);
  }
  /* set time interval */
  struct timeval timout;
  timout.tv_sec = 5;
  timout.tv_usec = 0;
  if(setsockopt(socketfd,SOL_SOCKET,SO_RCVTIMEO,&timout, sizeof(timout))<0){
    fprintf(stderr,"can't set time out\n");
    exit(1);
  }
  int infile_fd = open(in_filename,O_RDWR,0644);
  if (infile_fd==-1){
    fprintf(stderr,"can't open");
    exit(-1);
  }

  struct stat info;
  fstat(infile_fd,&info);
  int file_size = info.st_size;

  /* set IP address */
  struct sockaddr_in server_address;
  struct sockaddr_in rece_address;
  bzero(&server_address,sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port= htons(port_number);
  server_address.sin_addr.s_addr = inet_addr(ip_address);
  socklen_t addr_length = sizeof (server_address);

  /*start sending*/
  List l = genfromfile(mtu,infile_fd,file_size);
  if (length(l)<=0){
    fprintf(stderr,"can't pass empty file\n");
    exit(-1);
  }
  /*Test Connection*/
  struct header connect;
  connect.type = TEST;
  sendto(socketfd,&connect,sizeof(connect),0,(struct sockaddr *)&server_address, addr_length);
  struct header connect_result;
  int rec_size = recvfrom(socketfd,&connect_result,sizeof(connect_result),0,(struct sockaddr *)&rece_address,&addr_length);
  if (rec_size<0){
    fprintf(stderr,"can't detect server\n");
    exit(-1);
  }

  moveFront(l);
  while (length(l)>0){
    window_size = min(length(l),window_size);
    for (int i = 0; i < window_size; ++i) {

      struct header *cur = get(l);

      cur->window_size = window_size;
      cur->base_seq = cur->seq-i;
      if (cur->count>5){
        fprintf(stderr,"%d resend over 5 time\n",cur->seq);
        struct header error;
        error.type=ERROR;
        error.seq = -1;
        memcpy(error.filename,out_filename, 64);
        error.window_size = -1;
        sendto(socketfd,&error,sizeof(error),0,(struct sockaddr *)&server_address, addr_length);
        exit(-1);
      }
      if (cur->count>0){
        fprintf(stderr,"Detect package lost\n");
      }
      cur->count++;

      memcpy(cur->filename,out_filename, 64);
      sendto(socketfd,cur,sizeof(*cur),0,(struct sockaddr *)&server_address, addr_length);
      struct timeb timebuf;
      ftime( &timebuf );
      time_t t;
      time(&t);
      time_t timep;
      struct tm *p;
      time(&timep);
      p = gmtime(&timep);
      printf( "%04d-%02d-%02dT%02d:%02d:%02d.%huZ,",(1900+p->tm_year), (1+p->tm_mon), p->tm_mday,p->tm_hour, p->tm_min,p->tm_sec,timebuf.millitm );
      printf("DATA,%d,%d,%d,%d\n",cur->seq,cur->base_seq,cur->seq+1,cur->base_seq+window_size);
      if (length(l)>0) {
        moveNext(l);
      }

    }
    struct header rece_header;
    rece_header.seq=-1;
    long receive_size = recvfrom(socketfd,&rece_header,sizeof(rece_header),0,(struct sockaddr *)&rece_address,&addr_length);
    if (receive_size<0){
      fprintf(stderr,"time out\n");
      rece_header.seq = -1;
    //TODO exit after 5 drop
    }
    struct timeb timebuf;
    ftime( &timebuf );
    time_t t;
    time(&t);
    time_t timep;
    struct tm *p;
    time(&timep);
    p = gmtime(&timep);
    printf( "%04d-%02d-%02dT%02d:%02d:%02d.%huZ,",(1900+p->tm_year), (1+p->tm_mon), p->tm_mday,p->tm_hour, p->tm_min,p->tm_sec,timebuf.millitm );
    printf("ACK,%d,%d,%d,%d\n",rece_header.seq,rece_header.base_seq,rece_header.seq+1,rece_header.base_seq+window_size);
    while (length(l)>0) {
      moveFront(l);
      struct header *cur = get(l);

      if (cur->seq <= rece_header.seq) {
        delete (l);
      } else{
        break;
      }
    }
  }
  struct header fin = {0,-1,FIN,"",-1,"",0,0};
  memcpy(fin.filename,out_filename, 64);
  sendto(socketfd,&fin,sizeof(fin),0,(struct sockaddr *)&server_address, addr_length);
  close(socketfd);

}

