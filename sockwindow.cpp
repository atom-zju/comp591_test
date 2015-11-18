#include <iostream>
#include <cstdlib>
#include "sockwindow.h"
#include "global.h"
#include <stdio.h>
#include <string>
#include <zlib.h>

SockWindow::SockWindow():seq_num(),offset(0),win_stat(empty)
{
    //malloc memory to data pointer
    pkt_p=(char*)malloc(PKT_SIZE);
    if(!pkt_p){
        std::cout<<"Err: cannot malloc memeory to one of the window."<<std::endl;
        exit(-1);
    }
}

SockWindow::~SockWindow()
{
    //free the data pointer
    free(pkt_p);
}

void SockWindow::setTimestamp(struct timeval tv)
{
    timestamp.tv_sec=tv.tv_sec;
    timestamp.tv_usec=tv.tv_usec;
}

WinStat SockWindow::getStat()
{
    return win_stat;
}

void SockWindow::setStat(WinStat w_stat)
{
    win_stat=w_stat;
}

char* SockWindow::getDataPtr()
{
    return pkt_p;
}

void SockWindow::setContentSize(unsigned int size)
{
    content_size=size;
}

void SockWindow::setSeqNum(unsigned int seq)
{
    seq_num=seq;
}

int SockWindow::getCotentSize()
{
    return content_size;
}

unsigned int SockWindow::getSeqNum()
{
    return seq_num;
}

struct timeval SockWindow::getTimestamp()
{
    return timestamp;
}

void SockWindow::setWinType(WinType win_t)
{
    win_type=win_t;
}

WinType SockWindow::getWinType()
{
    return win_type;
}

void SockWindow::addHeader()
{
    //fill pkt type
    if(win_type==normal)
        *((char *)pkt_p) = 0;
    else if(win_type==begin)
        *((char *)pkt_p) = 1;
    else
        *((char *)pkt_p) = 2;

    //fill content len
    *((unsigned short *)pkt_p+1) = (unsigned short) content_size;

    //fill offset
    *((unsigned int *)pkt_p+1) = (unsigned int) offset;

    //fill seq
    *((unsigned int *)pkt_p+2) = (unsigned int) seq_num;

    //fill in checksum
    uLong crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, (const Bytef*)pkt_p ,HEADER_LEN-sizeof(uLong));
    crc = crc32(crc, (const Bytef*)(pkt_p+HEADER_LEN),content_size);
    *((uLong*)pkt_p+2) = crc;

}

void SockWindow::swapDataPtr(char*& swap_buf)
{
    char* tmp=swap_buf;
    swap_buf=pkt_p;
    pkt_p=tmp;
}

void SockWindow::setOffset(unsigned int offs)
{
    offset=offs;
}

unsigned int SockWindow::getOffest()
{
    return offset;
}

void SockWindow::updateSeqNum()
{
    seq_num+=WIN_NUM;
}

void SockWindow::display()
{
    std::cout<<"=================================="<<"Size:"<<content_size<<std::endl;
    std::cout<<"Window Content:\n"<<"=================================="<<std::endl;
    printf("%.*s",content_size,pkt_p+HEADER_LEN);
}
