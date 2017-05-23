CC := g++

CFLAGS := -std=c++11 -Irapidjson/include -O3 -DNDEBUG
CFLAGS_32 := -m32 -msse2 -DRAPIDJSON_SSE2
CFLAGS_64 := -m64 -DRAPIDJSON_SSE42

CFLAGS_M := -bundle -undefined dynamic_lookup
CFLAGS_L := -shared -fPIC -lstdc++

SRC = qrapidjson.cpp

m32: $(SRC)
	$(CC) $(CFLAGS) $(CFLAGS_32) $(CFLAGS_M) $(SRC) -o qrapidjson_m32.so

m64: $(SRC)
	$(CC) $(CFLAGS) $(CFLAGS_64) $(CFLAGS_M) $(SRC) -o qrapidjson_m64.so

l32: $(SRC)
	$(CC) $(CFLAGS) $(CFLAGS_32) $(CFLAGS_L) $(SRC) -o qrapidjson_l32.so

l64: $(SRC)
	$(CC) $(CFLAGS) $(CFLAGS_64) $(CFLAGS_L) $(SRC) -o qrapidjson_l64.so

clean:
	rm -f qrapidjson_*.so

.PHONY: clean
