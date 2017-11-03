/*
 *
 *
 * zhp 2017-10-23
 *************************************************/
/* fix an issue between POSIX and C99 */
#if __STDC_VERSION__ >= 199901L
    #define _XOPEN_SOURCE 600
#else
    #define _XOPEN_SOURCE 500
#endif

 
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>      /* IP address conversion stuff */

#include <signal.h>

#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <stdint.h>         /* C99 types */
#include <stdbool.h>        /* bool type */

#include <string.h>         /* memset */
#include <time.h>           /* time, clock_gettime, strftime, gmtime */
#include <sys/time.h>       /* timeval */
#include <errno.h>          /* error messages */
#include <math.h>           /* modf */
#include <assert.h>

#include "log4cplus.h"
#include "MQTTClient.h"
#include "MQTTAsync.h"

#include "util.h"
#include "mqtt.h"
#include "lora_pkt_shipment.h"
#include "parson.h"


volatile bool exit_sig = false; 
volatile bool quit_sig = false; 
static pthread_mutex_t mx_brig_counterprint = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mx_clientAddr_0 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mx_clientAddr_1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_t multi_sx1301_udpser_pId;
static pthread_t sendto_LoRaWANSer_pId;
static pthread_t recvfrom_LoRaWANSer_pIdup;
static pthread_t recvfrom_LoRaWANSer_pIddown;
static pthread_t LoRaWANSer_pktprocess_pId;
static struct sockaddr_in  clientAddr_0;
static struct sockaddr_in  clientAddr_1;




static SHIPM_S shipM_s = {
	0
};

SHIPM_S *get_shipM_s()
{
	return &shipM_s;
}

bool ShipM_sentto_LoRaWANServer(SHIP_EDUdpPkt_Node_S data)
{
	char *p =NULL;
	SHIPM_S *s = get_shipM_s();
	p = data.pktBuf;
#if 0

#endif

	send(data.s_fd, (void *)data.pktBuf, data.pktLen, 0);
	return true;
}

bool ShipM_mark_IdInfo(SHIP_EDUdpPkt_Node_S data)
{
	SHIPM_PktId_Node_S node;
	unsigned char *p = NULL;

	memset(&node, 0, sizeof node);

	node.clientAddr = data.clientAddr;

	p = data.pktBuf;
	
	node.token_h = p[1];
	node.token_l = p[2];
	node.l_fd = data.l_fd;
	
	if(!Ship_PktID_Que_In(node))
	{
		SHIPM_err("[ShipM_mark_IdInfo] Ship_PktID_Que_In failed\r\n");
		return false;
	}

	return true;
}


void ShipM_sendtoLoRaWANSer_thread(void)
{
	SHIPM_S *s = get_shipM_s();
	SHIP_EDUdpPkt_Node_S data;

	SHIPM_dbg("[ShipM_sendtoLoRaWANSer_thread] getting into while loop\n");
	while(!exit_sig && !quit_sig)
	{
		memset(&data.pktBuf, 0, sizeof data.pktBuf);
		if(!Ship_EDUdpPkt_Que_Out(&data))
		{
			//SHIPM_err("[ShipM_sendtoLoRaWANSer_thread] Ship_EDUdpPkt_Que_Out failed\r\n");
			usleep(10000);
		    continue;   
		}
		else
		{
			//1 send to server
			ShipM_sentto_LoRaWANServer(data);
			SHIPM_dbg("send to server:Json up: %s\n",(char *)(data.pktBuf+4));
			SHIPM_dbg("[ShipM_sendtoLoRaWANSer_thread] ShipM_sentto_LoRaWANServer success\n");
			//2 mark the freq tmst clientaddr to IDque
			ShipM_mark_IdInfo(data);
			//SHIPM_dbg("[ShipM_sendtoLoRaWANSer_thread] ShipM_mark_IdInfo success-:----------------------\n");
		}

		//SHIPM_dbg("\n\n******************sendtoLoRaWANSer*******************\n\n");
		//usleep(5000);
	}	
}

void ShipM_recvfromLoRaWANSer_threadup(void)
{
	int ret;
	uint8_t buff[1024];
	SHIPM_S *s = get_shipM_s();

	SHIPM_dbg("[ShipM_recvfromLoRaWANSer_threadup] getting into while loop\n");
	while(!exit_sig && !quit_sig)
	{
		memset(buff, 0, sizeof buff);
		ret = recv(s->ser_up_fd, (void *)buff, (sizeof buff)-1, 0);
		if (ret == -1) 
		{
			usleep(5000);
		    continue;
		}

		(void)Ship_Ship_SerJsonPkt_Que_In(buff, ret);
		if(buff[3] == DOWN_PUSHACK_TYPE)
		{
			SHIPM_dbg("\n\nJson down thread up: push ack received!\n");
		}else
		//SHIPM_dbg("\n\nJson down thread up: %s\n",(char *)(buff + 4));
		SHIPM_dbg("******************recv threadup*******************\n\n");
	}
}

void ShipM_recvfromLoRaWANSer_threaddown(void)
{
	int ret;
	uint8_t buff[1024];
	SHIPM_S *s = get_shipM_s();

	SHIPM_dbg("[ShipM_recvfromLoRaWANSer_threaddown] getting into while loop\n");
	while(!exit_sig && !quit_sig)
	{
		memset(buff, 0, sizeof buff);
		ret = recv(s->ser_down_fd, (void *)buff, (sizeof buff)-1, 0);
		if (ret == -1) 
		{
			usleep(5000);
		    continue;
		}
		
		(void)Ship_Ship_SerJsonPkt_Que_In(buff, ret);
		if(buff[3] == DOWN_PULLACK_TYPE)
		{
			SHIPM_dbg("\n\nJson down thread down: pull ack received!\n");
		}else
		SHIPM_dbg("\n\nJson down thread down, %d: %s\n",buff[3],(char *)(buff + 4));
		//SHIPM_dbg("******************recv down*******************\n\n");
	}
}

#define SEND_SX1301_0  0
#define SEND_SX1301_1  1

void ShipM_LoRaWANSerpktprocess_thread(void)
{
	SHIP_SerJsonPkt_Node_S data;
	SHIPM_PktId_Node_S node;
	unsigned char *p = NULL;
	int ret;
	SHIPM_S *s = get_shipM_s();
	JSON_Value *root_val = NULL;
	JSON_Object *txpk_obj = NULL;
	JSON_Value *val = NULL; 
	double freq;
	SX1301_E styp;
	int l_fd;
	struct sockaddr_in  clientAddr;
	int last_send_sx1301 = -1;
	double t;

	while(!exit_sig && !quit_sig)
	{
		memset(&data, 0, sizeof data);
		memset(&node, 0, sizeof node);
	
		if(!Ship_SerJsonPkt_Que_Out(&data))
		{
			usleep(5000);
		    continue;
		}
		else
		{
			p = data.pktBuf;
			switch(p[3])
			{				
				case DOWN_PULLRESP_TYPE:
					SHIPM_dbg("\n\nPULLRESP: %s\n",(char *)(p + 4));
					root_val = json_parse_string_with_comments((const char *)(p + 4));
					if (root_val == NULL) {
                		SHIPM_err("WARNING: [down] invalid JSON, TX aborted\n");
						continue;
					}

					txpk_obj = json_object_get_object(json_value_get_object(root_val), "txpk");
		            if (txpk_obj == NULL) {
		                SHIPM_err("WARNING: [down] no \"txpk\" object in JSON, TX aborted\n");
		                json_value_free(root_val);
		                continue;
		            }
					
                	val = json_object_get_value(txpk_obj,"freq");
		            if (val == NULL) {
		                SHIPM_err("WARNING: [down] no mandatory \"txpk.freq\" object in JSON, TX aborted\n");
		                json_value_free(root_val);
		                continue;
		            }

					freq = (double)json_value_get_number(val);
					SHIPM_dbg("freq freq = %.6lf\n",freq);

					t = 1.0 * CN470_RX_WND_2_FREQ/1e6;

					if(fabs(t-freq)<0.11)
					{
						//RX2_WIN
						SHIPM_dbg("---------------RX2222, t = %.6lf\n",t);
						if(last_send_sx1301 == SEND_SX1301_0)
						{
							l_fd = s->localSer_fd1;
							pthread_mutex_lock(&mx_clientAddr_1);
							clientAddr = clientAddr_1;
							pthread_mutex_unlock(&mx_clientAddr_1);
						}else if(last_send_sx1301 == SEND_SX1301_1)
						{
							l_fd = s->localSer_fd;
							pthread_mutex_lock(&mx_clientAddr_0);
							clientAddr = clientAddr_0;
							pthread_mutex_unlock(&mx_clientAddr_0);
						}else{}
					}
					else
					{				
						styp = get_sx1301_from_floatfreq(freq);
						if(styp == SX1301_0)
						{
							SHIPM_dbg("---------------SX1301_0 pull resp received\n");
							l_fd = s->localSer_fd;
							pthread_mutex_lock(&mx_clientAddr_0);
							clientAddr = clientAddr_0;
							pthread_mutex_unlock(&mx_clientAddr_0);

							last_send_sx1301 = SEND_SX1301_0;
						}
						else if(styp == SX1301_1)
						{
							SHIPM_dbg("---------------SX1301_1 pull resp received\n");
							l_fd = s->localSer_fd1;
							pthread_mutex_lock(&mx_clientAddr_1);
							clientAddr = clientAddr_1;
							pthread_mutex_unlock(&mx_clientAddr_1);
							last_send_sx1301 = SEND_SX1301_1;
						}else{
							SHIPM_dbg("unknown SX1301 pull resp type\n");
						}
					}
					break;
					
				case DOWN_PUSHACK_TYPE:	
				case DOWN_PULLACK_TYPE:
					node.token_h= p[1];
					node.token_l = p[2];
					if(!Ship_PktID_Que_Find(&node))
					{
						SHIPM_err("[ShipM_multiSx1301UdpSer_thread] -----Ship_PktID_Que_Find failed \n");
						continue;
					}else
					{
						//SHIPM_dbg("[ShipM_multiSx1301UdpSer_thread] -----Ship_PktID Find sucess! \n");
						l_fd = node.l_fd;
						clientAddr = node.clientAddr;
					}
					break;
					
				default:
					//return false;
					break;
			}

			ret = sendto(l_fd,data.pktBuf,data.pktLen,0,
				(struct sockaddr *)&clientAddr,sizeof(clientAddr));
			if(ret < 0)
			{
				SHIPM_err("[ShipM_LoRaWANSerpktprocess_thread] udp sendto localSer_fd failed.\r\n");
				usleep(1000);
				continue;
			}

			SHIPM_dbg("[ShipM_multiSx1301UdpSer_thread] -----sendto localSer_fd  -_- *_* sucess ^_^\r\n");
			
		}

		//SHIPM_dbg("\n\n******************Ser pkt process*******************\n\n");
	}
}



void ShipM_multiSx1301UdpSer_thread(void)
{
	SHIPM_S *s = get_shipM_s();
	int ret; 
	struct sockaddr_in  clientAddr;
	char rBuf[1024] = {0};
	socklen_t temp;
	temp = sizeof(struct sockaddr);
	int i=0;
	int sfd;

	SHIPM_dbg("[ShipM_multiSx1301UdpSer_thread] getting into while loop\n");
	//printf("[ShipM_multiSx1301UdpSer_thread] getting into while loop\n");
	while(!exit_sig && !quit_sig)
	{
		memset(rBuf, 0, sizeof rBuf);
		if(i%12 ==0)
	
		//i++;
		ret = recvfrom(s->localSer_fd, (char *)rBuf, (sizeof rBuf) -1, 0, (struct sockaddr*)&clientAddr, &temp);
		if (!(ret>0))
		{
			//SHIPM_err("[ShipM_multiSx1301UdpSer_thread] recvfrom (error code %d)\r\n",ret);
			//usleep(10000);
		    continue;     
		}
		else
		{ 
			SHIPM_dbg("[ShipM_multiSx1301UdpSer_thread]Json up 0: %s\r\n",(char *)(rBuf + 12));
			//printf("[ShipM_multiSx1301UdpSer_thread] udp recv from Sx1301: %s\r\n",(char *)(rBuf + 12));
			switch(rBuf[3])
			{
				case UP_PUSHDATA_TYPE:
					sfd = s->ser_up_fd;
					break;
		
				case UP_PULLDATA_TYPE:
					sfd = s->ser_down_fd;
					pthread_mutex_lock(&mx_clientAddr_0);
					clientAddr_0 = clientAddr;
					pthread_mutex_unlock(&mx_clientAddr_0);
					break;
					
				case UP_TXACK_TYPE:
					sfd = s->ser_down_fd;
					break;
					
				default:
					return false;
			}
			if(!Ship_EDUdpPkt_Que_In(rBuf, ret, clientAddr,sfd,s->localSer_fd))
			{
				SHIPM_err("[ShipM_multiSx1301UdpSer_thread] Ship_EDUdpPkt_Que_In failed!\n");
			}
			 printf(" ***********************Sx1301_1 Udp Ser thread***********************\n\n");
		}
	}
}

void close_socket_fd()
{
	SHIPM_S *s = get_shipM_s();

	close(s->localSer_fd);
	close(s->ser_down_fd);
	close(s->ser_up_fd);
}

void sig_handler(int sigio) 
{
	/* unsubscribe mqtt topic */
    if (sigio == SIGQUIT) {
        quit_sig = true;		
    } else if ((sigio == SIGINT) || (sigio == SIGTERM)) {
        exit_sig = true;		
    }
	
	if(quit_sig || exit_sig)
	{
		close_socket_fd();
	}
    return;
}

int main()
{	
	struct sigaction sigact;
	int ret;
	printf("[main] start 0\n");
	
	/* configure signal handling */
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigact.sa_handler = sig_handler;
	sigaction(SIGQUIT, &sigact, NULL); /* Ctrl-\ */
	sigaction(SIGINT, &sigact, NULL); /* Ctrl-C */
	sigaction(SIGTERM, &sigact, NULL); /* default "kill" command */

	printf("[main] start\n");
	(void)log4cplus_init();
	printf("[main] start 1\n");
	SHIPM_dbg("[main] log4cplus_init [success]\n");
	printf("[main] start 2\n");
	
	ret = ShipM_Init(get_shipM_s(), SHIPMENT_CONF_FILE);
	if(ret != OK)
	{
		SHIPM_err("[%s] ShipM_Init failed\n",__FUNCTION__);
		return ERR;
	}
	printf("[main] start 3\n");
	SHIPM_dbg("[main] ShipM_Init [success]\n");
	//sleep(2);
	Ship_EDUdpPkt_Que_init();
	printf("[main] start 4\n");
	//sleep(2);
	Ship_SerJsonPkt_Que_init();
	printf("[main] start 5\n");
	//sleep(2);
	
	if(!Ship_PktID_Que_init())
	{
		SHIPM_err("[%s] Ship_PktID_Que_init failed\n",__FUNCTION__);
		return ERR;
	}
	printf("[main] start 6\n");
	//sleep(2);
	ret = ShipM_Socket_init();
	if(ret != OK)
	{
		SHIPM_err("[%s] ShipM_Socket_init failed\n",__FUNCTION__);
		return ERR;
	}
	printf("[main] start 7\n");
	//sleep(2);

	ret = pthread_create( &multi_sx1301_udpser_pId, NULL, (void * (*)(void *))ShipM_multiSx1301UdpSer_thread, NULL);
    if (ret != 0) {
        SHIPM_err("ERROR: [main] impossible to create ShipM_multiSx1301UdpSer_thread thread\n");
        exit(-1);
    }
	SHIPM_dbg("[main] ShipM_multiSx1301UdpSer_thread [success]\n");
	printf("[main] ShipM_multiSx1301UdpSer_thread [success]\n");

	ret = pthread_create( &sendto_LoRaWANSer_pId, NULL, (void * (*)(void *))ShipM_sendtoLoRaWANSer_thread, NULL);
    if (ret != 0) {
        SHIPM_err("ERROR: [main] impossible to create ShipM_sendtoLoRaWANSer_thread thread\n");
        exit(-1);
    }
	SHIPM_dbg("[main] ShipM_sendtoLoRaWANSer_thread [success]\n");

	ret = pthread_create( &recvfrom_LoRaWANSer_pIdup, NULL, (void * (*)(void *))ShipM_recvfromLoRaWANSer_threadup, NULL);
    if (ret != 0) {
        SHIPM_err("ERROR: [main] impossible to create ShipM_recvfromLoRaWANSer_threadup thread\n");
        exit(-1);
    }
	SHIPM_dbg("[main] ShipM_recvfromLoRaWANSer_threadup [success]\n");



	ret = pthread_create( &recvfrom_LoRaWANSer_pIddown, NULL, (void * (*)(void *))ShipM_recvfromLoRaWANSer_threaddown, NULL);
    if (ret != 0) {
        SHIPM_err("ERROR: [main] impossible to create ShipM_recvfromLoRaWANSer_threaddown thread\n");
        exit(-1);
    }
	SHIPM_dbg("[main] ShipM_recvfromLoRaWANSer_threaddown [success]\n");



#if 1

	ret = pthread_create( &LoRaWANSer_pktprocess_pId, NULL, (void * (*)(void *))ShipM_LoRaWANSerpktprocess_thread, NULL);
    if (ret != 0) {
        SHIPM_err("ERROR: [main] impossible to create ShipM_recvfromLoRaWANSer_threaddown thread\n");
        exit(-1);
    }
	SHIPM_dbg("[main] ShipM_recvfromLoRaWANSer_threaddown [success]\n");
	//printf("[main] ShipM_recvfromLoRaWANSer_threaddown [success]\n");
#endif

	pthread_detach(multi_sx1301_udpser_pId);
	pthread_detach(sendto_LoRaWANSer_pId);
	pthread_detach(recvfrom_LoRaWANSer_pIdup);
	pthread_detach(recvfrom_LoRaWANSer_pIddown);
	pthread_detach(LoRaWANSer_pktprocess_pId);

	struct sockaddr_in  clientAddr;
	char rBuf[1024] = {0};
	socklen_t temp;
	temp = sizeof(struct sockaddr);
	SHIPM_S *s = get_shipM_s();
	int sfd;
	
	while(!exit_sig && !quit_sig)
	{
	#if 1
		memset(rBuf, 0, sizeof rBuf);
		ret = recvfrom(s->localSer_fd1, (char *)rBuf, (sizeof rBuf) -1, 0, (struct sockaddr*)&clientAddr, &temp);
		if (!(ret>0))
		{
			//SHIPM_err("[ShipM_multiSx1301UdpSer_thread] recvfrom (error code %d)\r\n",ret);
			//usleep(10000);
		    continue;     
		}
		else
		{ 
			SHIPM_dbg("[main] Json up 1: %s\r\n",(char *)(rBuf + 12));
			//printf("[ShipM_multiSx1301UdpSer_thread] udp recv from Sx1301: %s\r\n",(char *)(rBuf + 12));			
			switch(rBuf[3])
			{
				case UP_PUSHDATA_TYPE:
					sfd = s->ser_up_fd;
					break;
		
				case UP_PULLDATA_TYPE:
					sfd = s->ser_down_fd;
					pthread_mutex_lock(&mx_clientAddr_1);
					clientAddr_1 = clientAddr;
					pthread_mutex_unlock(&mx_clientAddr_1);
					break;
					
				case UP_TXACK_TYPE:
					sfd = s->ser_down_fd;
					break;
					
				default:
					return false;
			}
			
			if(!Ship_EDUdpPkt_Que_In(rBuf, ret, clientAddr,sfd,s->localSer_fd1))
			{
				SHIPM_err("[ShipM_multiSx1301UdpSer_thread] Ship_EDUdpPkt_Que_In failed!\n");
			}
			//printf(" ***********************Sx1301_0 Udp Ser thread***********************\n\n");
		}
		 
	#else
		sleep(10);
	#endif
	}
	
    return 0;
}
