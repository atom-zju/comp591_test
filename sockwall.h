#ifndef SOCKWALL_H
#define SOCKWALL_H
#include "sockwindow.h"
#include "global.h"
#include <fstream>

enum WallStat {working,ending};

class SockWall
{
public:
    SockWall(const char *file_path,WallType wt);
    ~SockWall();
    int loadWin();                          //load content from target file to window
    //int checkIfHasDataToSend();             //check if has data to send, return # of window to send, -1 if noting to send
    int makePkt(char *&pkt,int& size);      //make packet, return idx of window to be sent and pkt size
    void setStatSent(int idx);              //change the stat of window of index 'idx' to 'sent'
    int handlePkt(char* pkt,int size);      //handle received packet, return -1 if fail to parse correctely
    bool isFinished();                      //return true if last pkt got acked, everything done

    char* handlePkt_recv(int recv_size,int& return_size);                 //(only receiver)handle packet in the received side, return the pkt pointer to be sent
    char* getRecvBuf();                     //(only receiver) get receive buffer for receiver
    unsigned int extractSeq();              //(only receiver) extract seq number from swap_buf

private:

    int loadFileName(const char *file_name);      //load filename into the first window
    int min_seq_idx;                //current window position
    std::fstream file;             //target file to be read/write
    unsigned int file_pos;          //record the current position of the file
    unsigned int seq_num;           //seq number, increase when loading one pkt
    unsigned int file_length;       //length of target file
    SockWindow window[WIN_NUM];     //sockwindow objects
    WallStat wall_stat;             //wall stat can be working or ending
    bool finished;                  //indicate whether have finished all task
    WallType wall_type;             //wall type can be either sender or receiver

    char* make_ACK_Pkt(int& return_size);                   //create ack message
    bool checkValid();                      //(only receiver) check the validility of the pkt in swap_buf
    void deliverPkt2Win(int winIdx);        //(only receiver)deliver pkt to corresponding window by swaping the data pointer
    int findWinBySeq();                     //(only receiver)find the index of window using seq extra
    bool tryWriteFile();                    //try to write file from the window array
    char* swap_buf;                 //(only receiver) swap_buf is used to receive pkt and hand over pkt to be sent to user
    int contentLen;
};

#endif // SOCKWALL_H
