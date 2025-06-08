CC = gcc
CFLAGS = -Wall -Wextra -O2
OBJ = icanspi.o circularBuffer.o
DEPS = circularBuffer.c
TARGET = icanspi
LDFLAGS = -ljetgpio

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)
