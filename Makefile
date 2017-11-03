### Environment constants 

ARCH ?=
CROSS_COMPILE ?=
export

### general build targets

all:
	$(MAKE) all -e -C semtech0
	$(MAKE) all -e -C semtech1
	$(MAKE) all -e -C shipment
	cp semtech0/packet_forwarder/lora_pkt_fwd/lora_pkt_fwd0 ./
	cp semtech1/packet_forwarder/lora_pkt_fwd/lora_pkt_fwd1 ./
	cp shipment/lora_pkt_shipment ./

clean:
	$(MAKE) clean -e -C semtech0
	$(MAKE) clean -e -C semtech1
	$(MAKE) clean -e -C shipment
	rm lora_pkt_fwd1
	rm lora_pkt_fwd0
	rm lora_pkt_shipment

### EOF
