
struct SObVersionInfo
{
	const char* m_szSourceName;
	const char* m_szFullPathName;
	const char* m_szModifyDate;
	const char* m_szFileSize;
	const char* m_szCvsVersion;
	const char* m_szMD5;
	const char* m_szSVNLabel;
};

struct SObLibrariesInfo
{	const char* m_szLibName;
	const char* m_szLibVersionNum;
};

#ifdef __cplusplus
extern "C" {
#endif
static const SObVersionInfo __VERSION_INFO[] =
{
	{ "mdb_proxy_sockapi.cpp", "/app/billbm/work/products/openboss60_new/public/socket_proxy/mdb_proxy_sockapi.cpp", "2017-06-22 11:17:19", "15991" , "" , "77F58D9A8E771BBDB8F7C7C5A9E4DED5",""},
	{ "mdb_proxy_sockapi.h", "/app/billbm/work/products/openboss60_new/public/socket_proxy/mdb_proxy_sockapi.h", "2017-06-22 11:17:19", "2820" , "" , "9B7E728ED7AEBA4226271AA477781872",""},
	{ "mdb_proxy_socknotify.h", "/app/billbm/work/products/openboss60_new/public/socket_proxy/mdb_proxy_socknotify.h", "2017-06-22 11:17:19", "12962" , "" , "8AC45359FC5D56B1C51CF0BBBABF7CF3",""},
	{ 0, 0, 0, 0, 0,0,0 }
};

static const SObLibrariesInfo __LIB_INFO[] = 
{
	{ 0, 0 }
};

const SObVersionInfo* get_verOfAIOpenBossBusinessDll()
{
	return __VERSION_INFO;
}

const SObVersionInfo* get_verOflibpublic_socket_proxyD()
{
	return __VERSION_INFO;
}

const SObLibrariesInfo* get_LibInfoOflibpublic_socket_proxyD()
{
	return __LIB_INFO;
}

//static CObVersionInfoRegister l_cRegister("./build/libpublic_socket_proxyD.so", __VERSION_INFO);

#ifdef __cplusplus
}

#endif
