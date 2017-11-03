#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>
#include <iomanip>
#include <log4cplus/logger.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/layout.h>
#include <time.h>
#include <string.h>

#include <unistd.h>
#include "log4cplus.h"
using namespace std;
using namespace log4cplus;
#ifdef __cplusplus
extern "C"
{
#endif
static Logger logger = Logger::getInstance("log");
static seq_pqueue log4cplus_que =NULL;

static pthread_mutex_t mx_log4cplus_queue = PTHREAD_MUTEX_INITIALIZER;


extern volatile bool exit_sig;
extern volatile bool quit_sig;


void log4cplus_init()
{
	int i=0;
	
	Logger root = Logger::getRoot();
	PropertyConfigurator::doConfigure("./config0.properties");

	pthread_mutex_lock(&mx_log4cplus_queue);
	
	log4cplus_que = (seq_pqueue)malloc(sizeof(seq_queue));	
	if(NULL == log4cplus_que){		
		perror("malloc");
		pthread_mutex_unlock(&mx_log4cplus_queue);
		exit(1);	
	}	
	log4cplus_que->front = log4cplus_que->rear = MAXSIZE -1;

	for(;i<MAXSIZE;i++)
	{
		log4cplus_que->data[i].isprinted = true;
		log4cplus_que->data[i].qstring = NULL;
	}
	
	pthread_mutex_unlock(&mx_log4cplus_queue);
}

bool in_seqqueue(log4cplus_datatype_sq data,seq_pqueue q)
{	
	if(is_full_seqqueue(q)){		
		printf("log4cplus queue is empty!\n");		
		return false;	
	}	

	pthread_mutex_lock(&mx_log4cplus_queue);
	q->rear = (q->rear+1)%MAXSIZE;	
	q->data[q->rear] = data;
	pthread_mutex_unlock(&mx_log4cplus_queue);
	
	return true;
}

bool out_seqqueue(log4cplus_datatype_sq *t,seq_pqueue q)
{	
	if(is_empty_seqqueue(q)){		
		printf("log4cplus queue is empty!\n");		
		return false;	
	}

	pthread_mutex_lock(&mx_log4cplus_queue);
	q->front = (q->front+1)%MAXSIZE;	
	*t = q->data[q->front];	
	q->data[q->front].isprinted = true;
	q->data[q->front].qstring = NULL;
	
	pthread_mutex_unlock(&mx_log4cplus_queue);
	
	return true;
}

bool is_full_seqqueue(seq_pqueue q)
{	
	pthread_mutex_lock(&mx_log4cplus_queue);
	if((q->rear+1)%MAXSIZE == q->front)
	{
		pthread_mutex_unlock(&mx_log4cplus_queue);
		return true;		
	}
	else
	{		
		pthread_mutex_unlock(&mx_log4cplus_queue);
		return false;
	}
}

bool is_empty_seqqueue(seq_pqueue q)
{	
	pthread_mutex_lock(&mx_log4cplus_queue);
	if(q->rear == q->front)	
	{
		pthread_mutex_unlock(&mx_log4cplus_queue);
		return true;	
	}
	else
	{
		pthread_mutex_unlock(&mx_log4cplus_queue);
		return false;		
	}
}


void LOG_DEBUG(const char *s)
{
	printf("+++++++++++++++++LOG_DEBUG\n");
	log4cplus_datatype_sq data;

	if(is_full_seqqueue(log4cplus_que))
	{
		printf("LOG_DEBUG: is_full_seqqueue");
		return;
	}
	data.datatype = LOG4CPLUS_DATA_TYPE_DEBUG;
	data.isprinted = false;
	data.qstring = s;

	if(!in_seqqueue(data,log4cplus_que))
	{
		printf("LOG_DEBUG: in_sequeue failed\n");
		return;
	}
	//LOG4CPLUS_DEBUG(logger, s);
}

void LOG_INFO(const char *s)
{
	log4cplus_datatype_sq data;
	printf("+++++++++++++++++LOG_INFO\n");
	if(is_full_seqqueue(log4cplus_que))
	{
		printf("LOG_INFO: is_full_seqqueue");
		return;
	}
	data.datatype = LOG4CPLUS_DATA_TYPE_INFO;
	data.isprinted = false;
	data.qstring = s;
	printf("+++++++++++++++++LOG_INFO 1\n");
	if(!in_seqqueue(data,log4cplus_que))
	{
		printf("LOG_INFO: in_sequeue failed\n");
		return;
	}

	printf("+++++++++++++++++LOG_INFO 2\n");
	//LOG4CPLUS_INFO(logger, s);
}

void LOG_WARN(const char *s)
{
	printf("+++++++++++++++++LOG_WARN\n");

	log4cplus_datatype_sq data;

	if(is_full_seqqueue(log4cplus_que))
	{
		printf("LOG_WARN: is_full_seqqueue");
		return;
	}
	data.datatype = LOG4CPLUS_DATA_TYPE_WARN;
	data.isprinted = false;
	data.qstring = s;

	if(!in_seqqueue(data,log4cplus_que))
	{
		printf("LOG_WARN: in_sequeue failed\n");
		return;
	}
	//LOG4CPLUS_WARN(logger, s);
}

void LOG_ERROR(const char *s)
{
	printf("+++++++++++++++++LOG_ERROR\n");

	log4cplus_datatype_sq data;

	if(is_full_seqqueue(log4cplus_que))
	{
		printf("LOG_ERROR: is_full_seqqueue");
		return;
	}
	data.datatype = LOG4CPLUS_DATA_TYPE_ERROR;
	data.isprinted = false;
	data.qstring = s;

	if(!in_seqqueue(data,log4cplus_que))
	{
		printf("LOG_ERROR: in_sequeue failed\n");
		return;
	}
	//LOG4CPLUS_ERROR(logger, s);
}

void LOG_FATAL(const char *s)
{
	printf("+++++++++++++++++LOG_FATAL\n");

	log4cplus_datatype_sq data;

	if(is_full_seqqueue(log4cplus_que))
	{
		printf("LOG_FATAL: is_full_seqqueue");
		return;
	}
	data.datatype = LOG4CPLUS_DATA_TYPE_FATAL;
	data.isprinted = false;
	data.qstring = s;

	if(!in_seqqueue(data,log4cplus_que))
	{
		printf("LOG_FATAL: in_sequeue failed\n");
		return;
	}
	//LOG4CPLUS_FATAL(logger, s);
}

void LOG_DEBUG_NPTHD(const char *s)
{
	LOG4CPLUS_DEBUG(logger, s);
}

void LOG_INFO_NPTHD(const char *s)
{
	LOG4CPLUS_INFO(logger, s);
}

void LOG_WARN_NPTHD(const char *s)
{
	LOG4CPLUS_WARN(logger, s);
}

void LOG_ERROR_NPTHD(const char *s)
{
	LOG4CPLUS_ERROR(logger, s);
}

void LOG_FATAL_NPTHD(const char *s)
{
	LOG4CPLUS_FATAL(logger, s);
}

void thread_log4cplus(void)
{
	log4cplus_datatype_sq data;
	
	//printf("+++++++++++++++++thread_log4cplus\n");
	while (!exit_sig && !quit_sig) 
	{
		usleep(10*1000);
		if(is_empty_seqqueue(log4cplus_que))
		{
			//printf("+++++++++++++++++is_empty_seqqueue\n");
			usleep(1000*1000);
		}
		else
		{
			while(!is_empty_seqqueue(log4cplus_que))
			{
				memset(&data,0,sizeof(data));
				out_seqqueue(&data,log4cplus_que);
				printf("+++++++++++++++++ full seqqueue\n");
				if(data.isprinted == false)
				{
					switch(data.datatype)
					{
						case LOG4CPLUS_DATA_TYPE_DEBUG:
							LOG4CPLUS_DEBUG(logger, data.qstring);
							break;

						case LOG4CPLUS_DATA_TYPE_INFO:
							LOG4CPLUS_INFO(logger, data.qstring);
							break;

						case LOG4CPLUS_DATA_TYPE_WARN:
							LOG4CPLUS_WARN(logger, data.qstring);
							break;

						case LOG4CPLUS_DATA_TYPE_ERROR:
							LOG4CPLUS_ERROR(logger,data.qstring);
							break;

						case LOG4CPLUS_DATA_TYPE_FATAL:
							LOG4CPLUS_FATAL(logger, data.qstring);
							break;
							
						default:
							LOG4CPLUS_WARN(logger, "unknown log4plus data type");
							break;
					}
				}
			}//while
		}//else		
	}//while
}






#ifdef __cplusplus
}
#endif
