truthtable: truthtable.c
	gcc -Wall -Werror -fsanitize=address -o truthtable truthtable.c -lm

clean:
	rm -rf truthtable 

