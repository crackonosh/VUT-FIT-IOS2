program:
	gcc -std=c99 -Wall -Wextra -Werror -pedantic proj2.c -pthread -o program
clean:
	rm -f program
