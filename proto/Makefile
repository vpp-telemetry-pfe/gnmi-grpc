CXX = g++
CXXFLAGS += `pkg-config --cflags protobuf grpc` -std=c++11
LDFLAGS += -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl
PROTOC = protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

all: gnmi_ext.pb.o gnmi.pb.o gnmi.grpc.pb.o

%.grpc.pb.cc: %.proto
	$(PROTOC) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

%.pb.cc: %.proto
	$(PROTOC) --cpp_out=. $<

%.grpc.pb.o: %.grpc.pb.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.pb.o: %.pb.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.{o,cc,h}
