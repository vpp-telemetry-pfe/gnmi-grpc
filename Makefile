CXX = g++
CXXFLAGS += `pkg-config --cflags protobuf grpc` -std=c++11
LDFLAGS += -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl
PROTOC = protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`
PROTOS_PATH = proto
SRC=src

vpath %.proto $(PROTOS_PATH)

all: gnmi_server

gnmi_server: $(PROTOS_PATH)/gnmi.pb.o $(PROTOS_PATH)/gnmi.grpc.pb.o gnmi_server.o
	$(CXX) $^ $(LDFLAGS) -o $@

gnmi_server.o: $(SRC)/main.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o gnmi_server
