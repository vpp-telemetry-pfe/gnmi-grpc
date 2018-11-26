FROM grpc/cxx:1.12.0

COPY . /home/app/
WORKDIR /home/app/
RUN make
CMD ["./build/gnmi_server"]
