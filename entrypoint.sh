#!/bin/bash

set -e

curl -s https://packagecloud.io/install/repositories/fdio/master/script.deb.sh | bash
apt-get update && apt-get install -y vpp vpp-dev

make

#On first argument, cut out 1st char starting at 0 and check it's a '-'
if [ "${1:0:1}" = '-' ]; then
    set -- ./build/gnmi_server "$@"
fi

exec "$@"

