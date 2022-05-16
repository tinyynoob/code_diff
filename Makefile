cc = gcc
ccflags = -pthread -O2
source = code_diff.c ptxt.c
header = ptxt.h

.PHONY: run format clean windows

code_diff: $(source)
	$(cc) -o $@ $^ $(ccflags)

run: code_diff
	./code_diff old.txt new.txt

format: $(source) $(header)
	clang-format -i $^

# cross-compile to windows
windows:
	 x86_64-w64-mingw32-gcc -o code_diff $(source) $(ccflags) 

clean:
	-rm code_diff code_diff.exe
