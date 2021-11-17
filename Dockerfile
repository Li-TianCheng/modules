FROM ubuntu:latest
MAINTAINER Li-TianCheng "li_tiancheng666@163.com"
RUN apt-get update \
    && apt-get install g++ -y \
    && apt-get install cmake -y \
    && apt-get install libjsoncpp-dev -y \
    && apt-get install libmysqlclient-dev -y \
    && apt-get install libhiredis-dev -y
