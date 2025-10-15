# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Wextra -O2 -std=c++17

# Executable name
TARGET := nxp_simtemp_cli

# Source files
SRCS := main.cpp device.cpp

# Object files
OBJS := $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Link executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile each .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJS) $(TARGET)