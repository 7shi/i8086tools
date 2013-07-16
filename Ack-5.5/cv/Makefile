TARGET = cv
SOURCES = cv-pdp.c ../mylib.c
OBJECTS = $(SOURCES:.c=.o)
CFLAGS  = -I../include
LDFLAGS = -L../lib -lobject

all: $(TARGET)

$(TARGET): $(OBJECTS)
	gcc -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(TARGET).exe $(OBJECTS)
