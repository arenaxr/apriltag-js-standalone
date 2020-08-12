
# adapted from https://github.com/pantuza/c-project-template

BINARY=atagjs_example

# Source code directory structure
BINDIR := bin
SRCDIR := src
LOGDIR := log
APRILTAG := apriltag
TESTDIR := test
WASMDIR := html

# Defines the C Compiler
CC := gcc

# Defines the language standards for GCC
STD := -std=gnu99 # See man gcc for more options

# Protection for stack-smashing attack
STACK := -fstack-protector-all -Wstack-protector

# Specifies to GCC the required warnings
WARNS := -Wall -Wextra -pedantic # -pedantic warns on language standards

# Flags for compiling
CFLAGS := -O3 $(STD) $(STACK) $(WARNS)

# Debug options
DEBUG := -g3 -DDEBUG=1

# Dependency libraries
LIBS := -lm  -lpthread -I../$(APRILTAG)
APRILTAGS := -lm  -lpthread -I$(APRILTAG)

# Test libraries
TEST_LIBS := -l cmocka -L /usr/lib

# Tests binary file
TEST_BINARY := $(BINARY)_test_runner

# valgrind test arguments
VALGRIND_TEST_ARGS := test/tag-imgs/* test/tag-imgs/tag36h11_all/*

# all source files except binary source
SRCS := $(shell ls $(SRCDIR)/*.c | grep -v -e $(SRCDIR)/$(BINARY).c )
OBJS := $(SRCS:%.c=%.o)

# remove pywrap and unnecessary tag families
APRILTAG_SRCS := $(shell ls $(APRILTAG)/*.c $(APRILTAG)/common/*.c | grep -v -e apriltag_pywrap.c -e tagCircle49h12.c -e tagCustom48h12.c -e tagStandard52h13.c)
APRILTAG_OBJS := $(APRILTAG_SRCS:%.c=%.o)

# test sources: all source files in test except main
TEST_SRCS := $(shell ls $(TESTDIR)/*.c | grep -v -e $(TESTDIR)/main.c )

#
# COMPILATION RULES
#

default: $(BINARY)

all: $(BINARY) apriltag_wasm.js

# Help message
help:
	@echo "Target rules:"
	@echo "    all      - Builds the example binary (atagjs_example) and the WASM files (apriltag_wasm.js)"
	@echo "    tests    - Compiles with cmocka and run tests binary file"
	@echo "    valgrind - Runs binary file using valgrind tool"
	@echo "    clean    - Clean the project by removing binaries"
	@echo "    help     - Prints a help message with target rules"

# Rule for link and generate the binary file
$(BINARY): $(APRILTAG_OBJS) $(OBJS) $(SRCDIR)/$(BINARY).o
	$(warning in all)
	$(CC) -o $(BINDIR)/$(BINARY) $^ $(DEBUG) $(CFLAGS) $(LIBS)
	@echo -en "\n--\nBinary file placed at" \
			  "$(BINDIR)/$(BINARY)\n";

# Rule for object binaries compilation
$(APRILTAG)/%.o: $(APRILTAG)/%.c
	$(warning in apriltag)
	$(CC) -c $^ -o $@ $(DEBUG) $(CFLAGS) $(APRILTAGS) -lpthread

# Rule for object binaries compilation
%.o: %.c
		$(CC) -c $^ -o $@ $(DEBUG) $(CFLAGS) $(APRILTAGS) -lpthread

# Rule for run valgrind tool
valgrind:
	valgrind \
		--track-origins=yes \
		--leak-check=full \
		--leak-resolution=high \
		--log-file=$(LOGDIR)/$@.log \
		$(BINDIR)/$(BINARY) $(VALGRIND_TEST_ARGS)
	@echo -en "\nCheck the log file: $(LOGDIR)/$@.log\n"

# Compile tests and run the test binary
tests: $(APRILTAG_OBJS) $(OBJS) $(TEST_SRCS)
	@echo -en "CC ";
	$(CC) $(TESTDIR)/main.c -o $(BINDIR)/$(TEST_BINARY) $^ $(DEBUG) $(CFLAGS) $(LIBS) $(TEST_LIBS) -I$(SRCDIR)
	@which ldconfig && ldconfig -C /tmp/ld.so.cache || true # caching the library linking
	@echo -en " Running tests: ";
	./$(BINDIR)/$(TEST_BINARY)

apriltag_wasm.js: $(APRILTAG_SRCS) $(SRCS)
	emcc -Os -s MODULARIZE=1 -s 'EXPORT_NAME="AprilTagWasm"' -s WASM=1 -Iapriltag -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_FUNCTIONS="['_free']" -s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap", "getValue", "setValue"]' -o $(WASMDIR)/$@ $^

docs:
	doxygen

# Rule for cleaning the project
clean:
	@rm -rvf ./$(BINDIR)/* ./$(APRILTAG)/*.o ./$(SRCDIR)/*.o ./$(LOGDIR)/*;
