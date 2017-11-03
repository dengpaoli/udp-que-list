### Application-specific constants

APP_NAME := lora_pkt_fwd0

### Environment constants 

LGW_PATH ?= ../../lora_gateway/libloragw
ARCH ?=
CROSS_COMPILE ?=

OBJDIR = obj
INCLUDES = $(wildcard inc/*.h)

### External constant definitions
# must get library build option to know if mpsse must be linked or not

include $(LGW_PATH)/library.cfg
RELEASE_VERSION := `cat ../VERSION`

### Constant symbols

CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar
CPP := $(CROSS_COMPILE)g++

CFLAGS := -O2 -Wall -Wextra -std=c99 -Iinc -I.
VFLAG := -D VERSION_STRING="\"$(RELEASE_VERSION)\""
C++FLAGS := -Os -Wall -Wextra

### Constants for Lora concentrator HAL library
# List the library sub-modules that are used by the application

LGW_INC =
ifneq ($(wildcard $(LGW_PATH)/inc/config.h),)
  # only for HAL version 1.3 and beyond
  LGW_INC += $(LGW_PATH)/inc/config.h
endif
LGW_INC += $(LGW_PATH)/inc/loragw_hal.h
LGW_INC += $(LGW_PATH)/inc/loragw_gps.h

### Linking options
ifeq ($(CROSS_COMPILE),) 
OBJLIBS = lib/liblog4cplus.a
LIBS := -lloragw -lrt -lpthread -lm -llog4cplus -lstdc++
else
OBJLIBS = lib/liblog4cplusarm.a
LIBS := -llog4cplusarm -lloragw -lrt -lpthread -lm -lstdc++
endif
### General build targets

all: $(APP_NAME)

clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(APP_NAME)

### Sub-modules compilation

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: src/%.c $(INCLUDES) | $(OBJDIR)
	$(CC) -c $(CFLAGS) -I$(LGW_PATH)/inc $< -o $@
$(OBJDIR)/%.o:src/%.cpp $(INCLUDES) $(OBJLIBS)
	$(CPP) -Iinc -c $(C++FLAGS) $< -o $@ -llog4cplusarm -lrt -lpthread -lm -lstdc++

### Main program compilation and assembly

$(OBJDIR)/$(APP_NAME).o: src/$(APP_NAME).c $(LGW_INC) $(INCLUDES) | $(OBJDIR)
	$(CC) -c $(CFLAGS) $(VFLAG) -I$(LGW_PATH)/inc $< -o $@

$(APP_NAME): $(OBJLIBS) $(OBJDIR)/log4cplus.o $(OBJDIR)/$(APP_NAME).o $(LGW_PATH)/libloragw.a $(OBJDIR)/parson.o $(OBJDIR)/base64.o $(OBJDIR)/jitqueue.o $(OBJDIR)/timersync.o
	$(CC) -L$(LGW_PATH) -Llib -I$(LGW_PATH)/inc -Iinc $< $(OBJDIR)/$(APP_NAME).o $(OBJDIR)/log4cplus.o $(OBJDIR)/parson.o $(OBJDIR)/base64.o $(OBJDIR)/jitqueue.o $(OBJDIR)/timersync.o -o $@ $(LIBS)

### EOF
