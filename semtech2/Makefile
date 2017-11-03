### Environment constants 

ARCH ?=
CROSS_COMPILE ?=
export

### general build targets

all:
	$(MAKE) all -e -C lora_gateway
	$(MAKE) all -e -C packet_forwarder

clean:
	$(MAKE) clean -e -C lora_gateway
	$(MAKE) clean -e -C packet_forwarder

### EOF
