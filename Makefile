EXECUTABLE = epsilon
INCLUDE = -Iinclude/
CXX = g++

OBJECTS = $(subst cpp,o,$(subst src/,obj/,$(shell find src/ -name '*.cpp')))

HEADERS = $(shell find include/ -name '*.hpp')

CPPFLAGS = -O2 -std=c++11 -march=native
LDLIBS = -lOpenCL

$(EXECUTABLE): $(OBJECTS) 
	@mkdir -p bin/
	@echo Linking $(EXECUTABLE)...
	@$(CXX) $(OBJECTS) -o $(addprefix bin/, $(EXECUTABLE)) $(LDLIBS)

$(OBJECTS): obj/%.o : src/%.cpp $(HEADERS)
	@mkdir -p $(@D)
	@echo Building $@ from $<...
	@$(CXX) $(CPPFLAGS) $(INCLUDE) -c $< -o $@

clean:
	@echo Cleaning $(EXECUTABLE)...
	@rm -f $(addprefix bin/, $(EXECUTABLE))
	@rm -f obj/ --recursive
