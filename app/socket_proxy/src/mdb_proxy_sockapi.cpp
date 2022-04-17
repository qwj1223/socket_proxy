#include	<stdio.h>
#include	<strings.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<signal.h>
#include	<errno.h>
#include	<netinet/in.h>
#include	<netinet/tcp.h>

#ifndef LINUX
#include	<stropts.h>
#endif

#ifdef SUNOSR
#include        <sys/filio.h>
#endif

#ifdef LINUX
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#endif

#include	"mdb_proxy_sockapi.h"
#include 	"mdb_proxy_socknotify.h"

namespace NAMESPACE_SOCKET_PROXY
{

	// ## CVS Version control APIs
	const static char *strCvsAuthor       = "@(#) $Author: chenyf $";
	const static char *strCvsDate         = "@(#) $Date: 2009/01/07 03:14:04 $";
	const static char *strCvsRevision     = "@(#) $Revision: 1.1.2.12 $";

	#define		BACK_LOG_NUM		256
	#define		CONNECT_TIME_OUT 	3

unsigned int CSockApi::m_nReadTimeOut = 0;
unsigned int CSockApi::m_nMaxNotifyLen = 0;

	CSockApi::CSockApi()
		: m_nSock(-1)
		, m_nPort(0)
		, m_nPortNum(5)
		, m_nStatus(STS_CL)
	{
	}

	CSockApi::CSockApi(int nSock)
		: m_nSock(nSock)
		, m_nPort(0)
		, m_nPortNum(5)
		, m_nStatus(STS_OK)
	{
		int iOpt = 1;
		if( setsockopt( m_nSock, IPPROTO_TCP, TCP_NODELAY, (const char*)&iOpt, sizeof(iOpt) ) < 0 )
		{
			//perror("Set construct nodelay option error - socket");
			//perror( strerror( errno));
		}
		
		iOpt = 1;
		if( setsockopt(nSock, SOL_SOCKET, SO_KEEPALIVE, (const char*)&iOpt, sizeof(iOpt) ) < 0 )
		{
			//perror("Set client keep alive option error - socket");
			//perror( strerror( errno));
		}
	}

	CSockApi::~CSockApi()
	{
		this->close();
	}

	void CSockApi::close()
	{
		if(m_nStatus != STS_CL)
		{
			int iRet = ::close(m_nSock);
			while( -1 == iRet && EINTR == errno)
			{
				iRet = ::close(m_nSock);
			}
			m_nSock = -1;//add by liukp 20090623
			m_nStatus = STS_CL;
		}
	}

	void CSockApi::set_sock(int nSock)
	{
		this->close();

		m_nSock = nSock;
		m_nStatus = STS_CL;

		if( m_nSock >= 0 )
		{
			m_nStatus = STS_OK;

			int iOpt = 1;
			if( setsockopt( m_nSock, IPPROTO_TCP, TCP_NODELAY, (const char*)&iOpt, sizeof(iOpt) ) < 0 )
			{
				//perror("Set sock nodelay option error - socket");
				//perror( strerror( errno));
			}
		}
	}

	// wait n second for read time out 
	int	CSockApi::readTO( int nSec )
	{
		struct timeval tv;
		tv.tv_sec = nSec;
		tv.tv_usec = 0;

		if( setsockopt(m_nSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv) ) < 0 )
		{
			//perror("Set read timeout error - timeout");
			//perror( strerror( errno));
			return 0;	
		}

		return 1;
	}

	// wait n second for write time out 
	int	CSockApi::writeTO( int nSec )
	{
		struct timeval tv;
		tv.tv_sec = nSec;
		tv.tv_usec = 0;

		if( setsockopt(m_nSock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv) ) < 0 )
		{
			//perror("Set write timeout error - timeout");
			//perror( strerror( errno));
			return 0;	
		}

		return 1;
	}

	// Read n bytes from descriptor
	int CSockApi::readN( char* pBuf, int nBytes)
	{
		int		nLeft = nBytes;
		int		nRead = 0;
		while( nLeft > 0 )
		{
			nRead = read( m_nSock, pBuf, nLeft);
			if( nRead < 0 )
			{
				if( errno == EINTR || errno == EAGAIN)  //wanggk-20120831-增加EAGAIN支持反算拆包
					continue;

				//perror("Read error - little than zero");
				//perror( strerror( errno));
				return	nRead;			// error, return < 0
			}
			else if( nRead == 0 )
			{
				break;					// EOF
			}

			nLeft -= nRead;
			pBuf += nRead;
		}
		return ( nBytes - nLeft);		// return >= 0
	}

	int CSockApi::readNT( char* pBuf, int nBytes)
	{
		fd_set  fdset;
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		if( m_nReadTimeOut > 0 )
		{	
			tv.tv_sec = m_nReadTimeOut;
			FD_ZERO( &fdset);
			FD_SET( m_nSock, &fdset);
		}
			
		int	nLeft = nBytes;
		int	nRead = 0;
		while( nLeft > 0 )
		{
			if( m_nReadTimeOut > 0 )
			{
				if( select(m_nSock + 1, &fdset, NULL, NULL, &tv) <= 0)
				{
					perror("Select error - ");
					perror( strerror( errno));
					break;
				}
				nRead = read( m_nSock, pBuf, 1);
			}
			else
			{
				nRead = read( m_nSock, pBuf, nLeft);
			}
			if( nRead < 0 )
			{
				if( errno == EINTR )
					continue;

				perror("Read error - little than zero");
				perror( strerror( errno));
				return	nRead;			// error, return < 0
			}
			else if( nRead == 0 )
			{
				break;					// EOF
			}

			nLeft -= nRead;
			pBuf += nRead;
		}
		return ( nBytes - nLeft);		// return >= 0
	}


	// write n bytes to a descriptor
	int CSockApi::writeN(  const char* pBuf, int nBytes)
	{
		int nLeft = nBytes;
		int nWrite = 0;
		while( nLeft > 0 )
		{
			nWrite = write( m_nSock, pBuf, nLeft);
			if( nWrite <= 0 )
			{
				if( errno == EINTR )
					continue;

				//perror("Write error - little than zero");
				//perror( strerror( errno));
				return nWrite;			// error, return <= 0
			}
		
			nLeft -= nWrite;
			pBuf += nWrite;
		}
		return ( nBytes - nLeft);		// return >= 0
	}

		int CSockApi::send_notify( CNotify& pNotify)
	{
//#ifdef NEED_NET_TRANS
	 //pNotify.hton();
//#endif
		int nLen = pNotify.get_len() + CNotify::NOTIFY_LEN;

		char* szBuf = pNotify.get_send();

		int nWrite = writeN( szBuf, nLen);

		if( nWrite != nLen)
		{
			m_nStatus = STS_ER;
			return 0;
		}

		return 1;
	}

	int	CSockApi::send_notify( CNotify* pNotify )
	{
		int nLen = pNotify->get_len() + CNotify::NOTIFY_LEN;

		char* szBuf = pNotify->get_send();

		int nWrite = writeN( szBuf, nLen);

		if( nWrite != nLen)
		{
			m_nStatus = STS_ER;
			return 0;
		}

		return 1;
	}

	int CSockApi::send_notify_win32( CNotify& pNotify)
	{
		return send_notify( pNotify);
	}

	int	CSockApi::send_notify_win32( CNotify* pNotify)
	{
		return send_notify( pNotify);
	}

	int CSockApi::receive_notify(CNotify& pNotify)
	{		
		int nRead = readN( pNotify.get_head(), CNotify::NOTIFY_LEN);
		if( CNotify::NOTIFY_LEN != nRead)
		{
			m_nStatus = STS_ER;
			return 0;
		}

		int nLen = pNotify.set_head();
		nRead = readN( pNotify.get_recv(), nLen);
		if( nRead != nLen)
		{
			m_nStatus = STS_ER;
			return 0;
		}

		return 1;
	}

	CNotify* CSockApi::receive_notify()
	{
		CNotify* pNotify = new CNotify;

		int nRead = readN( pNotify->get_head(), CNotify::NOTIFY_LEN);
		if( CNotify::NOTIFY_LEN != nRead)
		{
			m_nStatus = STS_ER;
			delete pNotify;
			return 0;
		}

		int nLen = pNotify->set_head();
		nRead = readN( pNotify->get_recv(), nLen);
		if( nRead != nLen)
		{
			m_nStatus = STS_ER;
			delete pNotify;
			return 0;
		}

		return pNotify;	
	}

	int CSockApi::receive_notify_timed(CNotify& pNotify)
	{
		if( readNT( (char*)&pNotify, CNotify::NOTIFY_LEN)
				!= CNotify::NOTIFY_LEN )
		{
			return 0;
		}
#ifdef NEED_NET_TRANS
		pNotify.ntoh();
#endif
		int	nLen = pNotify.get_len();
		if( nLen <= 0 || ( m_nMaxNotifyLen > 0 && nLen > m_nMaxNotifyLen ))
		{
			return 0;
		}

		pNotify.resize(nLen);

		int	nRead = readN( pNotify.get_data(), nLen);
		m_nStatus = (nRead == nLen ? STS_OK : STS_ER);
		return (nRead == nLen);
	}

	CNotify* CSockApi::receive_notify_timed()
	{
		char	szBuf[CNotify::NOTIFY_LEN];

		if( readNT( szBuf, CNotify::NOTIFY_LEN) != CNotify::NOTIFY_LEN )
		{
			m_nStatus = STS_ER;
			return NULL;
		}

		int*	pInt = (int*)szBuf;
		int		nType = *pInt++;
		int		nLen = *pInt++;
		int		nBlock = *pInt;
		nType = ntohl(nType);
		nLen = ntohl(nLen);
		nBlock = ntohl(nBlock);
		if( nLen <= 0 || ( m_nMaxNotifyLen > 0 && nLen > m_nMaxNotifyLen ))
		{
			return NULL;	
		}
		CNotify*	pNotify = new CNotify( nType, nLen, nBlock);
		if( readNT( pNotify->get_data(), nLen) != nLen)
		{
			delete	pNotify;
			m_nStatus = STS_ER;
			return NULL;
		}
		m_nStatus = STS_OK;
		return pNotify;	
	}

	int CSockApi::receive_notify_win32(CNotify& pNotify)
	{
		return receive_notify( pNotify);
	}

	CNotify* CSockApi::receive_notify_win32()
	{
		return receive_notify();
	}

	int CSockApi::commit_notify(CNotify& cSend, CNotify& cRecv)
	{
		int nRet = send_notify(cSend);

		if(nRet == 0)
		{
			return receive_notify(cRecv);
		}

		return nRet;
	}

	CNotify* CSockApi::commit_notify(CNotify& cSend)
	{
		if(send_notify(cSend) != 0)
			return 0;

		return receive_notify();
	}

	int CSockApi::commit_notify_win32(CNotify& cSend, CNotify& cRecv)
	{
		int nRet = send_notify_win32(cSend);

		if(nRet == 0)
		{
			return receive_notify_win32(cRecv);
		}

		return nRet;
	}

	CNotify* CSockApi::commit_notify_win32(CNotify& cSend)
	{
		if(send_notify_win32(cSend) != 0)
			return 0;

		return receive_notify_win32();
	}

	int CSockApi::get_peer(char* szPeer)
	{
		struct sockaddr_in	peerAddr;
	#ifdef HPUX
		int	nLen = sizeof(peerAddr);
	#else
		socklen_t	nLen = sizeof(peerAddr);
	#endif
		if(getpeername(m_nSock, (struct sockaddr *)&peerAddr, (socklen_t *)&nLen) == 0)
		{
			strcpy(szPeer, inet_ntoa(peerAddr.sin_addr));
			return int(peerAddr.sin_port);
		}

		//perror("Peer error - sock");
		//perror( strerror( errno));

		return -1;
	}

	int CSockApi::get_local(char* szLocal)
	{
		struct sockaddr_in	localAddr;
	#ifdef HPUX
		int	nLen = sizeof(localAddr);
	#else
		socklen_t	nLen = sizeof(localAddr);
	#endif
		if(getsockname(m_nSock, (struct sockaddr *)&localAddr, (socklen_t *)&nLen) == 0)
		{
			strcpy(szLocal, inet_ntoa(localAddr.sin_addr));
			return int(localAddr.sin_port);
		}

		//perror("Local error - sock");
		//perror( strerror( errno));

		return -1;
	}

	// CServerSock
	void CServerSock::shutdown()
	{
		::shutdown( m_nSock, SHUT_RDWR);
		m_nStatus = STS_ER;
	}

	int CServerSock::start(int nPort)
	{
		m_nPort = nPort;

		return listen();
	}

	int CServerSock::listen()
	{
		if ( 0 == m_nPort )
			return 0;

		int nSock = socket( AF_INET, SOCK_STREAM, 0);
		if( -1 == nSock )
		{
			//perror("Start server error - socket");
			//perror( strerror( errno));
			return 0;
		}

		int iOpt = 1;
		if( setsockopt(nSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&iOpt, sizeof(iOpt) )  < 0 )
		{
			//perror("Set server reuse port option error - socket");
			//perror( strerror( errno));
			::close( nSock);
			return 0;
		}

		char	bBind = 0;
		for( int i = 0 ; i < m_nPortNum ; i++ )
		{
			struct sockaddr_in	sin;
			sin.sin_family		= AF_INET;
			sin.sin_addr.s_addr = htonl(INADDR_ANY);
			sin.sin_port		= htons( m_nPort + i);
			
			int port = m_nPort + i;
			//printf("port: %d\n", port);
			if( bind( nSock, (struct sockaddr*)&sin, sizeof(sin)) < 0 )
				continue;

			bBind = 1;
			break;
		}
		if( !bBind)
		{
			//perror("Start server error - bind");
			//perror( strerror( errno));
			::close( nSock);
			return 0;
		}

		::listen( nSock, BACK_LOG_NUM);
		m_nSock = nSock ;
		m_nStatus = STS_OK;

		return nSock;
	}

	int CServerSock::accept()
	{
		struct	sockaddr_in	clientAddr;

		socklen_t	nClientLen = 1;
		int			nClientSock;

		#ifdef HPUX
		{
			nClientSock = ::accept( m_nSock, 
					(struct sockaddr*)&clientAddr, 
					(socklen_t*)&nClientLen);
					//(int*)&nClientLen); (socklen_t*)&nClientLen)
		}
		#else
		{
			nClientSock = ::accept( m_nSock, 
					(struct sockaddr*)&clientAddr, 
					&nClientLen); /*(socklen_t*)&nClientLen);*/
		}
		#endif
		
		if( nClientSock == -1 )
		{
			m_nStatus = STS_ER;
			//perror("Accept error");
			//perror( strerror( errno));
			return -1;
		}

		return nClientSock;
	}

	void CServerSock::set_port(int nPort, int nPortNum)
	{
		m_nPort = nPort;
		m_nPortNum = nPortNum;
	}

	// CClientSock
	CClientSock::CClientSock()
	{
		m_szHostname[0] = 0;
	}

	CClientSock::CClientSock(int nPort, const char* szIP,int nPortNum)
	{
		set_portAndHost(nPort, szIP, nPortNum);
	}

	void CClientSock::set_portAndHost(int nPort, const char* szIP,int nPortNum)
	{
		m_nPort = nPort;
		strcpy( m_szHostname, szIP);
		m_nPortNum = nPortNum;
	}

	void CClientSock::reconnect()
	{
		while(1)
		{
			if( connect())
			{
				return;
			}

			sleep(1);
		}
	}

	int CClientSock::connect()
	{
		if(m_nPort == 0)
		{
			//perror("Connect error - port zero");
			return 0;
		}

		this->close();

		for( int i = 0 ; i < m_nPortNum ; i++ )
		{
			int nSock = socket( AF_INET, SOCK_STREAM, 0); 
			if( nSock == -1 )
			{
				//perror("Connect error - socket");
				//perror( strerror( errno));
				return 0;
			}

	#ifdef SUNOSR
			struct hostent	host;
			struct hostent*  phost;
			char buffer[8192];
			int32 iRet = 0;
			phost = gethostbyname_r( m_szHostname, &host, buffer, sizeof(buffer), &iRet );
	#else

		#ifdef HPUXR
			struct hostent  *pHost = gethostbyname( m_szHostname);
        	if( !pHost )
        	{
        		//perror("Connect error - gethostbyname_r");
				//perror( strerror( errno));
				::close( nSock);
				return 0;
        	}
       		struct hostent&	host = *pHost;
       		int32 iRet = 0;
		//gethostbyname���̰߳�ȫ����LINUX�����£���Ҫ����Ϊgethostbyname_r
		#elif defined(LINUX)
			struct hostent	host;
			struct hostent*  phost;
			char buffer[8192];
			int32 iRet = 0;
			int32 iErrCode =0;
			iRet = gethostbyname_r( m_szHostname, &host, buffer, sizeof(buffer),&phost, &iErrCode );
		#else
			struct hostent	host;
			struct hostent_data ht_data;
			int32 iRet = gethostbyname_r( m_szHostname, &host, &ht_data );
		#endif
	#endif
			if( iRet )
			{
				//perror("Connect error - gethostbyname_r");
				//perror( strerror( errno));
				::close( nSock);
				return 0;
			}

			int iFlag = 1;
			if( ioctl( nSock, FIONBIO, &iFlag) < 0)
			{
				//perror("IO unblock crontol error - ioctl");
				//perror( strerror( errno));
				::close( nSock);
				return 0;
			}
			
			struct sockaddr_in	sin;
			bzero( &sin, sizeof( sin ) );
			sin.sin_family	= AF_INET;
			sin.sin_port	= htons( m_nPort + i);
			bcopy( host.h_addr, &sin.sin_addr, host.h_length);

			iRet = ::connect( nSock, (struct sockaddr *)&sin, sizeof(sin));
			if( iRet < 0 && errno == EINPROGRESS)
			{
				struct timeval tm;
				tm.tv_usec = 0;
				tm.tv_sec = CONNECT_TIME_OUT;

	//			struct fd_set set;
				fd_set set;
				FD_ZERO( &set);
				FD_SET( nSock, &set);

				if( select( nSock + 1, &set, &set, NULL, &tm) <= 0)
				{
					//perror("select error - select");
					//perror( strerror( errno));
					::close( nSock);
					return 0;
				}

				if( FD_ISSET( nSock, &set))
				{
					int iOpt = 0;
					#ifdef HPUX
						int	iLen = sizeof(int);
					#else
						socklen_t	iLen = sizeof(int);
					#endif
					if( getsockopt( nSock, SOL_SOCKET, SO_ERROR, &iOpt, (socklen_t*)&iLen) < 0)
					//socklen_t iLen = sizeof( int);
					//if( getsockopt( nSock, SOL_SOCKET, SO_ERROR, &iOpt, &iLen) < 0)
					{
						//perror("get socket opt - getsockopt");
						//perror( strerror( errno));
						::close( nSock);
						return 0;
					}

					if(iOpt == 0)
					{
						iRet = 0;	//connect sucess flag
					}
				}
			}

			iFlag = 0;
			if( ioctl( nSock, FIONBIO, &iFlag) < 0)
			{
				//perror("IO non block crontol error - ioctl");
				//perror( strerror( errno));
				::close( nSock);
				return 0;
			}
			
			if( iRet == 0)
			{
				int iOpt = 1;
				if( setsockopt(nSock, SOL_SOCKET, SO_KEEPALIVE, (const char*)&iOpt, sizeof(iOpt) ) < 0 )
				{
					//perror("Set client keep alive option error - socket");
					//perror( strerror( errno));
				}

				if( setsockopt(nSock, IPPROTO_TCP, TCP_NODELAY, (const char*)&iOpt, sizeof(iOpt) ) < 0 )
				{
					//perror("Set client nodelay option error - socket");
					//perror( strerror( errno));
				}

				m_nStatus = STS_OK;
				m_nSock = nSock;
				return nSock;
			}
			//perror( strerror( errno));
			::close( nSock);
		}

		//perror("Connect error - connect");
		return 0;
	}

}
