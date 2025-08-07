# Target output
OUTPUT = new_demo

# Cross compiler
CXX = riscv64-unknown-linux-gnu-g++

# Native Compiler
CXX_X86 = g++

# Directories
INCLUDE_DIR = ./include
UTILS_DIR   = ./utils
QUERY_DIR   = ./queries
DTL_DIR     = ./dtl/src

# Compilation flags
CXXFLAGS = -I$(INCLUDE_DIR) -I$(DTL_DIR) -O3 -g -D __aarch64__ -std=c++17

# C++ source files
CPP_SOURCES = demo.cpp \
              $(wildcard $(UTILS_DIR)/*.cpp) \
              $(wildcard $(QUERY_DIR)/*.cpp)

# Object files
OBJS = $(CPP_SOURCES:.cpp=.o)

# Default target
all: $(DTL_DIR)/libdtl.a $(DTL_DIR)/frontend.hh $(OUTPUT)

# Link final binary
$(OUTPUT): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -L$(DTL_DIR) -ldtl -o $@

# Compile .cpp files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Ensure DTL is built first (including frontend.hh and libdtl.a)
$(DTL_DIR)/libdtl.a:
	$(MAKE) -C $(DTL_DIR)

$(DTL_DIR)/frontend.hh: $(DTL_DIR)/parser.yy
	$(MAKE) -C $(DTL_DIR)

# Clean everything
clean:
	rm -f $(OUTPUT) $(OBJS)
	$(MAKE) -C $(DTL_DIR) clean
