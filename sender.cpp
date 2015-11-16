#include <iostream>
#include <cstdlib>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "sockwall.h"

using namespace std;

int main(int argc, char** argv)
{
    int sock;                 //socket descriptor
    struct sockaddr_in sin;
    socklen_t addrlen = sizeof(sin); /* length of addresses */
    fd_set read_set, write_set;
    struct timeval time_out;
    int select_retval;
    char* send_pkt;
    char read_buf[PKT_SIZE];

    //check input arguments
    if(argc!=5){
        cout<<"Usage: sendfile -r <recvhost>:<recvport> -f <filename>"<<endl;
        exit(0);
    }
    //parse input arguments
    string file_path,ip_str;
    short port;
    for(int i=0;i<argc;i++){
        //if this is not the last argument
        if(i+1!=argc){
            if(!strcmp(argv[i],"-r")){
                //next arg will be <recvhost>:<recvport>
                //cout<<"assign val for ip info"<<endl;
                string ip_addr_port(argv[i+1]);
                size_t sep=ip_addr_port.find_first_of(":");
                if(string::npos==sep){
                    //':' not found in argument
                    cout<<"Usage: sendfile -r <recvhost>:<recvport> -f <filename>"<<endl;
                    exit(0);
                }
               ip_str.assign(ip_addr_port.substr(0,sep));
               port = atoi(ip_addr_port.substr(sep+1).c_str());
               //port_str.assign(ip_addr_port.substr(sep+1));

            }
            else if(!strcmp(argv[i],"-f")){
                //next arg will be <filename>
                file_path.assign(argv[i+1]);
            }
        }
    }
    //check if critical arguments empty
    if(file_path.empty()||ip_str.empty()){
        //do not provide some of these contents, exit
        cout<<"Usage: sendfile -r <recvhost>:<recvport> -f <filename>"<<endl;
        exit(0);
    }

    // fill in client's address
    memset (&sin, 0, sizeof (sin));
    sin.sin_family = AF_INET;

    //convert ip address
    if(!inet_aton(ip_str.c_str(), &sin.sin_addr)){
        //if ip address is invalid
        cout<<"ip_address is invalid."<<endl;
        exit(-1);
    }
    sin.sin_port = htons(port);

    //create socket
    if((sock = socket(AF_INET,SOCK_DGRAM,0))<0){
        //fail to open socket
        cout<<"Err: fail to open socket."<<endl;
        exit(-1);
    }



    //create sockwall
    SockWall sock_wall(file_path.c_str(),sender);


    while(!sock_wall.isFinished()){

        FD_ZERO (&read_set); /* clear everything */
        FD_ZERO (&write_set); /* clear everything */

        //put sock into read set
        FD_SET (sock, &read_set);

        //load date to windows, return # of pending window
        int pending_win=sock_wall.loadWin();
        if(pending_win>0){
            //if there are windows pending to be sent, put sock into write set
            FD_SET (sock, &write_set);
        }

        //define timeout for select func
        time_out.tv_usec = 100000; /* 1-tenth of a second timeout */
        time_out.tv_sec = 0;

        //select
        select_retval = select(sock+1, &read_set, &write_set, NULL, &time_out);

        if(select_retval<0){
            cout<<"Err: select fail"<<endl;
            exit(-1);
        }

        if(select_retval==0){
            //nothing to do, continue
            continue;
        }

        //if sock has something to read
        if(FD_ISSET(sock, &read_set)){
            cout<<"start to recvfrom()"<<endl;
            int recv_size = recvfrom(sock,read_buf,PKT_SIZE,0,(struct sockaddr *)&sin,&addrlen);
            sock_wall.handlePkt(read_buf,recv_size);
        }

        //if sock has something to write
        if(FD_ISSET(sock, &write_set)){
            for(int i=0;i<pending_win;i++){
                int send_size;
                int win_idx = sock_wall.makePkt(send_pkt,send_size);
                cout<<"start to sendto()"<<endl;
                sendto(sock,send_pkt,send_size,0,(struct sockaddr *)&sin,addrlen);
                sock_wall.setStatSent(win_idx);
            }
        }

    }

    return 0;
}
