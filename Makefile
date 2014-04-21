objects = main.o ip.o ip_input.o ip_output.o ip_route.o

a.out : $(objects)
	gcc -o a.out $(objects)

main.o : ip.h ip_route.h ip_output.h ip_input.h protocol.h
	gcc -c main.c

ip.o : ip.h
	gcc -c ip.c

ip_input.o : ip_input.h
	gcc -c ip_input.c

ip_output.o : ip_output.h ip_input.h ip_route.h
	gcc -c ip_output.c

ip_route.o : ip_route.h
	gcc -c ip_route.c

clean :
	rm a.out $(objects)
