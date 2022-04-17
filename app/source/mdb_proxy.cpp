#include "mdb_proxy.h"
#include "pthread.h"
#include <sys/prctl.h>

namespace MMdbProxy
{
    void* CMdbProxyImpl::run(void* param)
    {
        ((CMdbProxyImpl*)param)->accept();
    }

    int32 CMdbProxyImpl::accept()
    {        
        while(m_bStatus)
        {
            int32 iSock = m_cServerSocket.accept();
            if(iSock > 0)
            {
            	if(m_sockQue.full())
        	{
        	//LOG_ERROR(MDB_PROXY_QUEUE_FULL,"queue is full,close current sock_id = %d",iSock)
        	close(iSock);
			continue;
        	}
                m_sockQue.push(iSock);     
            }
        }
        m_sockQue.destory();
        m_cServerSocket.shutdown();
	    m_bStatus = false;
        pthread_exit(NULL);
    }

    int32 CMdbProxyImpl::start_serv(int32 iPort)
    {
       	int32 iRet = -1;
        pthread_attr_t attr;
        if((iRet = pthread_attr_init(&attr)) != 0)
        {
            //LOG_ERROR(MDB_PROXY_TRHEAD_INIT_ERROR, "\n==[MDB_PROXY]==> pthread init, code:%d, errmsg:%s", iRet, strerror(iRet));
            return MDB_PROXY_TRHEAD_INIT_ERROR;
        }
        if((iRet = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0)
        {
            //LOG_ERROR(MDB_PROXY_TRHEAD_DETACH_ERROR, "\n==[MDB_PROXY]==> pthread detach, code:%d, errmsg:%s", iRet, strerror(iRet));
            return MDB_PROXY_TRHEAD_DETACH_ERROR;
        }
        size_t iStackSize = 4 * 1024 * 1024;
        if((iRet = pthread_attr_setstacksize(&attr, iStackSize)) != 0)
        {
            //LOG_ERROR(MDB_PROXY_TRHEAD_SETSTACK_ERROR,"\n==[MDB_PROXY]==> pthread set stack size, code:%d, errmsg:%s", iRet, strerror(iRet));
            return MDB_PROXY_TRHEAD_SETSTACK_ERROR;
        }
        if((iRet = pthread_attr_getstacksize(&attr, &iStackSize)) != 0)
        {
            //LOG_ERROR(MDB_PROXY_TRHEAD_GETSTACK_ERROR, "\n==[MDB_PROXY]==> pthread get stack size, code:%d, errmsg:%s", iRet, strerror(iRet));
            return MDB_PROXY_TRHEAD_GETSTACK_ERROR;
        }
        //LOG_ERROR(0,"\n==[MDB_PROXY]==> important information : thread size %d", iStackSize );

        m_cServerSocket.set_port(iPort,10);
        if (m_cServerSocket.start(iPort) == 0)
        {
            //LOG_ERROR(MDB_PROXY_TRHEAD_LISTEN_ERROR, "\n==[MDB_PROXY]==> start sock server error! Port:%d", iPort);
            return MDB_PROXY_TRHEAD_LISTEN_ERROR;
        }


        //create listen thread
        m_bStatus = true;
        pthread_t tid;
        if ((iRet = pthread_create(&tid, &attr ,run, (void *)this )) != 0)
        {
            //LOG_ERROR(MDB_PROXY_TRHEAD_CREATE_ERROR,"\n==[MDB_PROXY]==>listen pthread create fail, code:%d, errmsg:%s", iRet, strerror(iRet));
            return MDB_PROXY_TRHEAD_CREATE_ERROR;
        }

        pthread_attr_destroy(&attr);
        //LOG_ERROR(MDB_RPOXY_SUCCESS, "\n==[MDB_PROXY]==> MDB proxy ends success .");
		return 0;
    }

    int32 CMdbProxyImpl::deal_client(){
        std::string name = "deal_client";
        prctl(PR_SET_NAME, name.c_str());

        int32 iSock = -1;
		int32 iRet = 0 ;
		while(m_bStatus)
		{
			iSock = m_sockQue.pop();
            if(iSock <= 0)
            {
                //LOG_ERROR(-1,"\n==[MDB_PROXY]==> pop sock=%d, maybe is illegle,please check !",iSock);
                continue;
            }
				
	        NAMESPACE_SOCKET_PROXY::CSockApi sockClient(iSock);
	        pthread_t tid = pthread_self();

	        struct sockaddr_in clientAddr;
            socklen_t iClientLen = sizeof(clientAddr);
            getpeername(iSock, (struct sockaddr  *)&clientAddr, (socklen_t *)&iClientLen);

            NAMESPACE_SOCKET_PROXY::CNotify cInNotify;
	        NAMESPACE_SOCKET_PROXY::CNotify cOutNotify;
            while (m_bStatus)
	        {
	            iRet = sockClient.receive_notify(cInNotify);
                if (iRet != 1)
	            {
	                //LOG_TRACE("\n==[MDB_PROXY]==> Received notify is NULL(maybe client is closing), sock=%d, tid=%lld", iSock, tid);  
	                break; 
	            }

                //处理消息

                if (sockClient.send_notify(&cOutNotify) == 0)
	            {
	                //LOG_ERROR(MDB_PROXY_SEND_NOTIFY_ERROR,"\n==[MDB_PROXY]==> Send notify to client failed.");
	                break;
	            }
            }
            sockClient.close();
        }
    return 0;
    }
}
