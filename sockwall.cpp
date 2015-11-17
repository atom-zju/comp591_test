#include "sockwall.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <string.h>
#include <stdio.h>


SockWall::SockWall(const char* file_path, WallType wt):min_seq_idx(0),file_pos(0), wall_stat(working),finished(false),wall_type(wt),seq_num(0)
{
    //set seq num for each window
    for(int i=0;i<WIN_NUM;i++){
        window[i].setSeqNum(i);
    }
    if(wt==sender)
    {
        //open the file
        file.open(file_path,std::ifstream::in);
        if(!file.is_open()){
            //if fail to open the file, exit
            std::cout<<"Err:cannot open the input file"<<std::endl;
            exit(-1);
        }

        //get file length
        file.seekg(0,std::ios::end);
        file_length=file.tellg();
        file.seekg(0,std::ios::beg);


        //process and store file name into the first window
        std::string file_name(file_path);
        std::size_t found = file_name.find_last_of("/");
        if(std::string::npos!=found){
            //contain directory sign, extract name
            file_name=file_name.substr(found+1);
        }
        //if store file name into window failed
        if(loadFileName(file_name.c_str())<0){
            std::cout<<"cannot store filename into the first window."<<std::endl;
            exit(-1);
        }
    }
    else{
        // wt==receiver
        swap_buf=(char*)malloc(PKT_SIZE);
        if(!swap_buf){
            //fail to malloc memory
            std::cout<<"Err: fail to malloc memory to swap_buf."<<std::endl;
            exit(-1);
        }
    }
}
SockWall::~SockWall()
{
    //close the file descriptor
    file.close();
}

void SockWall::setStatSent(int idx)
{
    if(idx>=WIN_NUM){
        //idx excess limits
        std::cout<<"window idx execess limits"<<std::endl;
        return;
    }
    //window[idx].display();
    struct timeval tv;
    gettimeofday(&tv,0);
    window[idx].setTimestamp(tv);
    window[idx].setStat(sent);
}

int SockWall::loadWin()
{
    int loaded_cnt=0;
    for(int idx=min_seq_idx,i=0; i<WIN_NUM; i++,idx=(idx+1)%WIN_NUM){
        if(window[idx].getStat()==empty && wall_stat==working){
            //find an empty window, fill it with file content
            std::cout<<"Loading window ["<<window[idx].getSeqNum()<<"]"<<std::endl;
            //getchar();
            file.read(window[idx].getDataPtr()+HEADER_LEN,PKT_SIZE-HEADER_LEN);

	    std::streamsize size= file.gcount();
            std::cout<<"got "<<size <<" characters"<<std::endl;
            //getchar();
            window[idx].setContentSize(size);
            //window[idx].setSeqNum(seq_num);
            //seq_num++;
            window[idx].setOffset(file_pos);
            window[idx].setStat(loaded);
            //file_pos+=size;
	    if(file.eof()){
	      //reach the end of file
	      file_pos=file_length;
	      std::cout<<"Reaching to end of file."<<std::endl;
	      //getchar();
	      window[idx].setWinType(end);
	      wall_stat=ending;
	    }
	    else{
	      file_pos=file.tellg();
	      window[idx].setWinType(normal);
	    }

	    std::cout << "file_pos:\t"<<file_pos << "\n";
	    std::cout << "file_length:\t"<<file_length << "\n";
            window[idx].addHeader();
            loaded_cnt++;
        }
        else if(window[idx].getStat()==loaded){
            loaded_cnt++;
        }
        else if(window[idx].getStat()==sent){
            //check if the window is timeout
            struct timeval tv,timestamp=window[idx].getTimestamp();
            gettimeofday(&tv,0);
            unsigned int elapse_time=(tv.tv_sec-timestamp.tv_sec)*1000+(tv.tv_usec-timestamp.tv_usec)/1000;
            if(elapse_time>TIMEOUT)
            {
                //if timeout, reset to loaded status
                window[idx].setStat(loaded);
                loaded_cnt++;
            }
        }
    }
    return loaded_cnt;
}

int SockWall::loadFileName(const char* file_name)
{
    //check if the file name is too long
    if(strlen(file_name)+1>PKT_SIZE-HEADER_LEN){
        //if too long
        return -1;
    }
    //find the first empty window
    int idx,i;
    for(i=0, idx=min_seq_idx; empty!=window[idx].getStat() && i<WIN_NUM; idx=(idx+1)%WIN_NUM,i++);
    if(i==WIN_NUM){
        //did not find any window need to be load
        return 1;
    }
    strcpy(window[idx].getDataPtr()+HEADER_LEN,file_name);
    window[idx].setContentSize(strlen(file_name)+1);
    window[idx].setWinType(begin);
    window[idx].addHeader();
    window[idx].setStat(loaded);
    return 1;
}

int SockWall::makePkt(char*& pkt, int &size)
{
    //find any loaded yet not sent window and send them out
    int idx,i;
    for(idx=min_seq_idx,i=0;loaded!=window[idx].getStat() && i<WIN_NUM;idx=(idx+1)%WIN_NUM,i++);
    if(i==WIN_NUM){
        //if not find any pkt need to be sent
        return -1;
    }
    pkt=window[idx].getDataPtr();
    size=window[idx].getCotentSize()+HEADER_LEN;
    return idx;
}


int SockWall::handlePkt(char *pkt, int size)
{
    if(wall_type==receiver){
        std::cout<<"Err: receiver cannot call handlePkt.(call handlePkt_recv)"<<std::endl;
        return -1;
    }
    if(size<HEADER_LEN){
        //packet too short, fail to parse
        return -1;
    }
    if(*(pkt)==2){
        //if it is an ending pkt
        finished=true;
        return 1;
    }
    //extract the ack number
   unsigned int ack=*((unsigned int*)pkt+3);
   //try to find the corresponding window
   int idx,i,j;
   for(idx=min_seq_idx,i=0;ack!=window[idx].getSeqNum() && i<WIN_NUM;idx=(idx+1)%WIN_NUM,i++);
   if(i==WIN_NUM){
       if(ack!=window[min_seq_idx].getSeqNum()+WIN_NUM || ack<window[min_seq_idx].getSeqNum()){
            //did not find matched seq number, fail to parse
            return -1;
       }
   }
   for(j=0,idx=min_seq_idx;j<i;idx=(idx+1)%WIN_NUM,j++){
        //mark all the acked packet to empty again;
       window[idx].setStat(empty);
       window[idx].updateSeqNum();
   }
   min_seq_idx=idx;
   std::cout<<"acked upto window ["<<window[min_seq_idx].getSeqNum()<<"]"<<std::endl;

   return 1;
}

bool SockWall::isFinished()
{
    return finished;
}

char* SockWall::getRecvBuf()
{
    return swap_buf;
}

bool SockWall::checkValid()
{
    //first check the length
    if(contentLen<HEADER_LEN)
        return false;

    //////////////////////////////////////////// check validility using checksum or md5
    return true;
}

void SockWall::deliverPkt2Win(int winIdx)
{
    window[winIdx].swapDataPtr(swap_buf);
}

unsigned int SockWall::extractSeq()
{
    return *((unsigned int*)swap_buf+2);
}

int SockWall::findWinBySeq()
{
    unsigned int seq=extractSeq();
    int i,idx;
    for(idx=min_seq_idx,i=0;seq!=window[idx].getSeqNum() && i<WIN_NUM;idx=(idx+1)%WIN_NUM,i++);
    if(i==WIN_NUM){
        //did not find the corresponding window
        return -1;
    }
    return idx;
}

bool SockWall::tryWriteFile()
{
    //std::cout<<"start to try write file()"<<std::endl;
    bool changed=false;
    int i,idx,j;
    for(idx=min_seq_idx,i=0;window[idx].getStat()==loaded && i<WIN_NUM;idx=(idx+1)%WIN_NUM,i++){
    //std::cout<<"# of window to be write:"<<i<<std::endl;
    //getchar();
    //for(j=0,idx=min_seq_idx;j<i;idx=(idx+1)%WIN_NUM,j++){
         //write the content to file
        if((unsigned int)file.tellg()+1!=window[idx].getOffest()){
            std::cout<<"Err:offset cannot match, file might incorrect."<<std::endl;
            //getchar();
        }
        //window[idx].display();
        file.write(window[idx].getDataPtr()+HEADER_LEN,window[idx].getCotentSize());
        file.flush();
        if(window[idx].getWinType()==end){
            //if this is the last pkt to be written
            finished=true;
        }
        window[idx].setStat(empty);
        window[idx].updateSeqNum();
        changed=true;
    }
    min_seq_idx=idx;
    std::cout<<"wirted upto window ["<<window[min_seq_idx].getSeqNum()<<"]"<<std::endl;

    return changed;
}

char* SockWall::handlePkt_recv(int recv_size, int &return_size)
{
    if(wall_type==sender){
        //call the wrong function
        std::cout<<"Err: sender cannot call handlePkt_recv."<<std::endl;
        return NULL;
    }

    contentLen=recv_size;
    if(!checkValid()){
        //if pkt is mangled, discard
    }
    //determine pkt type can be name, content, end
    WinType win_t;
    if(*swap_buf==0)
        win_t=normal;
    else if(*swap_buf==1)
        win_t=begin;
    else
        win_t=end;

    switch(win_t){
        case begin:{
        //std::cout<<"pkt type is begin"<<std::endl;
            //if this is the first pkt containing file name, open output file
        std::string file_name(swap_buf+HEADER_LEN);
        file_name.append(".recv");
        file.open(file_name.c_str(),std::fstream::out);
            if(!file.is_open()){
                //if fail to open the file, exit
                std::cout<<"Err:cannot open the output file"<<std::endl;
                exit(-1);
            }
            window[0].setWinType(normal);
            window[0].updateSeqNum();
            min_seq_idx=1;
            tryWriteFile();
            //try to write all the window after 0
            break;
        }
        case end:
        case normal:
            {
        //std::cout<<"pkt type is end & normal"<<std::endl;
            //if this is a normal pkt containing file content
            int idx=findWinBySeq();
            if(idx>=0){
                if(window[idx].getStat()==empty){
                    //if find a corresponding window, deliver it by swapping the pointer
                    std::cout<<"loaded window ["<<window[idx].getSeqNum()<<"]"<<std::endl;
                    //getchar();
                    deliverPkt2Win(idx);
                    if(win_t==end){
                        window[idx].setWinType(end);
                        //std::cout<<"this is a ending window, please hit enter."<<std::endl;
                        //getchar();
                    }
                    else
                        window[idx].setWinType(normal);
                    window[idx].setStat(loaded);
                    window[idx].setContentSize(recv_size-HEADER_LEN);
                    tryWriteFile();
                }
            }
            else{
                //pkt seq num does not fall into window, discard
            }
            break;
        }
    }
    return make_ACK_Pkt(return_size);

}

char* SockWall::make_ACK_Pkt(int &return_size)
{
    if(finished){       //ending pkt
        *((char*)swap_buf) = 2;
    }
    else                //normal pkt
        *((char*)swap_buf) = 0;
    //fill ack number
    *((unsigned int*)swap_buf+3) = window[min_seq_idx].getSeqNum();

    //////////////////////////////////////////////////////////////////// fill in checksum

    return_size = HEADER_LEN;

    return swap_buf;
}
