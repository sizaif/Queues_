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
 * @LastEditTime: 2023-08-31 17:35:56
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


void read_func(CMessageQueue *ReadQueue)
{
    MSGData recv_msg;
    while (1) {
        BYTE data[1000] = {0};
        int len = ReadQueue->GetMessage(data);
        if (len > 0) {
            bool parse_suc = recv_msg.ParseFromArray(data,len);
            if(parse_suc){
                printf("RECV DATA IS : \n");    
                PrintMSGData(recv_msg);
            }else{
                printf("解析失败\n");
            }
            read_i++;
            if(recv_msg.end_() == 1){
                break;
            }
        }
        else {
            if (len != (int) eQueueErrorCode::QUEUE_NO_MESSAGE) {
                printf("Read failed ret = %d\n", len);
                ReadQueue->PrintTrunk();
                exit(-1);
            }
        }
    }
}
int main(int argc, const char *argv[])
{
    std::string re_str = "READ";
    CMessageQueue *messQueue2 = CMessageQueue::CreateInstance(re_str,SHAR_KEY_2, 10240,eQueueModel::ONE_READ_ONE_WRITE);
    read_func(messQueue2);
    delete  messQueue2;
    messQueue2 = nullptr;

}