### Application-specific constants
APP_NAME := lora_pkt_shipment
### Environment constants 
ARCH ?=
CROSS_COMPILE ?=
INCLUDES = -Iinc -Iinc/mqtt
### Constant symbols
CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar
CPP := $(CROSS_COMPILE)g++
C++FILES = $(wildcard src/*.cpp)
CFILES = $(wildcard src/*.c)
OBJFILES = $(CFILES:.c=.o) $(C++FILES:.cpp=.o)
LOG4CPLUS = lib
AA=


### Linking options
ifeq ($(CROSS_COMPILE),) 
OBJLIBS = lib/liblog4cplus.a
LIBS := -llog4cplus -lrt -lpthread -lm -lstdc++ -lpaho-mqtt3cs -lssl -lcrypto -lpaho-mqtt3as
else
OBJLIBS = lib/liblog4cplusarm.a
LIBS := -llog4cplusarm -lrt -lpthread -lm -lstdc++ -lpaho-mqtt3cs-arm -lssl-arm -lcrypto-arm -lpaho-mqtt3as-arm
endif

CFLAGS := -O2 -Wall -Wextra -std=gnu99 -Wunused-variable -v -Wl,-verbose
#-D_POSIX_SOURCE  //gcc-5.3 need this macro Define
#-D__USE_POSIX199309 
#-D__USE_XOPEN2K
C++FLAGS := -Os -Wall -Wextra

### General build targets
.PHONY: all clean install

all:$(APP_NAME)
	@echo “build sucessfully!!! ^_*”
$(APP_NAME): $(OBJFILES) $(OBJLIBS)
	$(CC) -L$(LOG4CPLUS) $(INCLUDES) $(OBJFILES) $(OBJLIBS) -o $@ $(LIBS)

### Sub-modules compilation  -I$(LGW_PATH)/inc

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCLUDES)
	@echo "^^^^^^^^^^^^^^^$(OBJLIBS)^^^^^^^^^^^"
%.o:%.cpp
	$(CPP) -c $(C++FLAGS) $(INCLUDES) $< -o $@ $(LIBS)

clean:
	rm -f src/*.o
	rm -f $(APP_NAME)

install:
	cp $(APP_NAME) /home/zhp/tftpboot
### EOF
