all: fibo

fibo: code
	make -C ./code
	gcc ./code/fibo.o ./code/main.o -o fibo

clean:
	make clean -C ./code
