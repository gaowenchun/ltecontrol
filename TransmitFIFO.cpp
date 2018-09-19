//=====================================================================
//Name              :TransmitFIFO.cpp
//Author            :ict
//Version           :
//Description       :
//=====================================================================

#include "includes/TransmitFIFO.h"
#include "includes/log.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>


struct ListNode
{
        struct  ListNode* mNext;
        void* mData;
};

/** A fast FIFO for pointer-based storage. */
struct PointerFIFO
{
        struct ListNode* mHead;         ///< points to next item out
        struct ListNode* mTail;         ///< points to last item in
        unsigned mSize;                 ///< number of items in the FIFO
};

static unsigned ictGetFIFOSize(struct PointerFIFO* fifo){assert(fifo != NULL); return fifo -> mSize;}

struct PointerFIFO* ictInitFIFO()
{

        struct PointerFIFO* fifo = (struct PointerFIFO*)malloc(sizeof(struct PointerFIFO));
        if (fifo == NULL)
        {
                LOG_ERROR("ERROR:NO memory for PointerFIFO! \n");
                return NULL;
        }
        memset(fifo, 0,sizeof(struct PointerFIFO));
        return fifo;
}

void* ictGet(struct PointerFIFO* fifo)
{

        struct ListNode* next;
        void* retVal;

        assert( fifo != NULL );
        // empty list?
        if (fifo -> mHead == NULL) return NULL;
        // normal case
        next = fifo->mHead->mNext;
        retVal = fifo->mHead->mData;

        free(fifo->mHead);
        fifo->mHead = next;

        if (next==NULL)
                fifo->mTail=NULL;

        fifo->mSize--;
        return retVal;
}

void ictCleanFIFO(struct PointerFIFO* fifo)
{
         char  data = 0;
         assert( fifo != NULL );

         while( fifo -> mSize != 0 ){

                 assert( fifo -> mTail != NULL &&  fifo -> mHead != NULL  && fifo != 0);

                 void*  mNodedata=ictGet(fifo);
         }

         free( fifo );
}

void ictPut(struct PointerFIFO* fifo,void* msg_p)
{
        struct ListNode *node;

        assert(fifo != NULL);

        node = (struct ListNode* )malloc(sizeof(struct ListNode));

        if(node == NULL){
                LOG_ERROR("No memory for Node\n");
                return ;
        }
        node->mData = msg_p;
        node->mNext = NULL;

        if (fifo->mTail!=NULL)
                fifo->mTail->mNext= node;
   fifo->mTail = node;

        if (fifo -> mHead == NULL)
                fifo -> mHead = node;

        fifo -> mSize ++;

}

struct Mutex;

/** A class for recursive mutexes based on pthread_mutex. */
struct Mutex {

        pthread_mutex_t mMutex;
        pthread_mutexattr_t mAttribs;
};

static void lock(struct Mutex* mutex)
{
        assert( mutex != NULL );
        pthread_mutex_lock(&mutex -> mMutex);
}

static void unlock(struct Mutex* mutex)
{
        assert( mutex != NULL );
        pthread_mutex_unlock(&mutex -> mMutex);
}

struct Signal {
        pthread_cond_t mSignal;
};
static void ictWait(struct Signal* sg,struct Mutex* wMutex)
{
        assert(sg != NULL && wMutex != NULL );

//      lock(wMutex);
        pthread_cond_wait(&sg -> mSignal,&wMutex->mMutex);
//      unlock(wMutex);
}

static void ictSignal(struct Signal* sg)
{
        assert( sg != NULL );

        pthread_cond_signal(&sg -> mSignal);
}

static void ictBroadcast(struct Signal* sg)
{
        assert( sg != NULL );
        pthread_cond_broadcast(&sg -> mSignal);
}

/** A C++ wrapper for pthread threads.  */
struct Thread {
        pthread_t mThread;
        pthread_attr_t mAttrib;
        // FIXME -- Can this be reduced now?
        int mStackSize;

};

static void ictJoin(struct Thread* tr)
{
        assert( tr != NULL );
        int s = pthread_join(tr -> mThread,NULL);
        assert(!s);
}

struct Mutex* ictInitMutex()
{
        int res;
        struct Mutex* mutex ;

        mutex = (struct Mutex*)malloc(sizeof( struct Mutex ));
        if(!mutex){
                LOG_ERROR("Mutex is NULL\n");
                return NULL;
        }
        memset(mutex,0,sizeof(struct Mutex));

        res = pthread_mutexattr_init(&mutex->mAttribs);
        assert(!res);
        res = pthread_mutexattr_settype(&mutex->mAttribs,PTHREAD_MUTEX_RECURSIVE);
        assert(!res);
        res = pthread_mutex_init(&mutex->mMutex,&mutex->mAttribs);
        assert(!res);

        return mutex;
}

void ictCleanMutex(struct Mutex* mutex)
{
        int res;
        assert(mutex != NULL);
        pthread_mutex_destroy(&mutex -> mMutex);
        res = pthread_mutexattr_destroy(&mutex -> mAttribs);
}

static struct timespec ictGetTimeout(unsigned timeout)
{
        struct timeval now;
        unsigned sec,msec;
        struct timespec ret;
        /* timeout */
        sec =  timeout / 1000;
        msec = timeout % 1000;
        /* now */
        gettimeofday(&now,NULL);
        now.tv_usec += msec * 1000;
        now.tv_sec += sec;
        /*eclipse */
        if( now.tv_usec > 1000000 ){
                now.tv_usec -= 1000000;
                now.tv_sec += 1;
        }
        ret.tv_sec = now.tv_sec;
        ret.tv_nsec = 1000 * (long)now.tv_usec;
        return ret;
}
struct Signal* ictInitSignal()
{
        struct Signal* sg ;

        sg = (struct Signal*)malloc(sizeof(struct Signal));

        if( !sg ){
                LOG_ERROR(" No Memory for Signal\n");
                return NULL;
        }
        int ret = pthread_cond_init(&sg -> mSignal,NULL);
        if( ret!=0 ){
                LOG_ERROR("pthread_cond_init error %d\n" << ret);
                free(sg);
                return NULL;
        }
        return sg;
}
void ictCleanSignal(struct Signal* sg)
{
        pthread_cond_destroy(&sg -> mSignal);
        free(sg);
}

/** Block for the signal up to the cancellation timeout. */
void ictWait_for_timeout(struct Signal* sg,struct Mutex* wMutex, unsigned timeout)
{
        struct timespec waitTime ;
        struct timespec now;
        assert(sg != NULL && wMutex != NULL);
        waitTime = ictGetTimeout(timeout);
        pthread_cond_timedwait(&sg -> mSignal,&wMutex->mMutex,&waitTime);
}
struct Thread* ictInitThread(int wStackSize )
{
        struct Thread* thread;

        thread = (struct Thread*) malloc(sizeof(struct Thread));
        if( !thread ){
                LOG_ERROR("No memory for Thread.\n");
                return NULL;
        }
        if( wStackSize < 1024 ){
                wStackSize = 65536*4;
        }
        thread -> mThread = (pthread_t)0;
        thread ->  mStackSize=wStackSize;
        return thread;
}
void ictCleanThread(struct Thread* th)
{
        assert( th != NULL );
        pthread_attr_destroy(&th->mAttrib);
        free(th);
}

/** Start the thread on a task. */
void ictStart(struct Thread* thread,void *(*task)(void*), void *arg)
{
        int res;
        assert(thread -> mThread==((pthread_t)0));
        res = pthread_attr_init(&thread->mAttrib);
        assert(!res);
        res = pthread_attr_setstacksize(&thread -> mAttrib, thread->mStackSize);
        assert(!res);
        res = pthread_create(&thread -> mThread, &thread -> mAttrib, task, arg);
        assert(!res);
}

struct Mutex* InitMutex()
{
    int res;
    struct Mutex* mutex ;
    mutex = (struct Mutex*)malloc(sizeof( struct Mutex ));
    if(!mutex){
        LOG_ERROR("Mutex is NULL\n");
        return NULL;
    }
    memset(mutex,0,sizeof(struct Mutex));
    res = pthread_mutexattr_init(&mutex->mAttribs);
    assert(!res);
    res = pthread_mutexattr_settype(&mutex->mAttribs,PTHREAD_MUTEX_RECURSIVE);
    assert(!res);
    res = pthread_mutex_init(&mutex->mMutex,&mutex->mAttribs);
    assert(!res);
    return mutex;
}

void CleanMutex(struct Mutex* mutex)
{
    int res;
    assert(mutex != NULL);
    pthread_mutex_destroy(&mutex -> mMutex);
    res = pthread_mutexattr_destroy(&mutex -> mAttribs);
}

struct InterthreadQueue* TransmitFIFO::QueueOpen()
{
    struct InterthreadQueue* queue;
    queue =(struct InterthreadQueue*)malloc(sizeof(struct InterthreadQueue));
    if(!queue){
        LOG_ERROR("No Memory for Interthread\n");
        return NULL;
    }
    queue -> mQ = ictInitFIFO();
    if(! queue -> mQ ){
        QueueClose(queue);
        return NULL;
    }
    queue -> mLock = InitMutex();
    if(! queue -> mLock ){
        QueueClose(queue);
        return NULL;
    }
    queue -> mWriteSignal = ictInitSignal();
    if(! queue -> mWriteSignal ){
     QueueClose(queue);
        return NULL;
    }
    return queue;
}

void TransmitFIFO::QueueClose(struct InterthreadQueue* tq)
{
    assert(tq != NULL);
    lock(tq -> mLock);
    while(ictGetFIFOSize(tq -> mQ)>0)
    free(ictGet(tq -> mQ));
    ictCleanFIFO(tq -> mQ);
    unlock(tq -> mLock);
    CleanMutex(tq -> mLock);
    ictCleanSignal(tq -> mWriteSignal);
    free(tq);
}
void Write(struct InterthreadQueue* tq,void* val)
{
    assert(tq != NULL && val != NULL);
    lock(tq->mLock);
    ictPut(tq->mQ,val);
    ictSignal(tq->mWriteSignal);
    unlock(tq->mLock);
}

void* readNoBlock(struct InterthreadQueue* tq)
{
    assert(tq != NULL);
    void* retVal;
    lock(tq->mLock);
    retVal = ictGet(tq->mQ);
    unlock(tq->mLock);
    return retVal;
}

