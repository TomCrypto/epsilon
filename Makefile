EXECUTABLE = epsilon
INCLUDE = -Iinclude/
CXX = g++

OBJECTS = $(subst cpp,o,$(subst src/,obj/,$(shell find src/ -name '*.cpp')))

HEADERS = $(shell find include/ -name '*.hpp')

           # The OpenCL C++ wrapper isn't fully 1.2 yet
CPPFLAGS = -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -Wno-cpp \
           -O2 -std=c++11 -march=native \
           -Wall -Wextra

LDLIBS = -lOpenCL -lncurses

$(EXECUTABLE): $(OBJECTS) 
	@mkdir -p bin/
	@$(CXX) $(OBJECTS) -o $(addprefix bin/, $(EXECUTABLE)) $(LDLIBS)

$(OBJECTS): obj/%.o : src/%.cpp $(HEADERS)
	@mkdir -p $(@D)
	@$(CXX) $(CPPFLAGS) $(INCLUDE) -c $< -o $@

document:
	@doxygen doc/Doxyfile > /dev/null

clean:
	@echo Cleaning $(EXECUTABLE)...
	@rm -f $(addprefix bin/, $(EXECUTABLE))
	@rm -f obj/ --recursive
