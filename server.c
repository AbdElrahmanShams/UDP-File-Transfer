//
// Created by ab_shams on 11/12/19.
//


#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<arpa/inet.h>

int main(int argc, char *argv[])
{
    int sock,fromlen;
    ssize_t n;
    size_t length;
    struct sockaddr_in server;
    struct sockaddr_in from;
    char buf[1024];
    char file_buf[1024];
    int packets = 0;
    int received = 0;
    FILE *newfp;
    newfp = fopen("output.jpg","wb");
    if(newfp==NULL)
    {
        printf("error opening the file\n");
        return 1;
    }
    if(argc<2)
    {
        fprintf(stderr, "no port number specified\n");

        exit(0);
    }
    sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(sock<0)
    {
        printf("error in opening socket\n");
        return 1;
    }
    length = sizeof(server);
    bzero(&server, (size_t) length);
    server.sin_family= AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons((uint16_t) atoi(argv[1]));
    if(bind(sock, (struct sockaddr*)&server, (socklen_t) length) < 0)
    {
        printf("cannot bind\n");
        return 1;
    }
    fromlen  =sizeof(from);

    if(recvfrom(sock, buf, strlen(buf), 0, (struct sockaddr *)&from, &fromlen) < 0)
    {
        printf("recvfrom  error\n");
        return 1;
    }

    printf("Client : %s\n",buf);

    if(sendto(sock, "ACK", strlen("ACK"), 0, (struct sockaddr *)&from, fromlen) < 0){
        printf("error sending ACK\n");
        return 0;
    }


    packets = atoi(buf);
    printf("Num packets expected: %d\n", packets);


    while(received<packets)
    {


        //printf ("%d\n", n);

        n = recvfrom(sock,buf,sizeof (buf),0,(struct sockaddr *)&from,&fromlen);

        //printf ("%d\n", n);
        printf("Packet num %d, numBytes received: %ld\n", received, n);

        if(n<0)
        {
            printf("recvfrom  error\n");
            return 1;
        }

        if(sendto(sock, "ACK", strlen("ACK"), 0, (struct sockaddr *)&from, fromlen) < 0){
            printf("error sending ACK\n");
            return 0;
        }
        if((fwrite(buf,1,n,newfp)) < n)
        {
            printf("error in writing to the file\n");
            return 1;
        }
        received++;

    }
    printf("Finished\n");
    fclose(newfp);
    return 0;
}