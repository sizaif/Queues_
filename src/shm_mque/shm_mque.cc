#include "shm_mque.h"
#include <stdio.h>
#include <iostream>
ShmQueueMeta::ShmQueueMeta():
  maxElem(0), elemSize(0), totalDataSize(0), queueSize(0), name(nullptr), shmemFd(-1), queue(nullptr)
{}


ShmQueueMeta::~ShmQueueMeta()
{
  if(this->role == ShmRole::CONSUMER){
    printf("we called :~ShmQueueMeta \n");
    closeQueue();
  }
    
}


void ShmQueueMeta::cleanInit()
{
  if (this->shmemFd != -1)
  {
    close(this->shmemFd);
    shm_unlink(name);
  }
  free(name);
}


void ShmQueueMeta::closeQueue()
{
  munmap(this->queue, this->queueSize);
  cleanInit();
}


bool ShmQueueMeta::initQueue(ShmRole role,const char *name, uint32_t maxElem, uint32_t elemSize, bool removeFile)
{
  if(removeFile) shm_unlink(name);
  return initQueue(role,name, maxElem, elemSize);
}


bool ShmQueueMeta::initQueue(ShmRole role,const char *name, uint32_t maxElem, uint32_t elemSize)
{
  bool created = false;

  this->maxElem       = maxElem;
  this->elemSize      = elemSize;
  this->totalDataSize = maxElem * (sizeof(ElemMeta) + elemSize);
  this->name          = strdup(name);
  this->queueSize     = totalDataSize + sizeof(ShmQueue) - 1;
  this->role          = role;
  std::cout<<"total: "<<totalDataSize<<" , sizeof(elemeata: "<<sizeof(ElemMeta)<<" , elesize "<< elemSize<<" , sizeof(shmque)"<<sizeof(ShmQueue)<<"\n";

  this->shmemFd = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
  if (this->shmemFd == -1)
  {
    if (errno == ENOENT)
    {
      this->shmemFd = shm_open(name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
      if (this->shmemFd == -1)
      {
        cleanInit();
        return false;
      }
      created = true;
    }
    else
    {
      cleanInit();
      return false;
    }
  }

  if (created && (-1 == ftruncate(this->shmemFd, this->queueSize)))
  {
    cleanInit();
    return false;
  }
  
  this->queue = (ShmQueue*) mmap(NULL, this->queueSize, PROT_READ | PROT_WRITE, MAP_SHARED, this->shmemFd, 0);
  printf(" shmemFd: %d ,%x \n",this->shmemFd,this->queue);
  if (this->queue == MAP_FAILED)
  {
    cleanInit();
    return false;
  }

  if (created)
  {
    queue->rear = queue->front = queue->numElem = 0;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    pthread_mutex_init(&queue->lock, &attr);

    pthread_mutexattr_destroy(&attr);
  }

  return true;
}


bool ShmQueueMeta::isFull()
{
  if(queue->numElem == maxElem)
  {
    return true;
  }

  return false;
}


bool ShmQueueMeta::isEmpty()
{
  if(queue->numElem == 0)
  {
    return true;
  }

  return false;
}


void ShmQueueMeta::printInfo()
{
  printf("===== ShmQueueMeta: %s =====\n", name);
  printf("maxElem %d, elemSize %d, elemMetaSize %ld \n", maxElem, elemSize, sizeof(ElemMeta));
  printf("totalDataSize: %d\n", totalDataSize);
  printf("ShmQueue size: %ld\n", sizeof(ShmQueue));
  printf("queueSize: %d\n", queueSize);
}


bool ShmQueueMeta::enqueueElem(void *element, int len, bool blocking)
{
  curElemAccess = false;
  return enqueueElemPart(element, 0, len, blocking, true);
}

bool ShmQueueMeta::dequeueElem(void *element, int &occupiedSize, int &len, bool blocking)
{
  return dequeueElemPart(element, occupiedSize, 0, len, blocking, true);
}


bool ShmQueueMeta::dequeueElem(void *element, uint32_t &occupiedSize, uint32_t &bufSize, bool blocking)
{
  if(blocking)
  {
    while(isEmpty()) std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  else
  {
    if(isEmpty()) return false;
  }

  pthread_mutex_lock(&queue->lock);

  uint32_t elemHead = queue->front * (sizeof(ElemMeta)+elemSize);
  elemMeta = (ElemMeta*)(&queue->data[elemHead]);

  occupiedSize = elemMeta->occupiedSize;
  if(bufSize < occupiedSize)
  {
    pthread_mutex_unlock(&queue->lock);
    printf("bufSize %d < occupiedSize %d \n", bufSize, occupiedSize);
    return false;
  }

  memcpy(element, &queue->data[elemHead + sizeof(ElemMeta)], occupiedSize);

  queue->numElem--;
  queue->front = (queue->front+1) % maxElem;

  pthread_mutex_unlock(&queue->lock);
  return true;
}


// enqueueElem from REAR
bool ShmQueueMeta::enqueueElemPart(void *element, int offset, int len, bool blocking, bool done)
{
  if(blocking)
  {
    while(isFull()) std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  else
  {
    if(isFull()) return false;
  }

  if (offset+len > this->elemSize)
  {
    return false;
  }

  pthread_mutex_lock(&this->queue->lock);


  uint32_t elemHead = queue->rear * (sizeof(ElemMeta)+elemSize);
  elemMeta = (ElemMeta*)(&queue->data[elemHead]);

  // first access to this elem
  // if(curElemAccess == false)
  // {
  //   curElemAccess = true;
  //   elemMeta->init();
  // }
  elemMeta->occupiedSize = 0;
  memcpy(&queue->data[elemHead + sizeof(ElemMeta) + offset], element, len);
  elemMeta->occupiedSize += len;

  if(done == true)
  {
    queue->numElem++;
    queue->rear = (queue->rear+1) % maxElem;
    curElemAccess = false;
  }

  pthread_mutex_unlock(&queue->lock);
  return true;
}


// dequeueElem from FRONT
bool ShmQueueMeta::dequeueElemPart(void *element, int &occupiedSize, int offset, int &len, bool blocking, bool done)
{
  if(blocking)
  {
    while(isEmpty()) std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  else
  {
    if(isEmpty()){
      return false;
    } 
  }

  pthread_mutex_lock(&queue->lock);

  uint32_t elemHead = queue->front * (sizeof(ElemMeta)+elemSize);
  elemMeta = (ElemMeta*)(&queue->data[elemHead]);

  if (offset+len > elemMeta->occupiedSize)
  {
    pthread_mutex_unlock(&queue->lock);
    return false;
  }

  occupiedSize = elemMeta->occupiedSize;
  len = occupiedSize;
  memcpy(element, &queue->data[elemHead + sizeof(ElemMeta) + offset], occupiedSize);

  // memcpy(element, &queue->data[elemHead + sizeof(ElemMeta) + offset], len);
  if(done == true)
  {
    queue->numElem--;
    queue->front = (queue->front+1) % maxElem;
  }

  pthread_mutex_unlock(&queue->lock);
  return true;
}

