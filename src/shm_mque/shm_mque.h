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
 * @Date: 2023-08-29 21:19:57
 * @LastEditTime: 2023-08-31 17:16:34
 * @Description: 
 * @
 */

#pragma once

#include <thread>
#include <cstdint>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

class ElemMeta
{
public:
  uint32_t occupiedSize;
  void init()
  {
    occupiedSize = 0;
  }
};


class ShmQueue
{
public:
  pthread_mutex_t lock;
  uint32_t rear;
  uint32_t front;
  uint32_t numElem;
  // char data[1];
  char data[]; // 柔性数组，长度可根据实际需要分配
};

enum class ShmRole: uint32_t
{
    PRODUCER = 0,
    CONSUMER = 1,
};
/*
 * | ShmQueue                                                                                           |
 * | ShmQueue | ElemMeta + Elem Data | ElemMeta + Elem Data | ElemMeta + Elem Data | ... |
 */
class ShmQueueMeta
{
public:
  uint32_t maxElem;
  uint32_t elemSize;
  uint32_t totalDataSize;
  uint32_t queueSize;

  ShmRole role;
  char* name;
  int shmemFd;
  ShmQueue* queue;

  ElemMeta *elemMeta;
  bool curElemAccess;

  ShmQueueMeta();
  ~ShmQueueMeta();

  bool initQueue(ShmRole role,char const* name, uint32_t maxElem, uint32_t elemSize);
  bool initQueue(ShmRole role,char const* name, uint32_t maxElem, uint32_t elemSize, bool removeFile);
  void cleanInit();
  void closeQueue();

  bool isFull();
  bool isEmpty();

  void printInfo();

  // blocking & nonblocking semantics
  bool enqueueElem(void* element, int len, bool blocking=false);
  bool dequeueElem(void* element, int &occupiedSize, int &len, bool blocking=false);
  bool dequeueElem(void* element, uint32_t &occupiedSize, uint32_t &bufSize, bool blocking=false);

  bool enqueueElemPart(void* element, int offset, int len, bool blocking, bool done);
  bool dequeueElemPart(void* element, int &occupiedSize, int offset, int &len, bool blocking, bool done);
};

