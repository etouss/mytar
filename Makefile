SOURCES=main.c struct.c archiver.c desarchiver.c md5.c modification.c creation.c lock.c
OBJECTS=$(SOURCES:.c=.o)
CFLAGS= -std=c99 -std=gnu99 -Wall -pedantic -g -D _POSIX_C_SOURCE=200112L
EXECUTABLE=mytar.out

$(EXECUTABLE): $(OBJECTS)
	gcc -o $(EXECUTABLE) $(OBJECTS)

%.o : %.c
	gcc -c $(CFLAGS) $< -o $@

cleanall:
	rm -f $(EXECUTABLE) $(OBJECTS)
