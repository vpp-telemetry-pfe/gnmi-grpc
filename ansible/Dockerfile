FROM ubuntu:bionic
RUN apt-get update && apt-get install -y ansible python-ncclient wait-for-it && \
        rm -rf /var/lib/apt/lists/* && \
        echo "[local]\nlocalhost ansible_connection=local" > /etc/ansible/hosts
COPY entrypoint.sh *.yaml /
ENTRYPOINT ["/entrypoint.sh"]
