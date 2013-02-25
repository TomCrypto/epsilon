CXX = g++
CXXFLAGS = -O3

PROTOBF = $(shell find ./ -name '*.proto')
SOURCES = $(subst proto,pb.cc,$(PROTOBF))
OBJECTS = $(subst proto,pb.o,$(PROTOBF))

default: $(OBJECTS)
	@echo -n

$(SOURCES): %.pb.cc : %.proto
	protoc --cpp_out=. --python_out=../scenes/tools/ $<

$(OBJECTS): %.pb.o : %.pb.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm *.pb.cc
	rm *.pb.h
	rm *.pb.o
