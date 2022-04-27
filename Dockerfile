FROM ubuntu:20.04
MAINTAINER Nitrokey <info@nitrokey.com>

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -qy --no-install-recommends \
    libhidapi-dev libusb-1.0-0-dev cmake gcc g++ make doxygen pkg-config alien git graphviz \
    && rm -rf /var/lib/apt/lists/*
RUN mkdir -p /app

WORKDIR /app