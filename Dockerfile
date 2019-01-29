FROM yohanpipereau/grpcpp1.12.0:xenial

ARG VPP=19.04-rc0~b6129
ARG HONEYCOMB=1.19.04-6318

ARG BASE=https://packagecloud.io/fdio/master/packages/ubuntu/xenial/

ARG PKG_VPP=${BASE}/vpp_${VPP}_amd64.deb/download.deb
ARG PKG_VPP_LIB=${BASE}/vpp-lib_${VPP}_amd64.deb/download.deb
ARG PKG_VPP_PLUGINS=${BASE}/vpp-plugins_${VPP}_amd64.deb/download.deb
ARG PKG_VPP_DEV=${BASE}/vpp-dev_${VPP}_amd64.deb/download.deb
ARG PKG_HONEYCOMB=${BASE}/honeycomb_${HONEYCOMB}_all.deb/download.deb

COPY . /home/app/

RUN apt-get update && apt-get install -y \
    # Utils
    iproute2 iputils-ping net-tools vim-tiny jshon telnet curl wget ethtool \
    # VPP package dependencies
    libnuma1 libssl1.0.0 libmbedtls10 libmbedx509-0 libboost-system1.58.0 python:any \
    # Honeycomb package dependencies
    openjdk-8-jre-headless \
# Install packages
&& mkdir /tmp/deb && cd /tmp/deb \
&& echo $PKG_VPP_LIB \\n $PKG_VPP_DEV \\n $PKG_VPP \\n $PKG_VPP_PLUGINS \\n $PKG_HONEYCOMB > urls \
&& wget -i urls && dpkg -i *.deb* \
# Reduce image size
&& cd / && rm -rf /var/lib/apt/lists/* /tmp/deb \
# compile my app
&& cd /home/app && make -j$(nproc)

WORKDIR /home/app/
ENTRYPOINT ["./entrypoint.sh"]
