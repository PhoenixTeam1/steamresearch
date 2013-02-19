CC = gcc
CFLAGS = -g -c -Wall
LDFLAGS =
SOURCES = swal_test.c swal.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = crawl

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *o $(EXECUTABLE)


