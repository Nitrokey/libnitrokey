FROM ubuntu:22.04
MAINTAINER Nitrokey <info@nitrokey.com>

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -qy --no-install-recommends \
    libhidapi-dev libusb-1.0-0-dev cmake gcc g++ make doxygen pkg-config alien git graphviz \
    build-essential:native pkg-kde-tools udev gnupg curl devscripts \
    debian-keyring ubuntu-keyring \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /app /root/.gnupg

WORKDIR /app
