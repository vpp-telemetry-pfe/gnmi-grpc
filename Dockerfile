FROM yohanpipereau/grpcpp:1.12.0

COPY . /home/app/
WORKDIR /home/app/
ENTRYPOINT ["./entrypoint.sh"]
