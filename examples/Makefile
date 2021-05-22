NAME = son
CXX = g++
CXX_STANDARD = c++17

INC_DIR = \
	../include

CXX_FLAGS = \
	-Wall \
	-Werror \
	-std=$(CXX_STANDARD) \

CXX_FLAGS += $(addprefix -I, $(INC_DIR))


.PHONY: all

OUT_DIR = ../bin/examples

all:
	@mkdir -p $(OUT_DIR)
	g++ example_1.cpp ../bin/release/libson.a -o $(OUT_DIR)/example_1 $(CXX_FLAGS)
