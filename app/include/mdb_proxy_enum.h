#ifndef __MDB_PROXY_ENUM_H__
#define __MDB_PROXY_ENUM_H__

#define MAX_THREAD_NUM  200

#define DATE_CHANGE_NUM         1000000LL
#define TIME_MAX_NUM            235959LL
enum MdbProxyCommonErrors
{
	MDB_RPOXY_SUCCESS 			= 	0,					//�ɹ�
	MDB_RPOXY_ERROR  			= 	308900,				//ʧ��
	MDB_PROXY_CFG_ERROR,								//���ó���
	MDB_PROXY_TRHEAD_INIT_ERROR , 						//���̳߳�ʼ������
	MDB_PROXY_TRHEAD_DETACH_ERROR, 			
	MDB_PROXY_TRHEAD_SETSTACK_ERROR, 					//�����߳�ջ��Сʧ��
	MDB_PROXY_TRHEAD_GETSTACK_ERROR, 					//��ȡ�߳�ջ��Сʧ��
	MDB_PROXY_TRHEAD_LISTEN_ERROR,   					//��������ʧ��
	MDB_PROXY_TRHEAD_CREATE_ERROR,   					//�����߳�ʧ��
	MDB_PROXY_QUEUE_FULL , 								//���������
	MDB_RPOXY_EXCEPTION, 								//�쳣
	MDB_RPOXY_ABMMDB_EXCEPTION, 						//��ѯABM MDB�쳣
	MDB_RPOXY_APSMDB_EXCEPTION, 						//��ѯAPS MDB�쳣
	MDB_PROXY_SEND_NOTIFY_ERROR,   						//������Ϣʧ��
	MDB_PROXY_RECV_NOTIFY_ERROR,   						//������Ϣʧ��
	MDB_PROXY_BASE_DATA_ERROR,							//���л������ݴ���
	MDB_PROXY_PKGLEN_ERROR ,							//�����С����ȷ
	MDB_PROXY_NOTIFY_ERROR,								//��Ϣ������ȷ
	MDB_RPOXY_IVRMDB_EXCEPTION, 						//��ѯIVR MDB�쳣

	MDB_PROXY_NOT_FIND_BILL  = 408900					//δ�ҵ���Ӧ���˵�
};
#endif
