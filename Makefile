CXX=g++
CXXFLAGS=-Wall -Werror -O3 -std=c++11 -g
LDFLAGS=`pkg-config --libs protobuf grpc++ grpc`\
	 -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
	 -ldl -lpthread
LDSTATFLAGS = -L/usr/lib/x86_64-linux-gnu -lvom -lvppapiclient -lvppinfra \
	      -lvlibmemoryclient -lvapiclient

PROTOC=protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

SRC=src
INC=include
BUILD=build
TEST=test
MKDIR_P=mkdir -p
PROTOS_PATH=proto
OBJ=$(SRC)/gnmi_security.o $(SRC)/gnmi_handle_request.o $(SRC)/gnmi_stat.o

proto_obj=proto/gnmi_ext.pb.o proto/gnmi.pb.o proto/gnmi_ext.grpc.pb.o \
	  proto/gnmi.grpc.pb.o

.PHONY: clean all

all: gnmi_server

gnmi_server: $(SRC)/gnmi_server.cpp $(proto_obj) $(OBJ)
	$(info ****** Compile and Link server ******)
	$(MKDIR_P) $(BUILD)
	$(CXX) $^ $(CXXFLAGS) $(LDFLAGS) $(LDSTATFLAGS) -o $(BUILD)/$@

#Requires to be compiled on a machine with VPP installed
gnmi_stat: $(SRC)/gnmi_stat.cpp
	$(MKDIR_P) $(BUILD)
	$(CXX) $^ $(CXXFLAGS) -o $(BUILD)/$@

#Static pattern rule (targets: target-pattern: prereq-patterns)
$(proto_obj): %.pb.o: %.pb.cc
	$(info ****** Compile protobuf generated CPP files ******)
	$(CXX) -c $(CXXFLAGS) $< -o $@

%.grpc.pb.cc: %.proto
	$(info ****** Compile proto to CPP GRPC *****)
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=$(PROTOS_PATH) --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

%.pb.cc: %.proto
	$(info ****** Compile proto to CPP  ******)
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=$(PROTOS_PATH) $<

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -rf $(BUILD) $(PROTOS_PATH)/*.pb.cc $(PROTOS_PATH)/*.pb.h $(PROTOS_PATH)/*.pb.o $(SRC)/*.o
