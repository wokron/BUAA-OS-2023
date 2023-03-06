%.o: %.c
	$(CC) -c $<

.PHONY: clean

readelf: main.o readelf.o
	$(CC) $^ -o $@

hello: hello.c
	$(CC) $^ -o $@ -m32 -static -g

clean:
	rm -f *.o readelf hello
