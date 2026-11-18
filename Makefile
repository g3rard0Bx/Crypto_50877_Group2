# Build only by default. Select the research compiler family explicitly.
ifeq ($(origin CC),default)
CC := gcc
endif
ifeq ($(origin CXX),default)
CXX := g++
endif
OPT ?= O2
CPPFLAGS += -Ithird_party/libhydrogen -Ithird_party/monocypher -Ithird_party/dudect -Icontrols -Isupport
CFLAGS ?= -g -Wall -Wextra
CXXFLAGS ?= -g -Wall -Wextra
COMPILER := $(notdir $(CC))-$(shell $(CC) -dumpfullversion -dumpversion)
TARGET := $(shell $(CC) -dumpmachine)
BUILD_DIR := build/$(COMPILER)/$(TARGET)/$(OPT)

NAMES := correctness-hydrogen correctness-monocypher memcheck-hydrogen memcheck-monocypher timing-monocypher controls-test controls-timing controls-branch
PROGRAMS := $(addprefix $(BUILD_DIR)/,$(NAMES))
.DEFAULT_GOAL := all
.PHONY: all test memcheck controls-memcheck smoke paths clean FORCE
all: $(PROGRAMS)

# Rebuild when compiler commands or flags change in this output directory.
$(BUILD_DIR)/.flags: FORCE
	@mkdir -p $(@D)
	@printf '%s\n' '$(CC)' '$(CXX)' '$(CPPFLAGS)' '$(CFLAGS)' '$(CXXFLAGS)' '$(OPT)' '$(LDFLAGS)' '$(LDLIBS)' > $@.tmp
	@cmp -s $@.tmp $@ || cp $@.tmp $@
	@rm -f $@.tmp

$(BUILD_DIR)/%.o: %.c $(BUILD_DIR)/.flags
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -$(OPT) -MMD -MP -c $< -o $@
$(BUILD_DIR)/%.o: %.cpp $(BUILD_DIR)/.flags
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -$(OPT) -MMD -MP -c $< -o $@

define program
$(BUILD_DIR)/$(1): $(addprefix $(BUILD_DIR)/,$(2))
	$(CXX) $(CXXFLAGS) -$(OPT) $(LDFLAGS) $$^ $(LDLIBS) -lm -o $$@
endef
HYDROGEN := third_party/libhydrogen/hydrogen.o
MONOCYPHER := third_party/monocypher/monocypher.o
$(eval $(call program,correctness-hydrogen,harnesses/correctness/libhydrogen.o $(HYDROGEN)))
$(eval $(call program,correctness-monocypher,harnesses/correctness/monocypher.o $(MONOCYPHER)))
$(eval $(call program,memcheck-hydrogen,harnesses/memcheck/libhydrogen.o $(HYDROGEN)))
$(eval $(call program,memcheck-monocypher,harnesses/memcheck/monocypher.o $(MONOCYPHER)))
$(eval $(call program,timing-monocypher,harnesses/timing/monocypher.o $(MONOCYPHER)))
$(eval $(call program,controls-test,controls/correctness_memcheck.o controls/targets.o))
$(eval $(call program,controls-timing,controls/timing.o controls/targets.o))
$(eval $(call program,controls-branch,controls/memcheck_branch.o))

VALGRIND ?= valgrind
MEMCHECK := $(VALGRIND) --tool=memcheck --track-origins=yes --error-exitcode=99

test: all
	$(BUILD_DIR)/correctness-hydrogen
	$(BUILD_DIR)/correctness-monocypher
	$(BUILD_DIR)/controls-test

# Ordinary correctness checks must have no Memcheck errors.
memcheck: all
	$(MEMCHECK) $(BUILD_DIR)/correctness-hydrogen
	$(MEMCHECK) $(BUILD_DIR)/correctness-monocypher
	$(MEMCHECK) $(BUILD_DIR)/controls-test

# Positive control must report taint; fixed-work must remain clean.
controls-memcheck: all
	@status=0; $(MEMCHECK) $(BUILD_DIR)/controls-test leaky || status=$$?; test $$status -eq 99
	$(MEMCHECK) $(BUILD_DIR)/controls-test fixed-work
	@status=0; $(MEMCHECK) $(BUILD_DIR)/controls-branch || status=$$?; test $$status -eq 99

smoke: all
	$(BUILD_DIR)/timing-monocypher 1
	$(BUILD_DIR)/controls-timing leaky 1
	$(BUILD_DIR)/controls-timing fixed-work 1

paths:
	@printf '%s\n' '$(BUILD_DIR)'

# Only this configuration's known outputs; retain prior experiment binaries.
OBJECTS := $(addprefix $(BUILD_DIR)/,harnesses/correctness/libhydrogen.o harnesses/correctness/monocypher.o harnesses/memcheck/libhydrogen.o harnesses/memcheck/monocypher.o harnesses/timing/monocypher.o controls/correctness_memcheck.o controls/timing.o controls/targets.o controls/memcheck_branch.o $(HYDROGEN) $(MONOCYPHER))
clean:
	rm -f $(PROGRAMS) $(OBJECTS) $(OBJECTS:.o=.d) $(BUILD_DIR)/.flags $(BUILD_DIR)/.flags.tmp

-include $(OBJECTS:.o=.d)
