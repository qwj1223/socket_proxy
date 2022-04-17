#ifndef _QUEUE_MGR_H_
#define _QUEUE_MGR_H_
#include "pthread.h"

#define MAX_QUEUE 200

namespace MMdbProxy
{
    template<typename T>
    class Queue
    {
    public: Queue():len(0),offset(0),m_bDestory(false)
        {
            pthread_mutex_init(&lock, NULL);
            pthread_cond_init(&emptySignal, NULL);
            pthread_cond_init(&fullSignal, NULL);
        }

        bool full()
        {
        	//LOG_TRACE("the number of the queue is :%d\n",len);
            return (MAX_QUEUE == len);
        }
        bool empty()
        {
            return (0 == len);
        }
        T  pop()
        {
            pthread_mutex_lock(&lock);
            while(!m_bDestory && empty())
            {
                pthread_cond_wait(&emptySignal, & lock);
            }
            
            if(m_bDestory && empty())
            {
                pthread_cond_broadcast(&emptySignal);
                //LOG_TRACE(" queue is empty,and get destory signal, will destory");
            }
            else
            {
            T  clt = clts[offset];
            len --;
            offset = (offset+1 == MAX_QUEUE)?0:(offset+1);
              
            pthread_cond_broadcast(&fullSignal);
            pthread_mutex_unlock(&lock);
            
            return clt;
            }
            pthread_mutex_unlock(&lock);
            return NULL;
        }
        void push(T&  clt)
        {
            pthread_mutex_lock(&lock);
            while(!m_bDestory && full())
            {
                pthread_cond_wait(&fullSignal, & lock);
            }
            
            if(m_bDestory && empty())
            {
                pthread_cond_broadcast(&fullSignal);
            }
            else
            {
            clts[(offset+len)%MAX_QUEUE] = clt;
            len ++;
            
            pthread_cond_broadcast(&emptySignal);
            }
            pthread_mutex_unlock(&lock);
        }

		void destory()
        {
            m_bDestory = true;
            pthread_cond_broadcast(&emptySignal);
            pthread_cond_broadcast(&fullSignal);
            //LOG_TRACE("get destory signal ,will destory queue empty");
        }
		
    private: 
        T clts[MAX_QUEUE];
        int len;
        int offset;
        pthread_mutex_t lock;
        pthread_cond_t emptySignal;
        pthread_cond_t fullSignal;
        bool m_bDestory;
    };
}

#endif // _QUEUE_MGR_H_
