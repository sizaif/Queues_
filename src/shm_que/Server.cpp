/*** 
 * @
 * @　　┏┓　　　┏┓+ +
 * @　┏┛┻━━━┛┻┓ + +
 * @　┃　　　　　　　┃ 　
 * @　┃　　　━　　　┃ ++ + + +
 * @ ████━████ ┃+
 * @　┃　　　　　　　┃ +
 * @　┃　　　┻　　　┃
 * @　┃　　　　　　　┃ + +
 * @　┗━┓　　　┏━┛
 * @　　　┃　　　┃　　　　　　　　　　　
 * @　　　┃　　　┃ + + + +
 * @　　　┃　　　┃
 * @　　　┃　　　┃ +  神兽保佑
 * @　　　┃　　　┃    代码无bug　　
 * @　　　┃　　　┃　　+　　　　　　　　　
 * @　　　┃　 　　┗━━━┓ + +
 * @　　　┃ 　　　　　　　┣┓
 * @　　　┃ 　　　　　　　┏┛
 * @　　　┗┓┓┏━┳┓┏┛ + + + +
 * @　　　　┃┫┫　┃┫┫
 * @　　　　┗┻┛　┗┻┛+ + + +
 * @
 * @
 * @Author: sizaif
 * @Date: 2023-08-31 17:20:58
 * @LastEditTime: 2023-08-31 17:36:48
 * @Description: 
 * @
 */

#include <iostream>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <list>
#include <string>
#include <string.h>

#include "shmmqueue.h"
#include "../sdmessagebuf/sdmessagebuf.pb.h"


using namespace shmmqueue;

#define SHAR_KEY_1 100010
#define SHAR_KEY_2 100020

using namespace std;

std::atomic_int read_count;

std::atomic_int write_count;

int read_i = 0;

int write_i = 0;

atomic_bool done_flag;

// #define SING_TEST_NUM 1000000
#define SING_TEST_NUM 10
#define  THREAD_NUM 5
#define THREAD_SEND_NUM 100000

typedef unsigned long long  u64;

void PrintMSGData(const MSGData& msg) {
    printf("__________________________start______________________________\n");
    std::cout << "sock_count: " << msg.sock_count() << std::endl;
    std::cout << "end_: " << msg.end_() << std::endl;
    std::cout << "io_msg: " << msg.io_msg() << std::endl;
    std::cout << "inst_type: " << msg.inst_type() << std::endl;
    std::cout << "extern_type: " << msg.extern_type() << std::endl;
    std::cout << "seq_: " << msg.seq_() << std::endl;
    std::cout << "vu1size: " << msg.vu1size() << std::endl;
    std::cout << "vu2size: " << msg.vu2size() << std::endl;
    std::cout << "vu3size: " << msg.vu3size() << std::endl;
    std::cout << "write_context: " << msg.write_context() << std::endl;
    std::cout << "vfile: " << msg.vfile() << std::endl;
    std::cout << "func_name: " << msg.func_name() << std::endl;

    // Print the repeated fields
    
    for (int i = 0; i < msg.vu1size(); ++i) {
        std::cout << "vu1[" << i << "]: " << msg.vu1(i) << std::endl;
    }

    for (int i = 0; i < msg.vu2size(); ++i) {
        std::cout << "vu2[" << i << "]: " << msg.vu2(i) << std::endl;
    }

    for (int i = 0; i < msg.vu3size(); ++i) {
        std::cout << "vu3[" << i << "]: " << msg.vu3(i) << std::endl;
    }
    printf("_________________________end_______________________________\n");
}

void InitMSGData(MSGData &msg_data){
    msg_data.Clear();
    msg_data.set_sock_count(0);
    msg_data.set_end_(0);
    msg_data.set_io_msg(0);
    msg_data.set_inst_type(12345);
    msg_data.set_extern_type(0);
    msg_data.set_seq_(0);
    msg_data.set_vu1size(0);
    msg_data.set_vu2size(0);
    msg_data.set_vu3size(0);
    msg_data.set_write_context("");
    msg_data.set_vfile("");
    msg_data.set_func_name("");
}

void write_func(CMessageQueue *WriteQueue){
    MSGData msgdata;

    int i = 0;
    while(1){
        i++;
        if(i > 11){ break;}

        InitMSGData(msgdata);
        msgdata.set_seq_(i);
        std::string serialed_ = msgdata.SerializeAsString();
        int iRet = WriteQueue->SendMessage((BYTE *) serialed_.c_str(), serialed_.length());
        if (iRet == 0) {
            // succ
            printf("[Send]/%s/%d , msg_data.size: %d \n",__FUNCTION__,__LINE__,serialed_.length());
        }
        else {
            if (iRet != (int) eQueueErrorCode::QUEUE_NO_SPACE) {
                printf("Write failed data = %d,ret = %d\n", write_i, iRet);
                WriteQueue->PrintTrunk();
                exit(-1);
            }
        }
    }


}
int main(int argc, const char *argv[])
{
    MSGData msgdata;
    std::string wri_str = "WRITE";
    CMessageQueue *WriteQueue =  CMessageQueue::CreateInstance(wri_str,SHAR_KEY_2, 10240,eQueueModel::ONE_READ_ONE_WRITE);
    write_func(WriteQueue);
    delete  WriteQueue;
    WriteQueue = nullptr;

}