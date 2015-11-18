#include <iostream>
#include <cstdlib>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "sockwall.h"
#include <stdio.h>

using namespace std;


int main(int argc, char** argv)
{
    int sock;                 //socket descriptor
    struct sockaddr_in sin;
    struct sockaddr_in remaddr;
    socklen_t addrlen = sizeof(remaddr); /* length of addresses */
    char* pkt_handle;
    //char read_buf[PKT_SIZE];

    //check input arguments
    if(argc!=3){
        cout<<"Usage: recvfile -p <recv_port>"<<endl;
        exit(0);
    }
    //parse input arguments
    int port_str;
    for(int i=0;i<argc;i++){
        //if this is not the last argument
        if(i+1!=argc){
            if(!strcmp(argv[i],"-p")){
                //next arg will be <recv_port>
                port_str = atoi(argv[i+1]);
            }
        }
    }

    // fill in client's address
    bzero ((char *)&sin,sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons((unsigned short)port_str);

    cout<<"port: "<<port_str<<endl;

    //create socket
    if((sock = socket(AF_INET,SOCK_DGRAM,0))<0){
        //fail to open socket
        cout<<"Err: fail to open socket."<<endl;
        exit(-1);
    }

    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    //bind socket
    if(bind( sock, (struct sockaddr *)&sin, sizeof(sin))<0){
        //fail to bind socket
        cout<<"Err: fail to bind socket."<<endl;
        exit(-1);
    }

    SockWall sock_wall(0,receiver);
    while(!sock_wall.isFinished()){
        pkt_handle = sock_wall.getRecvBuf();
        int recv_size,return_size;

        cout<<"start to recvfrom()"<<endl;
        recv_size = recvfrom(sock,pkt_handle,PKT_SIZE,0,(struct sockaddr *)&remaddr,&addrlen);
//        std::cout<<"==================================\n"<<"Recv Size:"<<recv_size<<std::endl;
//        std::cout<<"Recv Content:\n"<<"=================================="<<std::endl;
//        printf("%.*s",recv_size-HEADER_LEN,pkt_handle+HEADER_LEN);
	
        pkt_handle = sock_wall.handlePkt_recv(recv_size,return_size);
	if(pkt_handle == NULL)
	  continue;
        cout<<"start to sendto()"<<endl;
        sendto(sock,pkt_handle,return_size,0,(struct sockaddr *)&remaddr,addrlen);
    }
    return 0;
}
