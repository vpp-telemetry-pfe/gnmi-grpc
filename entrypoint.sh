#!/bin/bash

set -e

#Run vpp
mkdir /run/vpp
vpp -c /etc/vpp/startup.conf &
while [ ! -S "/run/vpp/stats.sock" -o ! -S "/run/vpp-api.sock" ]; do sleep 1; done

/opt/honeycomb/honeycomb-start &

#On first argument, cut out 1st char starting at 0 and check it's a '-'
if [ "${1:0:1}" = '-' ]; then
    set -- ./build/gnmi_server "$@"
fi

exec "$@"

