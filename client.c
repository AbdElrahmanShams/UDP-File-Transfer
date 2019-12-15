//
// Created by ab_shams on 11/12/19.
//

#include<stdio.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<arpa/inet.h>
#include <time.h>


struct packet {
/* Header */
    uint16_t len;
    uint32_t seqno;
/* Data */
    char data[1024]; /* Not always 1024 bytes, can be less */
};


struct ack_packet {
    uint16_t len;
    uint32_t ackno;
};

char *itoa(long i,char *s,int dummy_radix)
{
    sprintf(s,"%ld",i);
    return s;
}

int main(int argc, int *argv[])
{
    int sock;
    ssize_t n;
    struct sockaddr_in server,from;
    struct hostent *hp;
    long int packets =0;
    unsigned char buff[1024] = {0};

    clock_t tic,toc;


    // checking if hostname and the port address is provided //
    if(argc!=3)
    {
        printf("insufficient arguments\n");
        exit(1);
    }

    //create a socket//
    sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

    if(sock<0)
    {
        printf("error in opening socket\n");
        return 1;
    }

    //to get  the hostname of the system or the machine//
    hp= gethostbyname(argv[1]);

    if(hp==0)
    {
        printf("Unknown host\n");
        return 1;
    }
    //build the server's IP address //
    bzero((char *)&server,sizeof(server));
    bcopy((char*)hp->h_addr,(char *)&server.sin_addr,hp->h_length);
    server.sin_family = AF_INET;
    server.sin_port =  htons(atoi(argv[2]));
//    length = sizeof(server);

    /*open the file that we wish to transfer*/
    FILE *fp = fopen("image.jpg","rb");
    if(fp==NULL)
    {
        printf("file open error\n");
        return 1;
    }
    fseek(fp,0,SEEK_END); //if exists read the size of the file
    size_t file_size = ftell(fp);
    fseek(fp,0,SEEK_SET);

    printf("size of the file is %ld\n", file_size);

    /*find the number of packets*/

    packets = (file_size/1024)+1 ;

    /*send the number of packets to the server*/

    itoa(packets,buff,10);

//    tic = clock();
    n= sendto(sock,buff,strlen(buff),0,(struct sockaddr *)&server,sizeof(server));
    if(n<0)
    {
        printf("error in sending message to the serveR");
        return 1;
    }
    int fromSize  = sizeof(from);
    char buf[1024];

//    while(1){
        bzero(buf,1024);
        if (recvfrom(sock,buf,sizeof (buf),0,(struct sockaddr *)&from,&fromSize) < 0 ){
            printf("error receiving ACK from the serveR");
            return 1;
        }
//        toc = clock();
//        if( ((double) (toc - tic) /CLOCKS_PER_SEC) >= 1){
//            printf("time out\n");
//            break;
//        }
//    }

    printf("Server : %s\n",buf);




    /*Read data from file and send it*/
    int packetNum = 0;
    while(1)
    {
        /*First read file in chunks of  1024  bytes */

        size_t nread = fread(buff,1,1024,fp);
        //printf("Bytes read %d\n",nread);

        /*if read was success ,send data*/
        if(nread>0)
        {
//            printf("data sent now is %s\n",buff);
            n= sendto(sock,buff,1024,0,(struct sockaddr *)&server,sizeof(server));
            printf("Sending %d, numBytes sent: %ld\n", packetNum, n);
            packetNum++;
            if(n<0)
            {
                printf("error in sending message to the server");
                fclose(fp);
                return 1;
            }

            bzero(buf,1024);
            if (recvfrom(sock,buf,sizeof (buf),0,(struct sockaddr *)&from,&fromSize) < 0 ){
                printf("error receiving ACK from the serveR");
                return 1;
            }

            printf("Server :  %s\n",buf);

        }

        /*There is something tricky going on with the read..
         * Either there was error ,or we reached end of  file.
         */
        if(nread<1024)
        {
            if(feof(fp))
                printf("End of file\n");

            if(ferror(fp))
                printf("Error reading\n");
            break;
        }

    }
    close(sock);
    fclose(fp);

    return 0;
}