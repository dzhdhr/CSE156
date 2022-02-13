//
// Created by 董子豪 on 1/30/22.
//
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include "list.h"
#define ACK 1
#define DATA 2
#define ERROR 3
#define FIN 4
#define TEST 5

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

bool drop(int percent){

  //TODO finish drop package
  int random = rand()%100;
  if (random<percent){
//    printf("drop package!\n");
    return true;
  }
  return false;
}

void save_file(List pObj,int seq) {
  moveFront(pObj);
  struct header *first = get(pObj);
  char* filename = first->filename;
  int output = open(filename,O_CREAT|O_WRONLY|O_APPEND,S_IREAD|S_IWRITE);
  if (output==-1){
    fprintf(stderr,"can't open output file");
    exit(-1);
  }
  if (seq<0){
    fprintf(stderr,"save all\n");
    while (length(pObj)>0){
      moveFront(pObj);
      struct header *temp = get(pObj);

      //    printf("DATA SEQ: %d PACK_SIZE:%d WIN_SIZE %d payload :%s out_file: %s\n",temp->seq,temp->size,temp->window_size, temp->payload,temp->filename);
      write(output,temp->payload,temp->size);
      delete (pObj);
    }
  }
  else{
    while (length(pObj)>0) {
      moveFront(pObj);
      struct header *cur = get(pObj);
      if (cur->seq < seq) {
        write(output,cur->payload,cur->size);
        //printf("WRITE SEQ: %d PACK_SIZE:%d WIN_SIZE %d payload :%s out_file: %s\n",cur->seq,cur->size,cur->window_size, cur->payload,cur->filename);
        delete (pObj);
      } else{
        close(output);
        break;
      }
    }
  }

  close(output);
}

int main(int argc, char *argv[]){
  if (argc!=3){
    fprintf(stderr,"usage ./myserver <port number> <drop percent>\n");
    exit(-1);
  }

  int port_number = atol(argv[1]);
  int percent_drop = atol(argv[2]);
  if (port_number<1024||port_number>65535){
    fprintf(stderr, "port need to be between 1024 and 65535\n");
    exit(-1);
  }
  int timout_count = 0;
  if (percent_drop<0||percent_drop>100){
    fprintf(stderr, "percentage need to be between 0 and 100\n");
    exit(-1);
  }

  int socketfd = socket(AF_INET,SOCK_DGRAM,0);
  if(socketfd<0){
    fprintf(stderr,"Can't create sock");
    exit(-1);
  }
  struct timeval timout;
  timout.tv_sec = 3;
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
  socklen_t addr_length = sizeof (address);
  List l = newList();
  int seq = 0;
  int prev_seq = -1;
  while (1) {
    int windowsize = 5;
    struct sockaddr_in send;
    struct header h;
    int i;
    for (i = 0; i < windowsize; ) {
      int result = recvfrom(socketfd, &h, sizeof(h), 0, (struct sockaddr *)&send, &addr_length);//接受
      if (result<=0){
        timout_count++;
        if (timout_count>10){
          while (length(l)>0){
            moveFront(l);
            delete (l);
          }
          prev_seq = -1;
          timout_count = 0;
        }
        //fprintf(stderr,"nothing received\n");
        continue;
      }
      timout_count = 0;
      if (h.type==TEST){
        struct header temp = {TEST,-1,0,"",0,"",0,0};
        sendto(socketfd,&temp,sizeof(temp),0,(struct sockaddr *)&send, addr_length);
        continue;
      }
      if (length(l)>0){
        struct header *f = front(l);
        if (strcmp(f->filename,h.filename)){
          fprintf(stderr,"%s %s not same address\n",h.filename,f->filename);
          continue;
        }
      }
      if (h.type ==ERROR||timout_count>5){
        fprintf(stderr,"client side error\n");
        while (length(l)>0){
          moveFront(l);
          delete (l);
        }
        prev_seq = -1;
        continue;
      }
      timout_count = 0;



      if (h.type==FIN){
        fprintf(stderr,"**************recive finish storing file*******************\n");
        save_file(l,-1);
        prev_seq = -1;
        break;
      }
      windowsize = h.window_size;
      if (drop(percent_drop)){
        i++;
        struct timeb timebuf;
        ftime( &timebuf );
        time_t t;
        time(&t);
        time_t timep;
        struct tm *p;
        time(&timep);
        p = gmtime(&timep);
        printf( "%04d-%02d-%02dT%02d:%02d:%02d.%huZ,",(1900+p->tm_year), (1+p->tm_mon), p->tm_mday,p->tm_hour, p->tm_min,p->tm_sec,timebuf.millitm );
        printf("DATA,%d\n",h.seq);
        continue;
      }

      struct header *temp = malloc(sizeof(struct header));
      temp->seq = h.seq;
      temp->window_size = h.window_size;
      temp->type = h.window_size;
      memcpy(temp->payload,h.payload,h.size);
      memcpy(temp->filename,h.filename,64);
      temp->size = h.size;
      if (temp->seq>prev_seq) {
        insertByOrder(l, temp);
      }
        ++i;
    }

    struct header ret;

    seq = next_seq(l,h.base_seq);
    ret.seq = seq;
    ret.base_seq = h.base_seq;
    ret.type=ACK;
//    printf("list length %d expacted %d cur seq: %d\n", length(l),h.seq,seq);
    if (!drop(percent_drop)) {
      sendto(socketfd, &ret, sizeof(ret), 0, (struct sockaddr *)&send,
             addr_length);
    }
    else {
      struct timeb timebuf;
      ftime( &timebuf );
      time_t t;
      time(&t);
      time_t timep;
      struct tm *p;
      time(&timep);
      p = gmtime(&timep);
      printf( "%04d-%02d-%02dT%02d:%02d:%02d.%huZ,",(1900+p->tm_year), (1+p->tm_mon), p->tm_mday,p->tm_hour, p->tm_min,p->tm_sec,timebuf.millitm );
      printf("ACK,%d\n", ret.seq);
    }
    if (seq>0) {
      save_file(l, seq);
      prev_seq = seq;
    }
  }
}

