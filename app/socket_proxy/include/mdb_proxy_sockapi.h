#ifndef     _MDB_PROXY_SOCKAPI_H_
#define     _MDB_PROXY_SOCKAPI_H_

#include    <netdb.h>
#include    <unistd.h>
#include    <arpa/inet.h>
#include    <sys/socket.h>
#include    <string.h>
#include    <sys/ioctl.h>
#include    "mdb_proxy_socknotify.h"

#define SOCK_TIME_OUT
#define LINUX

namespace NAMESPACE_SOCKET_PROXY
{
    class CSockApi
    {
    public:
        enum
        {
            STS_ER, //error, socket read or write error status
            STS_CL, //close, socket initialize or close status
            STS_OK  //connect, socket connect status
        };
    public:
        CSockApi();
        CSockApi(int nSock);
        virtual ~CSockApi();

        int    send_notify(CNotify& pNotify);
        int    receive_notify(CNotify& pNotify);
		//add 20140328 by yangyz
		int    receive_notify_timed(CNotify& pNotify);

        int    send_notify_win32(CNotify& pNotify);
        int    receive_notify_win32(CNotify& pNotify);

        CNotify*    receive_notify();
		//add 20140328 by yangyz
		CNotify*	receive_notify_timed();
        int     send_notify( CNotify* pNotify );

        CNotify*    receive_notify_win32();
        int     send_notify_win32( CNotify* pNotify );

        int     commit_notify(CNotify& cSend, CNotify& cRecv);
        CNotify*    commit_notify(CNotify& cSend);

        int     commit_notify_win32(CNotify& cSend, CNotify& cRecv);
        CNotify*    commit_notify_win32(CNotify& cSend);
        
        void    close();
        void    set_sock(int nSock);
        int     get_sock()  { return m_nSock;   }
        int     get_status()    { return m_nStatus; }

        int     get_peer(char* szPeer);
        int     get_local(char* szLocal);

    public:
        int readTO(int nSec);
        int writeTO(int nSec);

    //protected:
        int readN( char* pBuf, int nLen);
		int	readNT( char* pBuf, int nLen);
        int writeN( const char* pBuf, int nLen);

    protected:
        int m_nSock;
        int m_nPort;
        int m_nPortNum;
        int m_nStatus;
	public:
		static unsigned int m_nReadTimeOut;
		static unsigned int m_nMaxNotifyLen;
    };

    class CServerSock : public CSockApi
    {
    public:
        int     listen(); //return nSock
        int     accept(); // return client sock
        void    shutdown();
        int     start( int nPort );
        void    set_port(int nPort,int nPortNum = 5);
    };

    class CClientSock : public CSockApi
    {
    public:
        CClientSock();
        CClientSock(int nPort, const char* szHostName, int nPortNum);
        void    set_portAndHost(int nPort, const char* szIP,int nPortNum=5);
        int     connect();
        void    reconnect();
    private:
        char    m_szHostname[256];
    };
}

#endif //_MDB_PROXY_SOCKAPI_H_
