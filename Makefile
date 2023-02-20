.PHONY: clean

out: calc case_all
	./calc < case_all > out

# Your code here.

clean:
	rm -f out calc casegen case_*
