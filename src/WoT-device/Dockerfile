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
RUN mkdir /root/CoAP $$ mkdir /root/CoAP/Server 
COPY ./* /root/CoAP/Server/
COPY ./libcoap /root/CoAP/Server/libcoap/
### Increasing the number of file descriptors
RUN sed -i 's/1024/65536/g' /usr/include/linux/posix_types.h && sed -i 's/1024/65536/g' /usr/include/x86_64-linux-gnu/bits/typesizes.h
RUN cd /root/CoAP/Server/libcoap/libcoap-master  && make clean && ./configure --disable-examples && make && make install
ENV  LD_RUN_PATH /usr/local/lib
RUN cd /root/CoAP/Server && make
### ENTRYPOINT ["/root/CoAP/Server/CDB-server"]
