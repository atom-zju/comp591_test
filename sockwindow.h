#ifndef SOCKWINDOW_H
#define SOCKWINDOW_H
#include <sys/time.h>

//define 4 window status:
    //empty:    ready to be filled with content
    //loaded:   already have content
    //sent:     already sent
    //acked:    already being acknowleged
enum WinStat {empty,loaded,sent,acked};
enum WinType {begin,normal,end};

class SockWindow
{
public:
    SockWindow();
    ~SockWindow();
    void setTimestamp(struct timeval tv);       //set time stamp for sending
    WinStat getStat();                          //get window status
    void setStat(WinStat w_stat);               //set window status
    char* getDataPtr();                         //get data pointer
    void swapDataPtr(char *&swap_buf);          //(only receiver) swap data pointer
    void setContentSize(unsigned int size);     //set content size
    void setSeqNum(unsigned int seq);           //set seq number
    void addHeader();                           //write header to the data pointer
    int getCotentSize();                        //get content size
    unsigned int getSeqNum();                   //get sequence number
    struct timeval getTimestamp();              //get timestamp
    void setWinType(WinType win_t);             //set window type
    WinType getWinType();                       //get window type
    void setOffset(unsigned int offs);          //set offset
    unsigned int getOffest();                   //get offset
    void updateSeqNum();                        //add seq num by WIN_NUM
    void display();
private:
    char* pkt_p;                                //data pointer
    unsigned int seq_num;                       //sequence number
    unsigned int content_size;                  //content size
    unsigned int offset;
    struct timeval timestamp;                   //time stamp for sending
    WinStat win_stat;                           //window status
    WinType win_type;

};

#endif // SOCKWINDOW_H
