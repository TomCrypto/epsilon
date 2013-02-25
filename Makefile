EXECUTABLE = epsilon
INCLUDE = -Iinclude/ -Ipb/
CXX = g++

           # The OpenCL C++ wrapper isn't fully 1.2 yet
CXXFLAGS = -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -Wno-cpp \
           -O3 -std=c++11 -march=native \
           -Wall -Wextra -pedantic

HEADERS = $(shell find include/ -name '*.hpp')

PB_OBJS = $(shell find pb/ -name '*.pb.o')
OBJECTS = $(subst cpp,o,$(subst src/,obj/,$(shell find src/ -name '*.cpp')))

LDLIBS = -lOpenCL -lncurses

$(EXECUTABLE): $(OBJECTS)
	@mkdir -p bin/
	@$(CXX) $(OBJECTS) $(PB_OBJS) -o $(addprefix bin/, $(EXECUTABLE)) $(LDLIBS) `pkg-config --cflags --libs protobuf`

$(OBJECTS): obj/%.o : src/%.cpp $(HEADERS)
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

document:
	@doxygen > /dev/null

clean:
	@echo Cleaning $(EXECUTABLE)...
	@rm -f $(addprefix bin/, $(EXECUTABLE))
	@rm -f obj/ --recursive
	@rmdir bin/

