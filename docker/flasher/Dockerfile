FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
  python3 python3-pip \
  usbutils udev \
  && rm -rf /var/lib/apt/lists/*

COPY docker/flasher/requirements.txt /tmp/requirements.txt
RUN pip3 install --no-cache-dir -r /tmp/requirements.txt

WORKDIR /workspace
ENTRYPOINT ["/bin/bash","-lc"]
