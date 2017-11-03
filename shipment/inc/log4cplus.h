#ifndef __LOG4CPLUS_H
#define __LOG4CPLUS_H
#ifdef __cplusplus
extern "C"
{
#endif


#define MAXSIZE 10

typedef enum log4cplus_data_type_e {
    LOG4CPLUS_DATA_TYPE_DEBUG,
    LOG4CPLUS_DATA_TYPE_INFO,
    LOG4CPLUS_DATA_TYPE_WARN,
    LOG4CPLUS_DATA_TYPE_ERROR,
    LOG4CPLUS_DATA_TYPE_FATAL,
}log4cplus_data_type; 

typedef struct log4cplus_data{
	log4cplus_data_type datatype;
	bool isprinted;
	const char *qstring;
}log4cplus_datatype_sq;

typedef struct seqqueue{	
	log4cplus_datatype_sq data[MAXSIZE];	
	int front,rear;
}seq_queue,*seq_pqueue;



extern void log4cplus_init();
extern void LOG_DEBUG(const char *s);
extern void LOG_INFO(const char *s);
extern void LOG_WARN(const char *s);
extern void LOG_ERROR(const char *s);
extern void LOG_FATAL(const char *s);

extern void LOG_DEBUG_NPTHD(const char *s);
extern void LOG_INFO_NPTHD(const char *s);
extern void LOG_WARN_NPTHD(const char *s);
extern void LOG_ERROR_NPTHD(const char *s);
extern void LOG_FATAL_NPTHD(const char *s);

extern bool in_seqqueue(log4cplus_datatype_sq data,seq_pqueue q);
extern bool out_seqqueue(log4cplus_datatype_sq *t,seq_pqueue q);
extern bool is_full_seqqueue(seq_pqueue q);
extern bool is_empty_seqqueue(seq_pqueue q);
extern void thread_log4cplus(void);




#ifdef __cplusplus
}
#endif
#endif
