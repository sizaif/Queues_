/*** 
 * @_______________#########_______________________ 
 * @______________############_____________________ 
 * @______________#############____________________ 
 * @_____________##__###########___________________ 
 * @____________###__######_#####__________________ 
 * @____________###_#######___####_________________ 
 * @___________###__##########_####________________ 
 * @__________####__###########_####_______________ 
 * @________#####___###########__#####_____________ 
 * @_______######___###_########___#####___________ 
 * @_______#####___###___########___######_________ 
 * @______######___###__###########___######_______ 
 * @_____######___####_##############__######______ 
 * @____#######__#####################_#######_____ 
 * @____#######__##############################____ 
 * @___#######__######_#################_#######___ 
 * @___#######__######_######_#########___######___ 
 * @___#######____##__######___######_____######___ 
 * @___#######________######____#####_____#####____ 
 * @____######________#####_____#####_____####_____ 
 * @_____#####________####______#####_____###______ 
 * @______#####______;###________###______#________ 
 * @________##_______####________####______________ 
 * @
 * @Author: sizaif
 * @Date: 2023-08-30 21:37:41
 * @LastEditTime: 2023-08-30 22:02:52
 * @Description: 
 * @
 */



#include <stdio.h>
#include <thread>
#include <bits/stdc++.h>

#include "../include/que.h"
using namespace std;

bool _enabled_no_make_shared;
bool exit_;             // 告知写线程是否终端退出

ArrayLockFreeQueuePtr<Data> handlebufferALFQPtr_v2; // 线程之间的无锁队列 智能指针管理 直接使用TDATA
ArrayLockFreeQueuePtr<Data> handlebufferALFQPtr_v3; // 线程之间的无锁队列 使用裸指针 直接使用TDATA

void send_pkt2queue(Data *pkt_data)
{
    {
        // std::cout<<"try to send data with ptr"<<std::endl;
        // pkt_data 存入到队列后 会有新的指针指向其内容，pkt_data指针会被销毁
        // 裸指针
        if (likely(_enabled_no_make_shared))
        {
            pkt_data->seq_ = seq_que_++;
            while (!handlebufferALFQPtr_v3.enqueue_ptr(pkt_data) && !exit_)
            {
                DEBUGS("主线程存到队列失败, %s/%d ", __FUNCTION__, __LINE__);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
        else
        {
            // shared_ptr
            // 共享指针
            pkt_data->seq_ = seq_que_++;
            // In order to ensure memory pool security delete
            // use copy_pkt
            // Data copy_pkt = *pkt_data;
            Data copy_pkt = *pkt_data;
            // make_shared_ptr with pool
            std::shared_ptr<Data> pkt_v2 = make_shared_with_allocator(&memory_pool, std::move(copy_pkt));

            while (!handlebufferALFQPtr_v2.enqueue_sharedptr(std::move(pkt_v2)) && !exit_)
            {
                // wait until queue is not full
                DEBUGS("主线程存到队列失败, %s/%d ", __FUNCTION__, __LINE__);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    }
}

void write_func_alfq(){
    Data *pkt_data_ptr = new (vec_allocator.allocate(sizeof(Data))) Data{};
    while (!exit_){
        pkt_data_ptr->clear();
        pkt_data_ptr->inst_type = 1;
        pkt_data_ptr->extern_type = 2;
        // 写入数据到pkt_data_ptr;
        send_pkt2queue(pkt_data_ptr);
        if (exit_){
            break;
        }
    }
    delete pkt_data_ptr;
}

void read_func_alfq(){
    while (!exit_)
    {
        if (likely(_enabled_no_make_shared))
        {
            // 裸指针
            Data *pkt_data;
            while (!handlebufferALFQPtr_v3.try_dequeue_ptr(pkt_data) && !exit_)
            {
                // DEBUGS("处理线程从环形队列中取数据失败");
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            if (exit_){
                break;
            }
                
            // 解析处理 pkt_data 等操作

        }
        else
        {
            // share_ptr
            // 无锁环形队列
            std::shared_ptr<Data> packet_v2;
            while (!handlebufferALFQPtr_v2.try_dequeue_sharedptr(packet_v2) && !exit_)
            {
                // DEBUGS("处理线程从环形队列中取数据失败");
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                // wait until queue is not empty
            }
            if (exit_)
                break;

            Data *pData = packet_v2.get();
            // 解析处理 pData

        }
    }
}


void write_func_safeq(){
    Data data;
    u64 seq_cache_ = 0;
    while(!exit_){
        
        data.clear();
        // 构造数据
        data.seq_ = seq_cache_++;

        // send data
        cachebufferQueue.push(data);

    }
}
void read_func_safeq(){
    std::string write_context = "";
    Data pkt_rec;
    int itstime2End_ = 0;
    while (!exit_)
    {
        // 挂起等待
        cachebufferQueue.wait_and_pop(pkt_rec);
        if (exit_)
            break;
        
        // 处理数据Data
        std::string msg = pkt_rec.write_context;
    }
}

int main(int argc, char **argv) {

    _enabled_no_make_shared = true;// 裸指针
    
    exit_ = false;  // 这里exit_ 可以修改成其他条件

    thread read_thread_1(write_func_alfq);
    thread write_thread_1(read_func_alfq);


    read_thread_1.join();
    write_thread_1.join();
    
    thread read_thread_2(read_func_safeq);
    thread write_thread_2(write_func_safeq);
    read_thread_2.join();
    write_thread_2.join();
    return 0;
}