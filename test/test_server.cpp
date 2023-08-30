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
 * @LastEditTime: 2023-08-30 20:04:30
 * @Description: 
 * 
 */

#include <shm_mque.h>
#include <bits/stdc++.h>
#include "../sdmessagebuf/sdmessagebuf.pb.h"

using namespace std;
typedef unsigned long long u64;


int main()
{
  ShmQueueMeta meta;
  meta.initQueue(ShmRole::PRODUCER,"/msgdata_server", 50000, 10240);
  meta.printInfo();

  std::string test = "123456780";
  

  MSGData msgdata;
  u64 ss = 1;
  msgdata.Clear();
  msgdata.set_seq_(1);
  msgdata.set_sock_count(ss);
  msgdata.set_vfile("foo_"+to_string(ss));
  msgdata.set_func_name("this is foo_func_"+to_string(ss));
  msgdata.set_vu1size(3);
  u64 a1 = ss+1000;
  u64 a2 = ss+1001;
  u64 a3 = ss+1002;
  msgdata.add_vu1(a1);
  msgdata.add_vu1(a2);
  msgdata.add_vu1(a3);

  for(int i = 1; i <= 10; i++)
  {
    msgdata.Clear();
    msgdata.set_seq_(i);
    msgdata.set_sock_count(i);
    msgdata.set_vfile("foo_"+to_string(i));
    msgdata.set_func_name("this is foo_func_"+to_string(i));
    if(i == 2){
      msgdata.set_write_context("abc");
    }
    if(i == 3){
      msgdata.set_write_context("abcasdadasdasd");
    }
    if(i == 4){
      msgdata.set_write_context("abcasdadasdasdfkawsf");
    }
    if(i == 5){
      msgdata.set_write_context("abcasdadasdasdfkawsfx");
    }

    msgdata.set_vu1size(3);
    a1 = ss+1000;
    a2 = ss+1001;
    a3 = ss+1002;
    msgdata.add_vu1(a1);
    msgdata.add_vu1(a2);
    msgdata.add_vu1(a3);
    std::string serialed_ = msgdata.SerializeAsString();
    printf("size: %lu , %lu, %lu \n",serialed_.size(),sizeof(MSGData), sizeof(msgdata));
    bool res = meta.enqueueElem((unsigned char*)(serialed_.c_str()),serialed_.size());
    if(res == false)
      printf("failed to put data %d \n", i);
    sleep(1);
  }
}

