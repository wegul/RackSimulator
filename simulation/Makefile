CC = gcc

SRCDIR = src
OBJDIR = obj
BINDIR = bin
INCLUDEDIR = include
LIBDIR = lib

CFLAGS = -std=gnu99 -g -O3 -Wall -I$(INCLUDEDIR)
LFLAGS = -lm -pthread

INCLUDE = $(INCLUDEDIR)/*.h

OBJ = $(OBJDIR)/bounded_buffer.o \
      $(OBJDIR)/arraylist.o \
      $(OBJDIR)/hashmap.o \
      $(OBJDIR)/link.o \
      $(OBJDIR)/node.o \
      $(OBJDIR)/tor.o \
      $(OBJDIR)/spine.o \
      $(OBJDIR)/links.o \
      $(OBJDIR)/packet.o \
      $(OBJDIR)/flow.o \
      $(OBJDIR)/flow_patterns.o \
      $(OBJDIR)/system_stats.o \
      $(OBJDIR)/routing_table.o

default: all

# list of all the targets to be generated
all: $(BINDIR)/driver

# % is the wildcard, $< means the first item to the right of :
$(OBJDIR)/%.o: $(LIBDIR)/%.c $(INCLUDE)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDE)
	$(CC) -c -o $@ $< $(CFLAGS)

# $@ means everything to the left of :, $^ means everything to the right of :
$(BINDIR)/%: $(OBJDIR)/%.o $(OBJ)
	$(CC) -o $@ $^ $(LFLAGS)

# to ensure make doesnot delete intermediate files like *.o
.PRECIOUS: $(OBJDIR)/%.o

.PHONY: default all clean

clean:
	rm -f $(OBJDIR)/*.o; rm -f $(BINDIR)/*;
