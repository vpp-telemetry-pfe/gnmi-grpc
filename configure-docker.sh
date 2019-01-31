#!/bin/bash

conf_vpp1() {
  docker-compose exec vpp1 curl -k -u admin:admin \
      -X POST -H "Content-Type: application/yang.data+json" \
      -d '{"interface": [{
        "name": "host-eth0",
        "type": "v3po:af-packet",
        "v3po:af-packet": {"host-interface-name": "eth0", "mac": "00:00:00:00:12:10"},
        "ietf-ip:ipv6": {"address": [{"ip": "fd12::10", "prefix-length": 64}]}
      }]}' https://localhost:8445/restconf/config/ietf-interfaces:interfaces
}

conf_vpp2() {
  docker-compose exec vpp2 curl -k -u admin:admin \
      -X POST -H "Content-Type: application/yang.data+json" \
      -d '{"interface": [{
        "name": "host-eth0",
        "type": "v3po:af-packet",
        "v3po:af-packet": {"host-interface-name": "eth0", "mac": "00:00:00:00:12:20"},
        "ietf-ip:ipv6": {"address": [{"ip": "fd12::20", "prefix-length": 64}]}
      }]}' https://localhost:8445/restconf/config/ietf-interfaces:interfaces
}

printf "Configure VPP1-eth0: mac=00:00:00:00:00:12:10 & ip=fd12::10"
conf_vpp1 > /dev/null 2>&1
while [ "$?" = "7" ]; do
  conf_vpp1 > /dev/null 2>&1
done

printf "Configuring VPP2-eth0: mac=00:00:00:00:00:12:20 & ip=fd12::20"
conf_vpp2 > /dev/null 2>&1
while [ "$?" = "7" ]; do
  conf_vpp2 > /dev/null 2>&1
done

printf "\n\nvpp2 pings vpp1 at fd::10\n"
docker-compose exec vpp2 vppctl ping fd12::10
