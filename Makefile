cc = gcc
ccflags = -pthread
source = code_diff.c ptxt.c
header = ptxt.h

.PHONY: run format clean

run: code_diff
	./code_diff old.txt new.txt

code_diff: $(source)
	$(cc) -o $@ $^ $(ccflags)

format: $(source) $(header)
	clang-format -i $^

clean:
	-rm code_diff
