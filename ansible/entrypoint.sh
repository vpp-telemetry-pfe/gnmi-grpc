#!/bin/bash
echo Waiting for honeycomb to start...
wait-for-it vpp1:2831 -t 300
wait-for-it vpp2:2831 -t 300

if [ "$#" = 0 ]; then
    for playbook in /*.yaml; do
        ansible-playbook $playbook
    done
else
    for playbook in "$@"; do
        ansible-playbook $playbook
    done
fi
