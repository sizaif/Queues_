/*
 * 
 *    ┏┓　　　┏┓
 *  ┏┛┻━━━┛┻┓
 *  ┃　　　　　　　┃
 *  ┃　　　━　　　┃
 *  ┃　＞　　　＜　┃
 *  ┃　　　　　　　┃
 *  ┃...　⌒　...　┃
 *  ┃　　　　　　　┃
 *  ┗━┓　　　┏━┛
 *      ┃　　　┃　
 *      ┃　　　┃
 *      ┃　　　┃
 *      ┃　　　┃  神兽保佑
 *      ┃　　　┃  代码无bug　　
 *      ┃　　　┃
 *      ┃　　　┗━━━┓
 *      ┃　　　　　　　┣┓
 *      ┃　　　　　　　┏┛
 *      ┗┓┓┏━┳┓┏┛
 *        ┃┫┫　┃┫┫
 *        ┗┻┛　┗┻┛
 * 
 * @Author: sizaif
 * @Date: 2023-08-29 21:19:57
 * @LastEditTime: 2023-08-31 23:14:48
 * @Description: 
 * 
 */

#include <shm_mque.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <thread>
#include "../sdmessagebuf/sdmessagebuf.pb.h"
using namespace std;
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


void read_func(){
  ShmQueueMeta meta;

  meta.initQueue(ShmRole::CONSUMER,"/msgdata_server",50000, 10240, true);
  meta.printInfo();

  printf("now we in read_thread \n");
  char *t = new char[1024*1024*10];
  int num = 0;
  string str_;
  MSGData recv;
  while(1)
  {
    int occupiedElem = 0;
    int len_ = 0; 
    bool res = meta.dequeueElem(t, occupiedElem, len_);
    if(res == true)
    {
      // cout << t << endl;
      printf("read len: %d \n",len_);
      bool parse_suc = recv.ParseFromArray(t,len_);

      if(parse_suc){
        PrintMSGData(recv);
      }else{
        printf("反序列化失败\n");
      }
      
      cout << "occupiedElem: " << occupiedElem << ", elemSize: " << meta.elemSize << endl;
      num++;
    }

    if(num == 10) break;
  }
  printf("done...\n");
  
}



int main()
{

  thread read_thread(read_func);

  read_thread.join();

  printf("end:! \n");
  return 0;
}

