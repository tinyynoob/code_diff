cc = gcc
ccflags = -pthread -O2
source = code_diff.c ptxt.c
header = ptxt.h

.PHONY: run format clean

code_diff: $(source)
	$(cc) -o $@ $^ $(ccflags)

run: code_diff
	./code_diff old.txt new.txt

format: $(source) $(header)
	clang-format -i $^

clean:
	-rm code_diff
