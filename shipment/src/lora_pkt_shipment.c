#if __STDC_VERSION__ >= 199901L
    #define _XOPEN_SOURCE 600
#else
    #define _XOPEN_SOURCE 500
#endif
#include <stdlib.h>
#include <sys/types.h>			
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>      /* IP address conversion stuff */
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>         /* C99 types */
#include <stdbool.h>        /* bool type */
#include <string.h>
#include <unistd.h>
#include <time.h>           /* time, clock_gettime, strftime, gmtime */
#include <sys/time.h>  
#include <math.h>
#include <netinet/ip.h> 
#include <asm/socket.h>

#include "lora_pkt_shipment.h"
#include "parson.h"
//#include <linux/socket.h>



static SHIP_EDUdpPkt_Que_S  edUdpPkt_Que = {0};
static SHIP_SerJsonPkt_Que_S  serJsonPkt_Que = {0};
static SHIPM_PktId_Que_S pktId_Que = {0};

SHIPM_PktId_Node_S *pPktId_que_header = NULL;

static pthread_mutex_t mx_shipudpkt_que = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mx_shipserpkt_que = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mx_pktId_que = PTHREAD_MUTEX_INITIALIZER;
#define SX1301_PIECES 2
static SHIPM_SX1301FREQ_S sx1301_freq[SX1301_PIECES];
unsigned int sx1301_freq_0[8],sx1301_freq_1[8];

SX1301_E get_sx1301_from_floatfreq(double f)
{
	double t;
	int i;

	for(i = 0; i < 8; i++)
	{
		t = 1.0 * sx1301_freq_0[i]/1e6;
		//SHIPM_dbg("---sx1301_freq_0 [%d] = %u,  t= %.6lf\n",i,sx1301_freq_0[i],t);		
		if(fabs(30.0+t-f)<0.11)
			return SX1301_0;
		
		t = 1.0 * sx1301_freq_1[i]/1e6;
		//SHIPM_dbg("---sx1301_freq_1 [%d] = %u,  t= %.6lf\n",i,sx1301_freq_1[i],t);	
		if(fabs(30.0+t-f)<0.11)
			return SX1301_1;
	}

	return SX1301_BUTT;
}


void Ship_EDUdpPkt_Que_init()
{
	int i;

	pthread_mutex_lock(&mx_shipudpkt_que);
	edUdpPkt_Que.front = edUdpPkt_Que.rear = UDPPKT_NODEARRAY_NUM -1;
	
	for(i = 0; i < UDPPKT_NODEARRAY_NUM; i ++)
	{
		edUdpPkt_Que.NodeArray[i].done = NO_PKT;
		edUdpPkt_Que.NodeArray[i].pktLen = 0;
		edUdpPkt_Que.NodeArray[i].l_fd = -1;
		edUdpPkt_Que.NodeArray[i].s_fd = -1;
		memset(edUdpPkt_Que.NodeArray[i].pktBuf,0,sizeof edUdpPkt_Que.NodeArray[i].pktBuf);
	}
	pthread_mutex_unlock(&mx_shipudpkt_que);
}

bool Ship_EDUdpPkt_Que_IsFull()
{
	pthread_mutex_lock(&mx_shipudpkt_que);
	if((edUdpPkt_Que.rear+1)%UDPPKT_NODEARRAY_NUM == edUdpPkt_Que.front)
	{
		pthread_mutex_unlock(&mx_shipudpkt_que);
		return true;		
	}
	else
	{		
		pthread_mutex_unlock(&mx_shipudpkt_que);
		return false;
	}
}

bool Ship_EDUdpPkt_Que_IsEmpty()
{	
	pthread_mutex_lock(&mx_shipudpkt_que);
	if(edUdpPkt_Que.rear == edUdpPkt_Que.front)	
	{
		pthread_mutex_unlock(&mx_shipudpkt_que);
		return true;	
	}
	else
	{
		pthread_mutex_unlock(&mx_shipudpkt_que);
		return false;		
	}
}
/*
	int done;
	int pktLen;
	char pktBuf[512];
*/

bool Ship_EDUdpPkt_Que_In(char *pdata, int len, struct sockaddr_in clientAddr,int sfd,int lfd)
{	
	if(Ship_EDUdpPkt_Que_IsFull()){		
		SHIPM_err("Ship_EDUdpPkt_Que_IsFull!\n");		
		return false;	
	}	

	pthread_mutex_lock(&mx_shipudpkt_que);
	edUdpPkt_Que.rear = (edUdpPkt_Que.rear+1)%UDPPKT_NODEARRAY_NUM;	
	edUdpPkt_Que.NodeArray[edUdpPkt_Que.rear].done = HAVE_PKT;
	edUdpPkt_Que.NodeArray[edUdpPkt_Que.rear].pktLen= len;
	edUdpPkt_Que.NodeArray[edUdpPkt_Que.rear].clientAddr = clientAddr;
	edUdpPkt_Que.NodeArray[edUdpPkt_Que.rear].s_fd=sfd;
	edUdpPkt_Que.NodeArray[edUdpPkt_Que.rear].l_fd=lfd;
	memset(edUdpPkt_Que.NodeArray[edUdpPkt_Que.rear].pktBuf, 0,1024);
	memcpy(edUdpPkt_Que.NodeArray[edUdpPkt_Que.rear].pktBuf, pdata, len);
#if 0
	SHIPM_dbg("[Ship_EDUdpPkt_Que_In]: edUdpPkt_Que.node[edUdpPkt_Que.rear].qdata: %d %d %d %d\r\n\r\n",edUdpPkt_Que.NodeArray[edUdpPkt_Que.rear].pktBuf[0],
		edUdpPkt_Que.NodeArray[edUdpPkt_Que.rear].pktBuf[1],edUdpPkt_Que.NodeArray[edUdpPkt_Que.rear].pktBuf[2],
		edUdpPkt_Que.NodeArray[edUdpPkt_Que.rear].pktBuf[3]);
	SHIPM_dbg("[Ship_EDUdpPkt_Que_In]: pdata: %d %d %d %d\r\n\r\n",pdata[0],pdata[1],pdata[2],pdata[3]);
#endif
	pthread_mutex_unlock(&mx_shipudpkt_que);

	return true;
}

bool Ship_EDUdpPkt_Que_Out(SHIP_EDUdpPkt_Node_S *data)
{
	
	if(Ship_EDUdpPkt_Que_IsEmpty()){		
		//SHIPM_err("Ship_EDUdpPkt_Que_IsEmpty!\n");		
		return false;	
	}

	pthread_mutex_lock(&mx_shipudpkt_que);
	edUdpPkt_Que.front = (edUdpPkt_Que.front+1)%UDPPKT_NODEARRAY_NUM;	
	#if 0
	SHIPM_dbg("[Ship_EDUdpPkt_Que_Out]: q->node[q->front].qdata: %d %d %d %d\r\n\r\n",edUdpPkt_Que.NodeArray[edUdpPkt_Que.front].pktBuf[0],
		edUdpPkt_Que.NodeArray[edUdpPkt_Que.front].pktBuf[1],
		edUdpPkt_Que.NodeArray[edUdpPkt_Que.front].pktBuf[2],edUdpPkt_Que.NodeArray[edUdpPkt_Que.front].pktBuf[3]);
	#endif

	data->done = edUdpPkt_Que.NodeArray[edUdpPkt_Que.front].done;
	data->s_fd= edUdpPkt_Que.NodeArray[edUdpPkt_Que.front].s_fd;
	data->l_fd= edUdpPkt_Que.NodeArray[edUdpPkt_Que.front].l_fd;
	data->pktLen = edUdpPkt_Que.NodeArray[edUdpPkt_Que.front].pktLen;
	data->clientAddr= edUdpPkt_Que.NodeArray[edUdpPkt_Que.front].clientAddr;
	memcpy(data->pktBuf,edUdpPkt_Que.NodeArray[edUdpPkt_Que.front].pktBuf,edUdpPkt_Que.NodeArray[edUdpPkt_Que.front].pktLen);
	edUdpPkt_Que.NodeArray[edUdpPkt_Que.front].done= NO_PKT;
	
	memset(edUdpPkt_Que.NodeArray[edUdpPkt_Que.rear].pktBuf, 0,1024);
	edUdpPkt_Que.NodeArray[edUdpPkt_Que.front].pktLen=0;
	memset(&edUdpPkt_Que.NodeArray[edUdpPkt_Que.front].clientAddr, 0,sizeof(struct sockaddr_in));	
	pthread_mutex_unlock(&mx_shipudpkt_que);
	
	return true;
}


/************************************************boarder************************************************************/
void Ship_SerJsonPkt_Que_init()
{
	int i;

	pthread_mutex_lock(&mx_shipserpkt_que);
	serJsonPkt_Que.front = serJsonPkt_Que.rear = UDPPKT_NODEARRAY_NUM -1;
	
	for(i = 0; i < UDPPKT_NODEARRAY_NUM; i ++)
	{
		serJsonPkt_Que.NodeArray[i].done = NO_PKT;
		serJsonPkt_Que.NodeArray[i].pktLen = 0;
		memset(serJsonPkt_Que.NodeArray[i].pktBuf,0,sizeof serJsonPkt_Que.NodeArray[i].pktBuf);
	}
	pthread_mutex_unlock(&mx_shipserpkt_que);
}

bool Ship_SerJsonPkt_Que_IsFull()
{
	pthread_mutex_lock(&mx_shipserpkt_que);
	if((serJsonPkt_Que.rear+1)%UDPPKT_NODEARRAY_NUM == serJsonPkt_Que.front)
	{
		pthread_mutex_unlock(&mx_shipserpkt_que);
		return true;		
	}
	else
	{		
		pthread_mutex_unlock(&mx_shipserpkt_que);
		return false;
	}
}

bool Ship_SerJsonPkt_Que_IsEmpty()
{	
	pthread_mutex_lock(&mx_shipserpkt_que);
	if(serJsonPkt_Que.rear == serJsonPkt_Que.front)	
	{
		pthread_mutex_unlock(&mx_shipserpkt_que);
		return true;	
	}
	else
	{
		pthread_mutex_unlock(&mx_shipserpkt_que);
		return false;		
	}
}

bool Ship_Ship_SerJsonPkt_Que_In(char *pdata, int len)

{	
	if(Ship_SerJsonPkt_Que_IsFull()){		
		SHIPM_err("Ship_SerJsonPkt_Que_IsFull!\n");		
		return false;	
	}	

	pthread_mutex_lock(&mx_shipserpkt_que);
	serJsonPkt_Que.rear = (serJsonPkt_Que.rear+1)%UDPPKT_NODEARRAY_NUM;	
	serJsonPkt_Que.NodeArray[serJsonPkt_Que.rear].done = HAVE_PKT;
	serJsonPkt_Que.NodeArray[serJsonPkt_Que.rear].pktLen= len;
	memset(serJsonPkt_Que.NodeArray[serJsonPkt_Que.rear].pktBuf, 0,sizeof serJsonPkt_Que.NodeArray[serJsonPkt_Que.rear].pktBuf);
	memcpy(serJsonPkt_Que.NodeArray[serJsonPkt_Que.rear].pktBuf, pdata, len);
	#if 0
	SHIPM_dbg("[Ship_EDUdpPkt_Que_In]: edUdpPkt_Que.node[edUdpPkt_Que.rear].qdata: %d %d %d %d\r\n\r\n",serJsonPkt_Que.NodeArray[serJsonPkt_Que.rear].pktBuf[0],
		serJsonPkt_Que.NodeArray[serJsonPkt_Que.rear].pktBuf[1],serJsonPkt_Que.NodeArray[serJsonPkt_Que.rear].pktBuf[2],
		serJsonPkt_Que.NodeArray[serJsonPkt_Que.rear].pktBuf[3]);
	#endif
	//SHIPM_dbg("[Ship_EDUdpPkt_Que_In]: pdata: %d %d %d %d\r\n\r\n",pdata[0],pdata[1],pdata[2],pdata[3]);
	pthread_mutex_unlock(&mx_shipserpkt_que);

	return true;
}

bool Ship_SerJsonPkt_Que_Out(SHIP_SerJsonPkt_Node_S *data)
{
	
	if(Ship_SerJsonPkt_Que_IsEmpty()){		
		//SHIPM_err("Ship_SerJsonPkt_Que_IsEmpty!\n");		
		return false;	
	}

	pthread_mutex_lock(&mx_shipserpkt_que);
	serJsonPkt_Que.front = (serJsonPkt_Que.front+1)%UDPPKT_NODEARRAY_NUM;	
	#if 0
	SHIPM_dbg("[Ship_SerJsonPkt_Que_Out]: q->node[q->front].qdata: %d %d %d %d\r\n\r\n",serJsonPkt_Que.NodeArray[serJsonPkt_Que.front].pktBuf[0],
		serJsonPkt_Que.NodeArray[serJsonPkt_Que.front].pktBuf[1],
		serJsonPkt_Que.NodeArray[serJsonPkt_Que.front].pktBuf[2],serJsonPkt_Que.NodeArray[serJsonPkt_Que.front].pktBuf[3]);
#endif
	data->done = serJsonPkt_Que.NodeArray[serJsonPkt_Que.front].done;
	data->pktLen = serJsonPkt_Que.NodeArray[serJsonPkt_Que.front].pktLen;
	memcpy(data->pktBuf,serJsonPkt_Que.NodeArray[serJsonPkt_Que.front].pktBuf,serJsonPkt_Que.NodeArray[serJsonPkt_Que.front].pktLen);
	serJsonPkt_Que.NodeArray[serJsonPkt_Que.front].done= NO_PKT;
	
	memset(serJsonPkt_Que.NodeArray[serJsonPkt_Que.rear].pktBuf, 0,sizeof serJsonPkt_Que.NodeArray[serJsonPkt_Que.rear].pktBuf);
	serJsonPkt_Que.NodeArray[serJsonPkt_Que.front].pktLen=0;
	
	pthread_mutex_unlock(&mx_shipserpkt_que);
	
	return true;
}

/*************************************************************/
bool Ship_PktID_Que_init()
{
#if 1

	pthread_mutex_lock(&mx_pktId_que);
	pPktId_que_header = (SHIPM_PktId_Node_S*)malloc(sizeof(SHIPM_PktId_Node_S));
	if(NULL == pPktId_que_header)
	{
		SHIPM_err("Ship_PktID_Que_init malloc!\n");	
		pthread_mutex_unlock(&mx_pktId_que);
		return false;
	}

	pPktId_que_header->next = NULL;
	pthread_mutex_unlock(&mx_pktId_que);
#else
	
#endif
	return true;
}

bool Ship_PktID_Que_isfull()
{	
	if(pktId_Que.last == PKT_QUE_SIZE-1)		
		return true;	
	else		
		return false;
}

bool Ship_PktID_Que_isempty()
{	
	if(pktId_Que.last == -1)		
		return true;	
	else		
		return false;
}



bool Ship_PktID_Que_In(SHIPM_PktId_Node_S node)
{
#if 1
	SHIPM_PktId_Node_S *p = NULL;

	p = (SHIPM_PktId_Node_S*)malloc(sizeof(SHIPM_PktId_Node_S));
	if(NULL == p)
	{
		SHIPM_err("Ship_PktID_Que_In malloc!\n");	
		return false;
	}

	p->clientAddr = node.clientAddr;
	p->token_h= node.token_h;
	p->token_l= node.token_l;
	p->l_fd = node.l_fd;
	
	pthread_mutex_lock(&mx_pktId_que);
	p->next = pPktId_que_header->next;
	pPktId_que_header->next = p;
	pthread_mutex_unlock(&mx_pktId_que);
#else
#endif

	return true;
}

bool Ship_PktID_Que_Find(SHIPM_PktId_Node_S *pNode)
{
#if 1
	SHIPM_PktId_Node_S *p,*q;

	pthread_mutex_lock(&mx_pktId_que);
	q = pPktId_que_header;
	p = pPktId_que_header->next;

	for(;p != NULL;p=p->next,q=q->next)
	{
		if(pNode->token_h== p->token_h)
		{
			if(pNode->token_l== p->token_l)
			{
				pNode->clientAddr = p->clientAddr;
				pNode->l_fd = p->l_fd;
				break;
			}
		}
	}

	if(p == NULL)
	{
		pthread_mutex_unlock(&mx_pktId_que);
		return false;
	}
	
	q->next = p->next;
	pthread_mutex_unlock(&mx_pktId_que);
	if(p!=NULL)
		free(p);
	p=NULL;
#else

#endif
	return true;
}

int pase_sx1301conffile_togetfreq(char *conf_file,unsigned int *freq)
{
	JSON_Value *root_val = NULL;
	JSON_Value *str = NULL;
    JSON_Object *conf_obj = NULL;
	int radio_freq_0,radio_freq_1;
	int i,radio,freq_hz;
	char param_name[32];
	
	root_val = json_parse_file_with_comments(conf_file);
    if (root_val == NULL) {
		SHIPM_err("[%s] could not json_parse_file %s, parsing it\n",__FUNCTION__,conf_file);
        return ERR;
    }

	conf_obj = json_object_get_object(json_value_get_object(root_val), "SX1301_conf");
    if (conf_obj == NULL) {
        SHIPM_err("[%s] could not json_parse_file %s, parsing it\n",__FUNCTION__,"SX1301_conf");
        return ERR;
    } else {
        SHIPM_dbg("INFO: %s does contain a JSON object named %s, parsing bridge parameters\n", conf_file, "SX1301_conf");
    }

  	root_val = json_object_get_value(conf_obj, "radio_0"); /* fetch value (if possible) */
    if (json_value_get_type(root_val) != JSONObject) {
        SHIPM_err("INFO: no configuration for radio %i\n", 0);
        return ERR;
    }
	radio_freq_0 = (uint32_t)json_object_dotget_number(conf_obj, "radio_0.freq");

	root_val = json_object_get_value(conf_obj, "radio_1"); /* fetch value (if possible) */
    if (json_value_get_type(root_val) != JSONObject) {
        SHIPM_err("INFO: no configuration for radio %i\n", 1);
        return ERR;
    }
	radio_freq_1 = (uint32_t)json_object_dotget_number(conf_obj, "radio_1.freq");

	for (i = 0; i < 8; ++i) {
        snprintf(param_name, sizeof param_name, "chan_multiSF_%i", i); /* compose parameter path inside JSON structure */
        root_val = json_object_get_value(conf_obj, param_name); /* fetch value (if possible) */
        if (json_value_get_type(root_val) != JSONObject) {
            SHIPM_err("INFO: no configuration for Lora multi-SF channel %i\n", i);
            continue;
        }
  
        snprintf(param_name, sizeof param_name, "chan_multiSF_%i.radio", i);
        radio = (uint32_t)json_object_dotget_number(conf_obj, param_name);
		
        snprintf(param_name, sizeof param_name, "chan_multiSF_%i.if", i);
        freq_hz = (int32_t)json_object_dotget_number(conf_obj, param_name);

		if(radio == 0)
		{
			freq_hz += radio_freq_0; 
		}else if(radio == 1)
		{
			freq_hz += radio_freq_1; 
		}else{}

		freq[i] = freq_hz;
    }

	return OK;
}


int ShipM_Init(SHIPM_S *s, char *conf_file)
{
	JSON_Value *root_val = NULL;
	JSON_Value *str = NULL;
    JSON_Object *conf_obj = NULL;
	char *sx1301_conf0_file = "global_conf0.json";
	char *sx1301_conf1_file = "global_conf1.json";
	//printf("");
	SHIPM_dbg("[%s] start to init\n",__FUNCTION__);
	printf("[%s] start to init\n",__FUNCTION__);
	
	if (access(conf_file, R_OK) != 0) { /* if there is a debug conf, parse only the debug conf */
        SHIPM_err("[%s] could not found configuration file %s, parsing it\n",__FUNCTION__,conf_file);
        return ERR;
    }

	if (access(sx1301_conf0_file, R_OK) != 0) { /* if there is a debug conf, parse only the debug conf */
        SHIPM_err("[%s] could not found configuration file %s, parsing it\n",__FUNCTION__,sx1301_conf0_file);
        return ERR;
    }
	
	if (access(sx1301_conf1_file, R_OK) != 0) { /* if there is a debug conf, parse only the debug conf */
		SHIPM_err("[%s] could not found configuration file %s, parsing it\n",__FUNCTION__,sx1301_conf1_file);
		return ERR;
	}

	printf("[%s] 1\n",__FUNCTION__);
	root_val = json_parse_file_with_comments(conf_file);
    if (root_val == NULL) {
		SHIPM_err("[%s] could not json_parse_file %s, parsing it\n",__FUNCTION__,conf_file);
        return ERR;
    }

	SHIPM_dbg("[%s] start to parse json\n",__FUNCTION__);
	printf("[%s] 2\n",__FUNCTION__);
	
	conf_obj = json_object_get_object(json_value_get_object(root_val), "shipment_conf");
    if (conf_obj == NULL) {
        SHIPM_err("[%s] could not json_parse_file %s, parsing it\n",__FUNCTION__,"shipment_conf");
        return ERR;
    } else {
        SHIPM_dbg("INFO: %s does contain a JSON object named %s, parsing bridge parameters\n", conf_file, "shipment_conf");
    }
	printf("[%s] 3\n",__FUNCTION__);
	memset(s->ser_ip, 0, sizeof s->ser_ip);
	str = json_object_get_string(conf_obj, "server_address"); /* fetch value (if possible) */
    if (str != NULL) {
        strncpy(s->ser_ip, str, sizeof s->ser_ip); 
		SHIPM_dbg("INFO: server_address is configured to \"%s\"\n",s->ser_ip);
    }else{
		strncpy(s->ser_ip, SERVER_ADDRESS_DEFAULT, sizeof SERVER_ADDRESS_DEFAULT);
		SHIPM_dbg("INFO: server_address is configured to default: \"%s\"\n", SERVER_ADDRESS_DEFAULT);
	}
	printf("[%s] 4\n",__FUNCTION__);
	memset(s->localudpser_ip, 0, sizeof s->localudpser_ip);
	str = json_object_get_string(conf_obj, "local_udpserver_address"); /* fetch value (if possible) */
    if (str != NULL) {
        strncpy(s->localudpser_ip, str, sizeof s->localudpser_ip); 
		SHIPM_dbg("INFO: localudpser_ip is configured to \"%s\"\n",s->localudpser_ip);
    }else{
		strncpy(s->ser_ip, LOCAL_SERVER_ADDRESS_DEFAULT, sizeof LOCAL_SERVER_ADDRESS_DEFAULT);
		SHIPM_dbg("INFO: localudpser_ip is configured to default: \"%s\"\n", LOCAL_SERVER_ADDRESS_DEFAULT);
	}
	printf("[%s] 5\n",__FUNCTION__);
	s->ser_port_up = (int)json_object_get_number(conf_obj, "ser_port_up");
	if(s->ser_port_up == 0)
	{
		s->ser_port_up = SER_PORT_UP_DEFAULT;
		SHIPM_dbg("INFO: ser_port_up is configured to default: \"%d\"\n", SER_PORT_UP_DEFAULT);
	}else{
		SHIPM_dbg("INFO: ser_port_up is configured to: \"%d\"\n", s->ser_port_up);
	}
	printf("[%s] 6\n",__FUNCTION__);
	//sleep(5);
	s->ser_port_down = (int)json_object_get_number(conf_obj, "ser_port_down");
	if(s->ser_port_down == 0)
	{
		s->ser_port_down = SER_PORT_DOWN_DEFAULT;
		SHIPM_dbg("INFO: ser_port_up is configured to default: \"%d\"\n", SER_PORT_DOWN_DEFAULT);
	}else{
		SHIPM_dbg("INFO: ser_port_up is configured to: \"%d\"\n", s->ser_port_down);
	}

	
	s->localudpser_port= (int)json_object_get_number(conf_obj, "local_udpserver_port");
	if(s->localudpser_port == 0)
	{
		s->localudpser_port = LOCAL_SER_PORT_DOWN_DEFAULT;
		SHIPM_dbg("INFO: localudpser_port is configured to default: \"%d\"\n", LOCAL_SER_PORT_DOWN_DEFAULT);
	}else{
		SHIPM_dbg("INFO: localudpser_port is configured to: \"%d\"\n", s->localudpser_port);
	}
	json_value_free(root_val);

	s->localudpser_port1 = 6667;
	//TO BE CONTINUED!!!
	root_val = json_parse_file_with_comments(sx1301_conf0_file);
    if (root_val == NULL) {
		SHIPM_err("[%s] could not json_parse_file %s, parsing it\n",__FUNCTION__,sx1301_conf0_file);
        return ERR;
    }
	SHIPM_dbg("[%s] start to parse json %s\n",__FUNCTION__,sx1301_conf0_file);

	int ret;
	ret = pase_sx1301conffile_togetfreq(sx1301_conf0_file,sx1301_freq_0);
	if(ret != OK)
		return ERR;
	SHIPM_dbg("[%s]finished parse json %s\n",__FUNCTION__,sx1301_conf0_file);
	
	ret = pase_sx1301conffile_togetfreq(sx1301_conf1_file,sx1301_freq_1);
	if(ret != OK)
		return ERR;
	SHIPM_dbg("[%s] finished to parse json %s\n",__FUNCTION__,sx1301_conf1_file);
	
	return OK;
}

int ShipM_Socket_init()
{
	int status; 
	struct addrinfo hints, *res, *q; 	
	struct sockaddr_in addr;
	char port[16];
	SHIPM_S *s = get_shipM_s();
	struct timeval push_timeout_half = {0, (100 * 500)}; 
	struct timeval pull_timeout = {0, (200 * 1000)}; 
	printf("[%s] 1\n",__FUNCTION__);
	memset(&hints, 0, sizeof(hints)); 
	memset(port, 0, sizeof port);
	sprintf(port,"%d",s->localudpser_port);
	printf("[%s] 0\n",__FUNCTION__);
	//sleep(3);
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_DGRAM; 
	status = getaddrinfo(s->localudpser_ip,port, &hints, &res);	  
	if (status != 0) { 
		//fprintf(stderr, "getaddrinfo, fail!"); 
		SHIPM_err("[ShipM_Socket_init]: get addrinfo, fail!\n");
		return ERR; 
	}
  printf("[%s] 2\n",__FUNCTION__);
  //sleep(2);
	for (q=res; q!=NULL; q=q->ai_next) 
	{
		s->localSer_fd= socket(q->ai_family, q->ai_socktype,q->ai_protocol);
		if (s->localSer_fd == -1) continue; /* try next field */
		else break; /* success, get out of loop */
	}
	if (q == NULL) 
	{
		SHIPM_err("[ShipM_Socket_init]:s->localSer_fd fail!\n");
		return ERR; 
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(s->localudpser_port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	printf("[%s] 3\n",__FUNCTION__);
  //sleep(2);
	if (bind(s->localSer_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		SHIPM_err("bind local udp server failed\r\n");
		return ERR;
	}
	printf("[%s] 4\n",__FUNCTION__);
  //sleep(2);
	freeaddrinfo(res);
	printf("[%s] 5\n",__FUNCTION__);
	  //sleep(2);
/////////////////////////////////////////////////////////////////////////
	  hints.ai_family = AF_INET; 
	  hints.ai_socktype = SOCK_DGRAM; 
	  status = getaddrinfo(s->localudpser_ip,port, &hints, &res);	
	  if (status != 0) { 
		  //fprintf(stderr, "getaddrinfo, fail!"); 
		  SHIPM_err("[ShipM_Socket_init]: get addrinfo, fail!\n");
		  return ERR; 
	  }
	printf("[%s] 2\n",__FUNCTION__);
	//sleep(2);
	  for (q=res; q!=NULL; q=q->ai_next) 
	  {
		  s->localSer_fd1= socket(q->ai_family, q->ai_socktype,q->ai_protocol);
		  if (s->localSer_fd1 == -1) continue; /* try next field */
		  else break; /* success, get out of loop */
	  }
	  if (q == NULL) 
	  {
		  SHIPM_err("[ShipM_Socket_init]:s->localSer_fd fail!\n");
		  return ERR; 
	  }
  
	  addr.sin_family = AF_INET;
	  addr.sin_port = htons(s->localudpser_port1);
	  addr.sin_addr.s_addr = htonl(INADDR_ANY);
	  //printf("[%s] 3\n",__FUNCTION__);
	//sleep(2);
	  if (bind(s->localSer_fd1, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	  {
		  SHIPM_err("bind local udp server failed\r\n");
		  return ERR;
	  }
	
	  freeaddrinfo(res);
	  printf("[%s] 55\n",__FUNCTION__);


	/////////////////////////////////////////////////////
	memset(&hints, 0, sizeof(hints)); 
	memset(port, 0, sizeof port);
	sprintf(port,"%d",s->ser_port_up);
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_DGRAM; 
	status = getaddrinfo(s->ser_ip, port, &hints, &res);	  
	if (status != 0) { 
		//fprintf(stderr, "getaddrinfo, fail!"); 
		SHIPM_err("[ShipM_Socket_init]: ser_port_up get addrinfo, fail!\n");
		return ERR; 
	}
	printf("[%s] 6\n",__FUNCTION__);
 // sleep(2);
	for (q=res; q!=NULL; q=q->ai_next) 
	{
		s->ser_up_fd= socket(q->ai_family, q->ai_socktype,q->ai_protocol);
		if (s->ser_up_fd == -1) continue; /* try next field */
		else break; /* success, get out of loop */
	}

	printf("[%s] 7\n",__FUNCTION__);
 // sleep(2);
	if (q == NULL) 
	{
		SHIPM_err("[ShipM_Socket_init]:s->localSer_fd fail!\n");
		return ERR; 
	}

	status= connect(s->ser_up_fd, q->ai_addr, q->ai_addrlen);
    if (status!= 0) {
        SHIPM_err("ERROR:  connect ser_up_fd failed!\n");
        return ERR;
    }

	printf("[%s] 8\n",__FUNCTION__);
 // sleep(2);
    freeaddrinfo(res);

	/////////////////////////////////////////////
	memset(&hints, 0, sizeof(hints)); 
	memset(port, 0, sizeof port);
	sprintf(port,"%d",s->ser_port_down);
	
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_DGRAM; 
	status = getaddrinfo(s->ser_ip, port, &hints, &res);	  
	if (status != 0) { 
		//fprintf(stderr, "getaddrinfo, fail!"); 
		SHIPM_err("[ShipM_Socket_init]: ser_port_up get addrinfo, fail!\n");
		return ERR; 
	}
	
	for (q=res; q!=NULL; q=q->ai_next) 
	{
		s->ser_down_fd= socket(q->ai_family, q->ai_socktype,q->ai_protocol);
		if (s->ser_down_fd == -1) continue; /* try next field */
		else break; /* success, get out of loop */
	}
	if (q == NULL) 
	{
		SHIPM_err("[ShipM_Socket_init]:s->localSer_fd fail!\n");
		return ERR; 
	}

	status= connect(s->ser_down_fd, q->ai_addr, q->ai_addrlen);
    if (status!= 0) {
        SHIPM_err("ERROR:  connect ser_up_fd failed!\n");
        return ERR;
    }
    freeaddrinfo(res);

	status = setsockopt(s->ser_up_fd, SOL_SOCKET, SO_RCVTIMEO, (void *)&push_timeout_half, sizeof push_timeout_half);
    if (status != 0) {
        SHIPM_err("ERROR: [up] setsockopt ser_up_fd failed\n");
        return ERR;
    }

	status = setsockopt(s->ser_down_fd, SOL_SOCKET, SO_RCVTIMEO, (void *)&pull_timeout, sizeof pull_timeout);
    if (status != 0) {
        SHIPM_err("ERROR: [up] setsockopt ser_up_fd failed\n");
        return ERR;
    }
#if 1
	status = setsockopt(s->localSer_fd, SOL_SOCKET, SO_RCVTIMEO, (void *)&push_timeout_half, sizeof push_timeout_half);
    if (status != 0) {
        SHIPM_err("ERROR: [up] setsockopt localSer_fd failed\n");
        return ERR;
    }
#endif
#if 0
	bool is = false;
	int report = 1;
	socklen_t len;
	int sb,rb,ret;
	len = sizeof len;
	
	sb =rb = 1024*2;
	setsockopt (s->localSer_fd,SOL_SOCKET,SO_RCVBUF,(void*)&rb,sizeof rb);
	setsockopt (s->localSer_fd,SOL_SOCKET,SO_SNDBUF,(void*)&sb,sizeof sb);

	getsockopt (s->localSer_fd,SOL_SOCKET,SO_RCVBUF,(void*)&rb,&len);
	printf("--------------rb = %d\n",rb);

	getsockopt (s->localSer_fd,SOL_SOCKET,SO_SNDBUF,(void*)&sb,&len);
	printf("--------------sb = %d\n",sb);
	//setsockopt (s->localSer_fd,SOL_SOCKET ,SO_REUSEADDR,(const char*)&is,sizeof(is));
	//setsockopt (s->localSer_fd,SOL_SOCKET,SO_DONTLINGER,(const char*)&is,sizeof(is));
	//setsockopt(s->localSer_fd,SOL_SOCKET, 2, &report, sizeof(report));
#endif		
	return OK;
}

