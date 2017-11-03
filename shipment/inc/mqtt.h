#ifndef MQTT_H
#define MQTT_H


#include <stdint.h>         /* C99 types */
#include <stdbool.h>        /* bool type */

#include "MQTTClient.h"
#include "MQTTAsync.h"

//#inc

/* ---------- */
#define MQTT_PUBFINISH_WAITTIME 1000L
//#define QOS 0 //qos level /*delay short*/
#define QOS 1 //qos level
#define MQTT_PUB_CLEAN_SESSION  true
#define MQTT_SUB_CLEAN_SESSION  true
#define MQTTCLIENT_PERSISTENCE_NONE 1

#define DISCONNET 0
#define CONNECT  1
#define FLAG_OFF  0
#define FLAG_ON  (!FLAG_OFF)


#define MQTT_CLIENT_DISCONNECT -3
#define MQTT_CLIENT_NULL -1


#define BRIG_MQTTPUBQUE_MAXSIZE 500
#define BRIG_MQTTSUBQUE_MAXSIZE 500

#define TOPIC_NAME_LENTH    64

#define MQTT_SYNCHRO_WAIT_TIME  3      /*ms unit*/


#define HOST                "192.168.129.200:1883"
#define USERNAME            "root"
#define PASSWD              "emdoor"
#define TOPIC_PUBLISH       "bridge/publishtest"
#define TOPIC_PUBLISH_DATA  "hello test"

//#define PUBLISHCLIENTID     "publisher"
//#define SUBSCRIBECLENTID    "subscriber"

#define BRIDGE_TOPIC                   		"bridge/+"

#define BRIDGE_TOPIC_UP                		"bridge/up/+"
#define BRIDGE_TOPIC_UP_PULLDATA          	"bridge/up/pulldata"
#define BRIDGE_TOPIC_UP_PUSHDATA         	"bridge/up/pushdata"
#define BRIDGE_TOPIC_UP_TXACK             	"bridge/up/txack"
#define BRIDGE_TOPIC_UP_MQTTPING            "bridge/up/mqttping"


#define BRIDGE_TOPIC_DOWN                   "bridge/down/+"
#define BRIDGE_TOPIC_DOWN_PULLRESP          "bridge/down/pullresp"
#define BRIDGE_TOPIC_DOWN_PUSHLOGFILE       "bridge/down/pushlogfile"
#define BRIDGE_TOPIC_DOWN_REMOTEUPGRADE     "bridge/down/remoteupgrade"
#define BRIDGE_TOPIC_DOWN_MQTTPING          "bridge/down/mqttping"
#define BRIDGE_TOPIC_DOWN_PULLACK           "bridge/down/pullack"
#define BRIDGE_TOPIC_DOWN_PUSHACK           "bridge/down/pushack"

#define GWMG_TOPIC_REQUEST                  		"gateway/manage/request/+/+"
#define GWMG_TOPIC_REQUEST_QUERY            		"gateway/manage/request/query/+"
#define GWMG_TOPIC_REQUEST_QUERY_BLUETOOTH  		"gateway/manage/request/query/bluetooth"
#define GWMG_TOPIC_REQUEST_QUERY_4G        	 		"gateway/manage/request/query/4G"
#define GWMG_TOPIC_REQUEST_QUERY_WIFI       		"gateway/manage/request/query/wifi"
#define GWMG_TOPIC_REQUEST_QUERY_ETHERNET   		"gateway/manage/request/query/ethernet"
#define GWMG_TOPIC_REQUEST_QUERY_PKTFORWARDER       "gateway/manage/request/query/pktForwader"
#define GWMG_TOPIC_REQUEST_QUERY_BRIDGE             "gateway/manage/request/query/bridge"
#define GWMG_TOPIC_REPLY_QUERY              	    "gateway/manage/reply/query/+"
#define GWMG_TOPIC_REPLY_QUERY_BLUETOOTH            "gateway/manage/reply/query/bluetooth"
#define GWMG_TOPIC_REPLY_QUERY_4G                   "gateway/manage/reply/query/4G"
#define GWMG_TOPIC_REPLY_QUERY_WIFI      		    "gateway/manage/reply/query/wifi"
#define GWMG_TOPIC_REPLY_QUERY_ETHERNET             "gateway/manage/reply/query/ethernet"
#define GWMG_TOPIC_REPLY_QUERY_PKTFORWARDER         "gateway/manage/reply/query/pktForwader"
#define GWMG_TOPIC_REPLY_QUERY_BRIDGE               "gateway/manage/reply/query/bridge"

#define GWMG_TOPIC_REQUEST_MODIFY                   "gateway/manage/request/modify/+"
#define GWMG_TOPIC_REQUEST_MODIFY_WIFI              "gateway/manage/request/modify/wifi"
#define GWMG_TOPIC_REQUEST_MODIFY_ETHERNET          "gateway/manage/request/modify/ethernet"
#define GWMG_TOPIC_REQUEST_MODIFY_PKTFORWARDER      "gateway/manage/request/modify/pktForwader"
#define GWMG_TOPIC_REQUEST_MODIFY_BRIDGE            "gateway/manage/request/modify/bridge"
#define GWMG_TOPIC_REPLY_MODIFY                     "gateway/manage/reply/modify/+"
#define GWMG_TOPIC_REPLY_MODIFY_WIFI                "gateway/manage/reply/modify/wifi"
#define GWMG_TOPIC_REPLY_MODIFY_ETHERNET            "gateway/manage/reply/modify/ethernet"
#define GWMG_TOPIC_REPLY_MODIFY_PKTFORWARDER        "gateway/manage/reply/modify/pktForwader"
#define GWMG_TOPIC_REPLY_MODIFY_BRIDGE              "gateway/manage/reply/modify/bridge"

#define GWMG_TOPIC_REQUEST_SYSTEM                   "gateway/manage/request/system/+"
#define GWMG_TOPIC_REQUEST_SYSTEM_REBOOT            "gateway/manage/request/system/reboot"
#define GWMG_TOPIC_REQUEST_SYSTEM_SSHREVPROXY       "gateway/manage/request/system/sshRevProxy"
#define GWMG_TOPIC_REQUEST_SYSTEM_GETLOG            "gateway/manage/request/system/getlog"
#define GWMG_TOPIC_REPLY_SYSTEM                     "gateway/manage/reply/system/+"
#define GWMG_TOPIC_REPLY_SYSTEM_REBOOT              "gateway/manage/reply/system/reboot"
#define GWMG_TOPIC_REPLY_SYSTEM_SSHREVPROXY         "gateway/manage/reply/system/sshRevProxy"
#define GWMG_TOPIC_REPLY_SYSTEM_GETLOG              "gateway/manage/reply/system/getlog"
#define GWMG_TOPIC_REPLY_SYSTEM_PING                "gateway/manage/request/system/ping"

//#define SSHPROXY_TOPIC_REQUEST   "gateway/manage/request/system/sshRevProxy/+"





/*
	topic types that for communication between mqtt client and mqtt server;
*/
typedef enum{
	TYPE_START = 0,
	GWMG_TYPE_REQUEST,               
    GWMG_TYPE_REQUEST_QUERY,
    GWMG_TYPE_REQUEST_QUERY_BLUETOOTH,
	GWMG_TYPE_REQUEST_QUERY_4G,
	GWMG_TYPE_REQUEST_QUERY_WIFI,
	GWMG_TYPE_REQUEST_QUERY_ETHERNET,
	GWMG_TYPE_REQUEST_QUERY_PKTFORWARDER,
	GWMG_TYPE_REQUEST_QUERY_BRIDGE,
	GWMG_TYPE_REPLY_QUERY,
	GWMG_TYPE_REPLY_QUERY_BLUETOOTH,
	GWMG_TYPE_REPLY_QUERY_4G,
	GWMG_TYPE_REPLY_QUERY_WIFI,
	GWMG_TYPE_REPLY_QUERY_ETHERNET,
	GWMG_TYPE_REPLY_QUERY_PKTFORWARDER,
	GWMG_TYPE_REPLY_QUERY_BRIDGE,
	GWMG_TYPE_REQUEST_MODIFY,
	GWMG_TYPE_REQUEST_MODIFY_WIFI,
	GWMG_TYPE_REQUEST_MODIFY_ETHERNET,
	GWMG_TYPE_REQUEST_MODIFY_PKTFORWARDER,
	GWMG_TYPE_REQUEST_MODIFY_BRIDGE,
	GWMG_TYPE_REPLY_MODIFY,
	GWMG_TYPE_REPLY_MODIFY_WIFI,
	GWMG_TYPE_REPLY_MODIFY_ETHERNET,
	GWMG_TYPE_REPLY_MODIFY_PKTFORWARDER,
	GWMG_TYPE_REPLY_MODIFY_BRIDGE,
	GWMG_TYPE_REQUEST_SYSTEM,
	GWMG_TYPE_REQUEST_SYSTEM_REBOOT,
	GWMG_TYPE_REQUEST_SYSTEM_SSHREVPROXY,//       "gateway/manage/request/system/sshRevProxy"
	GWMG_TYPE_REQUEST_SYSTEM_GETLOG,//            "gateway/manage/request/system/getlog"
	GWMG_TYPE_REPLY_SYSTEM,//                     "gateway/manage/reply/system/+"
	GWMG_TYPE_REPLY_SYSTEM_REBOOT,//              "gateway/manage/reply/system/reboot"
	GWMG_TYPE_REPLY_SYSTEM_SSHREVPROXY,//         "gateway/manage/reply/system/sshRevProxy"
	GWMG_TYPE_REPLY_SYSTEM_GETLOG,//              "gateway/manage/reply/system/getlog"
	//SSHPROXY_TYPE_REQUEST,
	GWMG_TYPE_REPLY_SYSTEM_PING ,
	TYPE_FULL_NUM				/*total of topic types*/
}TOPIC_TYPE_E;

typedef enum{
	AUTHEN_TYPE_PASSWD = 0,
	AUTHEN_TYPE_SINGLE_AUTHEFILE = 1,
	AUTHEN_TYPE_PASSWD_SINGLE_AUTHEFILE = 2,
	AUTHEN_TYPE_DOUBLE_AUTHEFILE = 3,
}AUTHEN_TYPE_E;

typedef struct{
	unsigned long long total;
	unsigned long long success;
	unsigned long long fail;
	unsigned long long pushdata;
	unsigned long long pulldata;
	unsigned long long upping;
}type_pub_s;

typedef struct{
	unsigned long long total;
	unsigned long long success;
	unsigned long long fail;
	unsigned long long upping;
}type_sub_s;


typedef struct{
	type_pub_s pub;
	type_sub_s sub;
	unsigned int inpubque;
	double mqttpingsend;
	double mqttpingdelay;
	double mqttpingmax;
	unsigned long long downpingnum;
}mqtt_counter_s;

typedef struct{	 
	TOPIC_TYPE_E type;	 
	char *topic;	 			/*mqtt topic type*/
	char topicData[512];	 		/*mqtt topic content*/
	//TOPIC_NODE_S *next; 
}TOPIC_NODE_S;

/*
	mqtt object struct consists of attributes and methods
*/
typedef struct brig_mqtt_publishdata{
	TOPIC_TYPE_E topicType;
	bool isPublished;
	int len;
	double udprecvtime;
	const char *qData;	
}brig_mqtt_publishdata_s;

typedef struct{
	TOPIC_TYPE_E topicType;
	bool isPublished;
	int len;
	const char qData[4*1024];	
	double udprecvtime;
}brig_mqtt_publishdatacarrior_s;


typedef struct brig_mqtt_seqqueue{	
	brig_mqtt_publishdata_s data[BRIG_MQTTPUBQUE_MAXSIZE]; 
	int front,rear;
}brig_mqtt_seq_queue,*brig_mqtt_seq_pqueue;


typedef struct{
	pthread_t pubThreadId;
	pthread_t subThreadId;
	int authentictype;
	bool isusessl;
	bool cleansession;
	int keepaliveinterval;
	int qos;
	char host[28];      /*mqtt server address*/
	char username[12];  /*mqtt server user name*/
	char passwd[32];	/*mqtt server user password*/
	char authenfile[32];	/*mqtt server authentication file*/
	char pubClientID[24];
	char subClientID[24];
	char topic[TYPE_FULL_NUM][TOPIC_NAME_LENTH];
	double minTimeUdpRecvtoPubDone;
	double maxTimeUdpRecvtoPubDone;
	#ifdef MQTT_A_SYNC
	MQTTAsync clientPub;
	MQTTAsync clientSub;
	#else
	MQTTClient clientPub;
	MQTTClient clientSub;
	#endif
	mqtt_counter_s counter;
	double maxstaytime;
	double minstaytime;
	//double timeinpubque;
		
	/* connect to host and return a MQTT publish client handle*/
	MQTTClient (*createPuber)(char *host, char *clientId, char *user, char *pwd);
	int (*publish)(MQTTClient *client_pt, char *topic, char *payload, int len);
	/**/
	MQTTClient (*createSuber)(char *host, char *clientId, char *topic, char *user, char *pwd);
	/*messageBuf: to get the received data, messageLen: the length of the received data*/
	int (*subscribe)(MQTTClient *client_pt, char *topic, char *messageBuf, int *messageLen);
	int (*unsubscribe)(MQTTClient handle, const char* topic);
	void (*close)(MQTTClient*);	 
	/*if run,will set SSL authentication; privateKeyFile:private key file, host:(IP:port)*/
	void (*authenticate)(char *privateKeyFile, char* host);
	void (*publishthread)(void);
	void (*sublishthread)(void);
	char *(*gettopic)(TOPIC_TYPE_E t);
	bool (*inPubQue)(TOPIC_TYPE_E topicType, char *topicData, int len,double time,brig_mqtt_seq_pqueue q);
	int (*publishQueueInit)(void);
	void (*topicInit)(void);
	double (*getmaxstaytime)(double t1, double t2);
	double (*getminstaytime)(double t1, double t2);
	int (*reconnect)(MQTTClient *client,char *topic, char *user, char *pwd);
	int (*SubMsgProcess)(char *topic, char *messageBuf, int len);
}MQTT_S;

/*
	For mqtt publish topic queue 
*/



extern void setCertification(char *privateKeyFile, char* host);
extern void closeConnection(MQTTClient*);
extern MQTTClient createPublisher(char *host, char *clientId, char *user, char *pwd);
extern int publishMsg(MQTTClient *client_pt, char *topic, char *payload,int len);
extern int startSubscribe(MQTTClient *client_pt, char *topic, char *messageBuf, int *messageLen);
extern MQTTClient createSubscriber(char *host, char *clientId, char *topic, char *user, char *pwd);
extern int mqtt_unsubscribe(MQTTClient handle, const char* topic);
extern char *getTopicStringbyType(TOPIC_TYPE_E t);
extern void Mqtt_Publishthread(void);
extern void Mqtt_Sublishthread(void);
extern bool Mqtt_PublishQueueIn(TOPIC_TYPE_E topicType, char *topicData,int len,double time, brig_mqtt_seq_pqueue q);
extern brig_mqtt_seq_pqueue getMqttPubQue();
extern int Mqtt_PublishQueueInit();
extern TOPIC_TYPE_E getTopicTypebyString(char *s);
extern void Mqtt_TopicInit();
extern void topicshow();
extern double Mqtt_GetMinStayTime(double t1, double t2);
extern double Mqtt_GetMaxStayTime(double t1, double t2);
extern int Mqtt_Reconnect(MQTTClient *client,char *topic, char *user, char *pwd);
extern int asyncPublishMsg(MQTTAsync *client, char *topic, char *payload, int len) ;
extern MQTTAsync createAsyncPublisher(char *host, char *clientId, char *user, char *pwd) ;
extern int AsyncSubscribe(MQTTClient *client, char *topic, char *messageBuf, int *messageLen);
extern MQTTAsync createAsyncSubscriber(char *host, char *clientId, char *topic, char *user, char *pwd);
extern int Mqtt_SubMsgProcess(char *topic, char *messageBuf, int len);
extern void connOptions_configure(MQTTClient_connectOptions *c, AUTHEN_TYPE_E t);
extern int Mqtt_gwMng_process(brig_mqtt_publishdatacarrior_s d);





#endif //MQTT_H
