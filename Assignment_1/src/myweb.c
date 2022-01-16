#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

long find_content_length(char *buffer) {
    char* chunk = "Transfer-Encoding: chunked";
    char* chunk_result = strstr(buffer,chunk);

    if (chunk_result!=NULL){
        fprintf(stderr,"has chunk");
        exit( -1);
    }
    char *end;
    char code[3] = {buffer[9],buffer[10],buffer[11]};
    int status = strtol(code, &end, 10);
    if (status!=200){
        fprintf(stderr,"Error code %d\n",status);
        for (int i = 0; ; ++i) {
            fprintf(stderr,"%c",buffer[i]);
            if (buffer[i]=='\n'){
                break;
            }
        }
        exit(-1);
    }
    char *substring = "Content-Length: ";
    char *result = strstr(buffer, substring);
    if (result == NULL) {
        return -1;
    }
    char *stop;
    char ret[1024] = {0};
    int count = 0;
    for (uint i = strlen(substring); i < strlen(result); ++i) {
        ret[count] = result[i];
        count++;
    }
    return strtol(ret, &stop, 10);
}

void get_content(int sockfd, long size, FILE *file) {
    char result;
    if (size == -1) {
        while (read(sockfd, &result, 1)) {

            fprintf(file, "%c", result);
            if (result == EOF) {
                break;
            }
        }
        return;
    }
    for (long i = 0; i < size; i++) {
        read(sockfd, &result, 1);

        fprintf(file, "%c", result);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        printf("Incorrect Parameter need to \"./bin/myweb <url> <ipaddress>:<port>/<path>\"");
        return -1;
    }
    char *url = argv[1];
    char *host_name = argv[2];
    int body = 1;
    if (argc == 4) {
        if (strcmp(argv[3], "-h") == 0) {
            body = 0;
        }
    }

    char splite[] = "/";
    char *ip_host = strtok(host_name, splite);
    char *path = strtok(NULL, ":");
    if (path==NULL){
        path="/";
    }
//    fprintf(stderr,"url:%s\n", url);
//    fprintf(stderr,"ip+host:%s\n", ip_host);
//    fprintf(stderr,"path:%s\n", path);
    char *ip_address = strtok(ip_host, ":");
//    fprintf(stderr,"ip address:%s", ip_address);
    long port_number = 80;
    char *port_str = strtok(NULL, ":");
    char *stop;
    if (port_str != NULL) {
        port_number = strtol(port_str, &stop, 10);
    }
//    printf("url:%s\n", url);
//    printf("ip+host:%s\n", ip_host);
//    printf("path:%s\n", path);
//    printf("ip address:%s\n", ip_address);
//    printf("port:%lu\n", port_number);

    char result;
    char buffer[1024];
    char httprequest[1024];
    sprintf(httprequest, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", path, url);
    fprintf(stdout,"request %s\n", httprequest);
    char *file_name = "./output.dat";

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Create Sock ERROR");
        return -1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_number);
    serv_addr.sin_addr.s_addr = inet_addr(ip_address);
    if (serv_addr.sin_addr.s_addr==INADDR_NONE){
        fprintf(stderr,"ip FORMAT Incorrect\n");
        exit(1);
    }
    int connection_result = connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (connection_result == -1) {
        fprintf(stderr,"Connection Error");
        return -1;
    }

    write(sock, httprequest, strlen(httprequest));
    int nextline = 0;
    for (int i = 0; read(sock, &result, 1)!=0; i++) {

        buffer[i] = result;
        if (nextline == 1 && result == '\r') {
            break;
        }
        if (result == '\n') {
            nextline = 1;
        } else {
            nextline = 0;
        }
        printf("%c", result);
    }
    read(sock, &result, 1);
    if (body) {
        FILE *output = fopen(file_name, "w+");
        long size = find_content_length(buffer);
        //printf("%lu", size);
        get_content(sock, size, output);
        fclose(output);


    }
    close(sock);
    return 0;
}
