#ifndef _MDB_PROXY_SOCKNOTIFY_
#define _MDB_PROXY_SOCKNOTIFY_
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>
//#include "compile.h"

#ifdef OB_NO_STD
#include <iostream.h>
#else
#include <iostream>
#endif

/*!	modify history
 *		chenyf, 20040510: CNotify(int nType, int nLen, int nBlock = 0) 函数
 *			 				指定长度有效性判断。
 *		chenyf, 20040510: void	set_data(char* pData ) 函数
 *							若buf内存已分配，不再重新分配内存， 提高性能。 
 */

typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

namespace NAMESPACE_SOCKET_PROXY
{

	#define AISIZEOF(x) (sizeof(x) + (sizeof(x) % 8 ? (8 - sizeof(x) % 8) : 0))

	template<int SIZE>
	class	CSizeBuffer
	{
	private:
		CSizeBuffer(const CSizeBuffer&);
		CSizeBuffer& operator=(const CSizeBuffer&);

	public:
		enum
		{
			MIN = sizeof(int) * 4,
			BUFSIZE = SIZE < MIN ? MIN : SIZE
		};

		CSizeBuffer()	
		{
			m_nRealLen = 0;
			m_pData = new char[BUFSIZE];
			m_nBufLen = BUFSIZE;
		}
		~CSizeBuffer()
		{
			if( NULL != m_pData )
			{
				delete[] m_pData;
			}
		}
	public:

		int set_content(const int& nContentLen, const char* m_pContent )
		{
			if( ( m_nRealLen + nContentLen ) > m_nBufLen )
			{
				m_nBufLen += ( ( m_nRealLen + nContentLen - m_nBufLen ) <= BUFSIZE ) ? \
								BUFSIZE	: ( m_nRealLen + nContentLen - m_nBufLen );
				
				char* pTemp = new char[m_nBufLen];
				if( NULL == pTemp )
				{
					return -1;
				}
				memcpy( pTemp, m_pData, m_nRealLen );
				delete[] m_pData;
				m_pData = pTemp;
			}
			memcpy( m_pData+m_nRealLen, m_pContent, nContentLen );
			m_nRealLen += nContentLen;
			
			return 0;
		}
		
		int	set_content_at( const int&nPosition, const int& nContentLen, const char* m_pContent )
		{
			if( ( nPosition + nContentLen ) > m_nBufLen )
			{
				m_nBufLen += BUFSIZE;
				m_nBufLen += ( ( nPosition + nContentLen - m_nBufLen ) <= BUFSIZE ) ? 
								BUFSIZE	: ( nPosition + nContentLen - m_nBufLen );

				char* pTemp = new char[m_nBufLen];
				if( NULL == pTemp )
				{
					return -1;
				}
				memcpy( pTemp, m_pData, nPosition );
				m_nRealLen = nPosition + nContentLen;
				delete[] m_pData;
				m_pData = pTemp;			
			}
			else if( ( nPosition + nContentLen ) > m_nRealLen )
			{
				m_nRealLen = nPosition + nContentLen;
			}
			
			memcpy( m_pData+nPosition, m_pContent, nContentLen );
			
			return 0;
		}

	public:
		int		m_nBufLen;
		int		m_nRealLen;
		char*	m_pData;	
	};

	typedef CSizeBuffer<4096> CBufStruct;

	class CNotify
	{
	public:
		enum { DATA_CONNECTION = 0, FRONT_CONNECTION, BACK_CONNECTION };

	public:
		void register_connection (int nType) { m_nType = nType; } 

	public:
		enum
		{
			NOTIFY_LEN = 12,
			RESERVE_LEN = 16,
			DATAOFF_LEN = 4
		};

	public:
		CNotify():m_nType(0),m_nLen(0),m_nBlock(0),m_nSize(0),m_pData(0)
		{
			resize( 0);
		}

		CNotify(int nType, int nLen, int nBlock = 0)
		{
			m_nType = nType;
			m_nLen = nLen;
			m_nBlock = nBlock;
			m_nSize = nLen;
			m_pData = 0;
			resize( nLen);
		}
		
		CNotify(const CNotify& cNotify)
		{
			m_nType = cNotify.m_nType;
			m_nLen = cNotify.m_nLen;
			m_nBlock = cNotify.m_nBlock;

			resize( m_nLen);

			memcpy( m_pData, cNotify.m_pData, m_nLen);
		}

		CNotify& operator=(const CNotify& cNotify)
		{
			if(&cNotify == this)
			{
				return *this;
			}

			m_nType = cNotify.m_nType;
			m_nLen = cNotify.m_nLen;
			m_nBlock = cNotify.m_nBlock;

			resize( m_nLen);

			memcpy( m_pData, cNotify.m_pData, m_nLen);
			return *this;
		}	

		~CNotify()
		{
			if ( m_pData != NULL)
			{
				delete[] m_pData;
			}
			m_pData = NULL;
		}

	public:
		void	set_type(int nType) { m_nType = nType; }
		int	get_type()	{ return m_nType;	}
		int	get_len()	{ return m_nLen < 0 ? 0 : m_nLen; }
		int	is_block()	{ return m_nBlock;	}
		int	resize(int nLen)
		{ 
			int nSize = nLen + RESERVE_LEN;
			if( m_nSize < nSize)
			{
				if( m_pData != NULL) 
				{
					delete[] m_pData;
				}
				m_nLen = nLen;
				m_nSize = nSize;
				m_pData = new char[m_nSize];
			}
			m_nLen = nLen;
			return nLen;
		}

		void	set_data(const char* pData,int nLen)	
		{ 
			resize( nLen);
			memcpy( m_pData + RESERVE_LEN, pData, m_nLen);
		}
		
		void	set_data(char* pData,int nLen)	
		{ 
			resize( nLen);
			memcpy( m_pData + RESERVE_LEN, pData, m_nLen);
		}

		template<class TYPE>
		void	set_data(TYPE& data)
		{
			resize( sizeof( TYPE));
	#ifdef NEED_NET_TRANS
			data.hton();
	#endif
			memcpy( m_pData+ RESERVE_LEN, &data, m_nLen );
		}

		template<int SIZE>
		void	set_data( const CSizeBuffer<SIZE>& clsBufStruct)
		{
			resize( clsBufStruct.m_nRealLen);
			memcpy( m_pData + RESERVE_LEN, clsBufStruct.m_pData, m_nLen);		
		}
		
		template<class TYPE>
		int32	set_data( const CBufStruct& clsBufStruct, const TYPE* p)
		{
			resize( clsBufStruct.m_nRealLen);
			memcpy( m_pData + RESERVE_LEN, clsBufStruct.m_pData, m_nLen);
			
	#ifdef NEED_NET_TRANS
			TYPE* pTemp = (TYPE*)(m_pData + RESERVE_LEN);
			return pTemp->hton(m_nLen);
	#else
			return 0;
	#endif
		}
		
		void set_data(int32 iValue)
		{
			resize( sizeof( iValue));
	#ifdef NEED_NET_TRANS
			iValue = htonl(iValue);
	#endif
			memcpy( m_pData + RESERVE_LEN, &iValue, m_nLen);
		}
		
		char*	get_data()
		{
			return m_pData + RESERVE_LEN;
		}

		template<class TYPE>
		char*	get_data(TYPE* p)
		{
			TYPE* pType = (TYPE*)(m_pData + RESERVE_LEN);
	#ifdef NEED_NET_TRANS
			pType->ntoh(m_nLen);
	#endif
			return m_pData + RESERVE_LEN;
		}
		
		char*	get_data(int32 iValue)
		{
			int32* pValue = (int32*)(m_pData + RESERVE_LEN);
	#ifdef NEED_NET_TRANS
			*pValue = ntohl(*pValue);
	#endif
			return m_pData + RESERVE_LEN;
		}

		char*	get_head()
		{
			return m_pData + DATAOFF_LEN;
		}

		int	set_head()
		{
			int* pValue = (int*)(m_pData + DATAOFF_LEN);
			m_nType = htonl(*pValue);

			++pValue;
			m_nLen = htonl(*pValue);
			
			++pValue;
			m_nBlock = htonl(*pValue);

			return m_nLen;
		}

		char*	get_recv()
		{
			resize( m_nLen);
			return m_pData + RESERVE_LEN;
		}

		char*	get_send()
		{
			int* pValue = (int*)(m_pData + DATAOFF_LEN);
			*pValue = ntohl( m_nType);

			++pValue;
			*pValue = ntohl( m_nLen);
			
			++pValue;
			*pValue = ntohl( m_nBlock);

			return m_pData + DATAOFF_LEN;
		}
		
		void	n2h()
		{
			m_nType = ntohl( m_nType );
			m_nLen	= ntohl( m_nLen );
			m_nBlock= ntohl( m_nBlock );
		}
		void	h2n()
		{
			m_nType = htonl( m_nType );
			m_nLen	= htonl( m_nLen );
			m_nBlock= htonl( m_nBlock );
		}
		void	ntoh()
		{
			m_nType = ntohl( m_nType );
			m_nLen	= ntohl( m_nLen );
			m_nBlock= ntohl( m_nBlock );
		}
		void	hton()
		{
			m_nType = htonl( m_nType );
			m_nLen	= htonl( m_nLen );
			m_nBlock= htonl( m_nBlock );
		}
		
	private:
		int	m_nType;
		int	m_nLen;		//message length
		int	m_nBlock;
		int	m_nSize;	//memory allocate size
		char*	m_pData;
	};

	#define BEGIN_SOCK_PKG_DESC									\
		int32 _hton(int flag, int32 iLen)						\
		{														\
			int32 iLength = 0;									\
			iLength = AISIZEOF(*this);                        	
	                                                        	
	#define PKG_SOCK_FIELD(field)								\
		switch(flag)											\
		{														\
			case 1:												\
			{													\
				field_hton(field);								\
				break;											\
			}													\
			case 2:												\
			{													\
				field_ntoh(field);								\
				break;											\
			}													\
			default:											\
			{													\
				break;											\
			}													\
		}

	#define PKG_SOCK_ARRAY(array, iNum)							\
		switch(flag)											\
		{														\
			case 1:												\
			{													\
				for(int32 iCount = 0; iCount < iNum; iCount++ )	\
				{												\
					field_hton(array[iCount]);					\
				}												\
				break;											\
			}													\
			case 2:												\
			{													\
				for(int32 iCount = 0; iCount < iNum; iCount++ )	\
				{												\
					field_ntoh(array[iCount]);					\
				}												\
				break;											\
			}													\
			default:											\
			{													\
				break;											\
			}													\
		}

	//包体采用PKG_SOCK_FIELD_LIST打包时 一定跟打包顺序一致。
	//即先CPkgA后CPkgB顺序打包，定义时CPkgA先定义后定义CPkgB
#ifdef NEED_NET_TRANS
#define PKG_SOCK_FIELD_LIST(COUNT, TYPE)								\
	{												\
		if( iLength > iLen )									\
			return -1;									\
		TYPE* p = (TYPE*)((char*)this + iLength);						\
		int32 iTempCount = 0; 									\
		switch(flag)										\
		{											\
			case 1:										\
			{										\
				iTempCount = COUNT;							\
				field_ntoh(iTempCount);							\
				break;									\
			}										\
			case 2:										\
			{										\
				iTempCount = COUNT;							\
				break;									\
			}										\
			default:									\
			{										\
				break;									\
			}										\
		}											\
		if(iTempCount < 0) iTempCount =0;							\
		iLength += iTempCount * AISIZEOF(*p);				\
		if( iLength > iLen )								\
			return -1;										\
		for( int32 iCount = 0; iCount < iTempCount; iCount++ )	\
		{													\
			switch(flag)									\
			{												\
				case 1:										\
				{											\
					p->hton();								\
					break;									\
				}											\
				case 2:										\
				{											\
					p->ntoh();								\
					break;									\
				}											\
				default:									\
				{											\
					break;									\
				}											\
			}												\
			p++;											\
		}													\
	}
#else
	#define PKG_SOCK_FIELD_LIST(COUNT, TYPE)					\
		{														\
			if( iLength > iLen )								\
				return -1;										\
			TYPE* p = (TYPE*)((char*)this + iLength);			\
			int32 iTempCount = ( COUNT >= 0 ? COUNT : 0 );		\
			iLength += iTempCount * AISIZEOF(*p);				\
			if( iLength > iLen )								\
				return -1;										\
			for( int32 iCount = 0; iCount < COUNT; iCount++ )	\
			{													\
				switch(flag)									\
				{												\
					case 1:										\
					{											\
						p->hton();								\
						break;									\
					}											\
					case 2:										\
					{											\
						p->ntoh();								\
						break;									\
					}											\
					default:									\
					{											\
						break;									\
					}											\
				}												\
				p++;											\
			}													\
		}
#endif
	#define END_SOCK_PKG_DESC									\
			return 0;											\
		} 														\
		int32 hton(int32 iLen=0) { return _hton(1, iLen); } 	\
		int32 ntoh(int32 iLen=0) { return _hton(2, iLen); }

	inline int64 ntohl64(const char* pIn )
	{
		if (pIn == NULL)
		{
			return 0;
		}
		
		int32 nIsNet = 0x12345678;
		
		if (*(char *)&nIsNet == 0x12)
		{
			nIsNet = 1;
		}
		else
		{
			nIsNet = 0;
		}
		
		unsigned char szBuffer[8];
		
		if (nIsNet == 1)
		{
			memcpy(szBuffer, pIn, 8);
		}
		else
		{
			//!< 取高4字节.
			int32 nRetH;
			int32 nTmpRetH;
			memcpy(&nTmpRetH, pIn, sizeof(int32));
			nRetH = ntohl(nTmpRetH);
			
			//!< 取低4字节.
			int32 nRetL;
			int32 nTmpRetL;
			memcpy(&nTmpRetL, pIn + sizeof(int32), sizeof(int32));
			nRetL = ntohl(nTmpRetL);
			
			//! 整合.
			memcpy(szBuffer, &nRetL, sizeof(int32));
			memcpy(szBuffer + sizeof(int32), &nRetH, sizeof(int32));
		}
		int64 llValue;
		memcpy(&llValue, szBuffer, sizeof(int64));
		return llValue;
	};

	inline int64 htonl64(const char* pIn )
	{
		if (pIn == NULL)
		{
			return 0;
		}
		
		int32 nIsNet = 0x12345678;
		if (*(char *)&nIsNet == 0x12)
		{
			nIsNet = 1;
		}
		else
		{
			nIsNet = 0;
		}      
		
		unsigned char szBuffer[16];
		
		if (nIsNet == 1)
		{
			memcpy(szBuffer, pIn, 8);
		}
		else
		{
			for (int32 i = 0, j = 7; i < 8; ++i, --j)
			{
				szBuffer[i] = pIn[j];
			}
		}
		int64 llValue;
		memcpy(&llValue, szBuffer, sizeof(int64));
		return llValue;
	};

	inline void field_ntoh(int16& nField)
	{
		nField = ntohs( nField );
	}

	inline void field_ntoh(int32& iField)
	{
		iField = ntohl( iField );
	}

	inline void field_ntoh(char* szField )
	{
		;
	}

	inline void field_ntoh(int64& llField)
	{
		llField = ntohl64( (char*)&llField );
	}

	inline void field_hton(int16& nField)
	{
		nField = htons( nField );
	}

	inline void field_hton(int32& iField)
	{
		iField = htonl( iField );
	}

	inline void field_hton(int64& llField)
	{
		llField = htonl64( (char*)&llField );
	}

	inline void field_hton(char* szField )
	{
		;
	}

}

#endif //_MDB_PROXY_SOCKNOTIFY_

