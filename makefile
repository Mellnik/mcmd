GCC = gcc
OUTPUT = "bin/mcmd.so"
COMPILER_FLAGS = -c -m32 -fPIC -O3 -DLINUX -w -I./include/ -I./include/SDK/amx/
LIBRARIES = -lrt

all: mcmd clean

mcmd:
	$(GCC) $(COMPILER_FLAGS) ./include/SDK/amx/*.c
	$(GCC) $(COMPILER_FLAGS) ./include/SDK/*.c
	$(GCC) $(COMPILER_FLAGS) ./src/*.c
	mkdir -p bin
	$(GCC) -m32 -O2 -fshort-wchar -shared -o $(OUTPUT) *.o $(LIBRARIES)
	
clean:
	rm -f *.o
