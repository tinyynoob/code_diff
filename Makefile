cc = gcc
ccflags = -pthread

.PHONY: run format

run: code_diff
	./code_diff old.txt new.txt

format: code_diff.c ptxt.c ptxt.h
	clang-format -i $^

code_diff: code_diff.c ptxt.c
	$(cc) -o $@ $^ $(ccflags)

