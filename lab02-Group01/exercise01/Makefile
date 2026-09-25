CC = gcc

TARGETS = gen_numbers.out select_arg.out gen_in_range.out
OS = $(shell uname -s)

.DEFAULT_GOAL := all

.PHONY: all clean test

ifeq ($(OS),Linux)

all: $(TARGETS)

gen_numbers.out: gen_numbers.c utils.c utils.h
	$(CC) -o $@ $< utils.c

select_arg.out: select_arg.c utils.c utils.h
	$(CC) -o $@ $< utils.c

gen_in_range.out: gen_in_range.c
	$(CC) -o $@ $<

test: all
	@./gen_numbers.out 5 > /dev/null
	@./select_arg.out one two three > /dev/null
	@./gen_in_range.out 1 10 > /dev/null
	@echo "All tests passed"

else

all:
	@echo "Sorry, I prefer Linux"

test:
	@echo "Sorry, I prefer Linux"

endif

clean:
	rm -f $(TARGETS)