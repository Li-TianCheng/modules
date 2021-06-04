FROM ubuntu:latest
MAINTAINER Li-TianCheng "li_tiancheng666@163.com"
RUN apt-get update \
    && apt-get install libjsoncpp-dev -y\
    && apt-get install uuid-dev -y
