#ifndef LORA_SHIPMENT__
#define LORA_SHIPMENT__

#include <sys/types.h>			
#include <sys/socket.h>
#include <netinet/in.h>

#include "log4cplus.h"
#include "config.h"

#define CN470_RX_WND_2_FREQ 505300000


#define UDPPKT_NODEARRAY_NUM 250
#define NO_PKT 0
#define HAVE_PKT 1
#define SHIPMENT_CONF_FILE "shipment_conf.json"

#define SERVER_ADDRESS_DEFAULT "112.74.163.76"
#define LOCAL_SERVER_ADDRESS_DEFAULT "127.0.0.1"

#define SER_PORT_UP_DEFAULT 1680
#define SER_PORT_DOWN_DEFAULT 1682
#define LOCAL_SER_PORT_DOWN_DEFAULT 2718

#define OK 0
#define ERR -1

typedef enum{
	UP_PUSHDATA_TYPE          = 0x00,
	DOWN_PUSHACK_TYPE          = 0x01,
	UP_PULLDATA_TYPE          = 0x02,	
	DOWN_PULLRESP_TYPE          = 0x03,
	DOWN_PULLACK_TYPE           = 0x04,
	UP_TXACK_TYPE             = 0x05,	
	LORA_TYPE_FULL_NUM				/*total of topic types*/
}LORA_TOPIC_TYPE_E;


typedef struct{
	char ser_ip[16];
	int ser_port_up;
	int ser_port_down;
	char localudpser_ip[16];
	int localudpser_port;
	int localudpser_port1;
	int localSer_fd;
	int localSer_fd1;
	int ser_up_fd;
	int ser_down_fd;
}SHIPM_S;

typedef struct SHIPM_PktId_Node_s{
	//unsigned int up_tmst;
	//unsigned int up_freq;
	unsigned char token_h;
	unsigned char token_l;
	int l_fd;
	struct sockaddr_in clientAddr;
	struct SHIPM_PktId_Node_s *next;
}SHIPM_PktId_Node_S;

#define PKT_QUE_SIZE 50
typedef struct{
	int last;
	SHIPM_PktId_Node_S node[PKT_QUE_SIZE];
}SHIPM_PktId_Que_S;

typedef struct{
	int sx1301;
	unsigned int freq[8];
}SHIPM_SX1301FREQ_S;

typedef enum{
	SX1301_0 = 0,
	SX1301_1 = 1,
	SX1301_BUTT
}SX1301_E;

// SHIPM_PktId_Node_s ;

/*
	enddev -->  server
*/
typedef struct{
	int done;
	int pktLen;
	int s_fd;
	int l_fd;
	struct sockaddr_in clientAddr;
	char pktBuf[1024];
}SHIP_EDUdpPkt_Node_S;

typedef struct{
	int front;
	int rear;
	SHIP_EDUdpPkt_Node_S NodeArray[UDPPKT_NODEARRAY_NUM];
}SHIP_EDUdpPkt_Que_S;

/*
	server  -->  enddev
*/
typedef struct{
	int done;
	int pktLen;
	char pktBuf[1024];
}SHIP_SerJsonPkt_Node_S;

typedef struct{
	int front;
	int rear;
	SHIP_SerJsonPkt_Node_S NodeArray[UDPPKT_NODEARRAY_NUM];
}SHIP_SerJsonPkt_Que_S;


static char *pmsg = NULL;
static char *pdbg = NULL;
static char *perr = NULL;

#define PRINTF_MSG_LEN  1024

#if (1 == SHIPM_MSG)
#define SHIPM_msg(args...) \
	do{\
		SHIPM_PRINT(args);\
		pmsg = (char*)malloc(PRINTF_MSG_LEN);\
		memset(pmsg,0,PRINTF_MSG_LEN);\
		sprintf(pmsg,args);\
		LOG_INFO_NPTHD(pmsg);\
		free(pmsg);\
		pmsg=NULL;\
	}while(0)
#else
#define SHIPM_msg(args...) //
#endif

#if (1 == SHIPM_DBUG)
#define SHIPM_dbg(args...) \
	do{\
		SHIPM_PRINT(args);\
		pdbg = (char*)malloc(PRINTF_MSG_LEN);\
		memset(pdbg,0,PRINTF_MSG_LEN);\
		sprintf(pdbg,args);\
		LOG_DEBUG_NPTHD(pdbg);\
		free(pdbg);\
		pdbg=NULL;\
	}while(0)
#else
#define SHIPM_dbg(args...) //
#endif

#if (1 == SHIPM_ERROR)
#define SHIPM_err(args...) \
	do{\
		SHIPM_PRINT(args);\
		perr = (char*)malloc(PRINTF_MSG_LEN);\
		memset(perr,0,PRINTF_MSG_LEN);\
		sprintf(perr,args);\
		LOG_ERROR_NPTHD(perr);\
		free(perr);\
		perr=NULL;\
	}while(0)
#else
#define SHIPM_err(args...) //
#endif


void Ship_EDUdpPkt_Que_init();
bool Ship_EDUdpPkt_Que_IsFull();
bool Ship_EDUdpPkt_Que_IsEmpty();
bool Ship_EDUdpPkt_Que_In(char *pdata, int len, struct sockaddr_in clientAddr,int sfd,int lfd);
bool Ship_EDUdpPkt_Que_Out(SHIP_EDUdpPkt_Node_S *data);
void Ship_SerJsonPkt_Que_init();
bool Ship_SerJsonPkt_Que_IsFull();
bool Ship_Ship_SerJsonPkt_Que_In(char *pdata, int len);
bool Ship_SerJsonPkt_Que_Out(SHIP_SerJsonPkt_Node_S *data);
//void Ship_PktID_Que_init();
bool Ship_PktID_Que_In(SHIPM_PktId_Node_S node);
bool Ship_PktID_Que_Find(SHIPM_PktId_Node_S *pNode);
int ShipM_Init(SHIPM_S *s, char *conf_file);
SHIPM_S *get_shipM_s();
bool Ship_PktID_Que_isfull();
bool Ship_PktID_Que_isempty();
bool Ship_PktID_Que_init();



#endif
