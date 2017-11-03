/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2013 Semtech-Cycleo

Description:
    LoRa concentrator : Packet Forwarder trace helpers

License: Revised BSD License, see LICENSE.TXT file include in the project
Maintainer: Michael Coracin
*/


#ifndef _LORA_PKTFWD_TRACE_H
#define _LORA_PKTFWD_TRACE_H

#define DEBUG_PKT_FWD   0
#define DEBUG_JIT       0
#define DEBUG_JIT_ERROR 1
#define DEBUG_TIMERSYNC 0
#define DEBUG_BEACON    0
#define DEBUG_LOG       1

#define PRINTF_MSG_LEN 1024

static char *pmsg = NULL;
static char *pdbg = NULL;
static char *perr = NULL;

#define MSG(args...) \
	do { \
		pmsg = (char*)malloc(PRINTF_MSG_LEN);\
		memset(pmsg,0,PRINTF_MSG_LEN);\
		sprintf(pmsg,args);\
		LOG_INFO_NPTHD(pmsg);\
		free(pmsg);\
		pmsg=NULL;\
		}\
	while(0)
#define MSG_DEBUG(FLAG, fmt, ...)                                                                         \
            do  {                                                                                         \
                if (FLAG)                                                                                 \
                    fprintf(stdout, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
            } while (0)

#define ERROR(args...) \
	do{\
		perr = (char*)malloc(PRINTF_MSG_LEN);\
		memset(perr,0,PRINTF_MSG_LEN);\
		sprintf(perr,args);\
		LOG_ERROR_NPTHD(perr);\
		free(perr);\
		perr=NULL;\
	}while(0)

#endif
/* --- EOF ------------------------------------------------------------------ */
