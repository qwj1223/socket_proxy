#ifndef __MDB_PROXY_H__
#define __MDB_PROXY_H__

#include "queue_mgr.h"
#include "mdb_proxy_socknotify.h"
#include "mdb_proxy_sockapi.h"

namespace MMdbProxy
{
	class CMdbProxyImpl;
	class CThreadInfo
	{
	public:
		CThreadInfo()
		{
			m_pProxyImpl = NULL;
		}

	public:
		int32 			m_iTid;
		int32 			m_iSock;
		int32 			m_iClientSinPort;
		AISTD string 	m_strCliendAddr;
		CMdbProxyImpl 	*m_pProxyImpl;
	};

	class CMdbProxyImpl
	{
	public:
		CMdbProxyImpl() {m_bStatus = false;}
		~CMdbProxyImpl() {m_bStatus = false; }
		int32 start_serv(int32 iPort);
		int32 deal_client();
		static void* run(void *param);
		int32 accept();
		
	public:
		bool get_runStatus() { return m_bStatus; }
		void set_runStatue(bool bStatus) { m_bStatus =  bStatus; }

	private:
		bool m_bStatus;
		NAMESPACE_SOCKET_PROXY::CServerSock m_cServerSocket;
		Queue<int32> m_sockQue;
	};	
}

#endif //__MDB_PROXY_H__
