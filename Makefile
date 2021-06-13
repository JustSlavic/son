PROJECT = son
CXX = g++
CXX_STANDARD = c++17

INC_DIR = \
	include

CXX_FLAGS = \
	-Wall \
	-Werror \
	-std=$(CXX_STANDARD) \

CXX_FLAGS += $(addprefix -I, $(INC_DIR))


# Settings for debug/release build configurations
ifndef MAKECMDGOALS
	# If nothing is set => no debug info is included in binary, also no flags are set
	SUB_DIR  := plain
else ifeq ($(MAKECMDGOALS),debug)
	# In debug build put debug info into binary, set DEBUG definition
	SUB_DIR  := debug
	CXX_FLAGS += -ggdb3 -DDEBUG
else ifeq ($(MAKECMDGOALS),release)
	# In release build set optimisation level O2, set RELEASE definition
	SUB_DIR  := release
	CXX_FLAGS += -O2 -DRELEASE
else ifeq ($(MAKECMDGOALS),examples)
	SUB_DIR  := debug
	CXX_FLAGS += -ggdb3 -DDEBUG
else
	SUB_DIR  := plain
endif


SOURCES = \
	value \
	parser \


OBJECTS := $(addprefix build/$(SUB_DIR)/, $(addsuffix .o,   $(SOURCES)))
SOURCES := $(addprefix src/,              $(addsuffix .cpp, $(SOURCES)))

PROJECT_LIB := bin/$(SUB_DIR)/lib$(PROJECT).a


# ================= RULES ================= #


# Unconditional rules
.PHONY: prebuild postbuild clean examples


all debug release: prebuild $(PROJECT_LIB) postbuild


prebuild:

postbuild:

clean:
	@find build -type f -name '*.o' -delete
	@find build -type f -name '*.d' -delete
	@rm -fv bin/*/lib$(PROJECT).a
	$(MAKE) -C examples clean

examples: $(PROJECT_LIB)
	$(MAKE) -C examples



-include $(OBJECTS:.o=.d)


$(PROJECT_LIB): $(OBJECTS)
	@mkdir -p $(dir $@)
	ar rcvs $(PROJECT_LIB) $(OBJECTS)


build/$(SUB_DIR)/%.o: src/%.cpp ./Makefile
	@mkdir -p $(dir $@)
	@g++ -MM -MT "$@" $(CXX_FLAGS) $< > build/$(SUB_DIR)/$*.d
	g++ $< -c -o $@ $(CXX_FLAGS)
