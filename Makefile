
all: cidr_random4 cidr_random6

cidr_random4: cidr_random4.c
	cc -Wall -Werror -o cidr_random4 cidr_random4.c

cidr_random6: cidr_random6.c
	cc -Wall -Werror -o cidr_random6 cidr_random6.c

test4: cidr_random4
	./cidr_random4 10.10.0.0 16 

test6: cidr_random6
	./cidr_random6 2001:db8:: 32

test: test4 test6

clean:
	rm -f cidr_random4 cidr_random6

.PHONY: all test4 test6 test clean
