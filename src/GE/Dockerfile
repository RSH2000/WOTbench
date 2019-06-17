FROM ubuntu:16.04
MAINTAINER Raoufeh Hashemian
ENV DEBIAN_FRONTEND noninteractive  
RUN apt-get -qq update -y --fix-missing
RUN apt-get install -y \
collectl \
iperf3 \
net-tools \
inetutils-ping \
sysstat \
ethtool \
autoconf \
make \
pkg-config \
vim \
libtool
RUN mkdir /root/CoAP $$ mkdir /root/CoAP/GE 
COPY ./* /root/CoAP/GE/
COPY ./libcoap /root/CoAP/GE/libcoap/
### Increasing the number of file descriptors
RUN sed -i 's/1024/65536/g' /usr/include/linux/posix_types.h && sed -i 's/1024/65536/g' /usr/include/x86_64-linux-gnu/bits/typesizes.h
RUN cd /root/CoAP/GE/libcoap/libcoap-master  && ./configure --disable-examples && make clean && make &&  make install
ENV  LD_RUN_PATH /usr/local/lib
RUN cd /root/CoAP/GE && make


###ENTRYPOINT ["/root/CoAP/GE/GatewayEmulator"]
