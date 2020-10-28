
# targets
SRCS    = $(wildcard src/*.c)
TARGETS = $(patsubst %.c,%.o,$(notdir $(SRCS)))

# test hexs
TESTSRCS  = $(wildcard test/*.c)
TESTHEXES = $(patsubst %.c,%.hex,$(notdir $(TESTSRCS)))

.PHONY: all test

all: $(TARGETS)
	
test: $(TESTHEXES)

include gcc.mk