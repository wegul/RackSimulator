CC = gcc

OBJDIR = obj
SRCDIR = src
BINDIR = bin
INCLUDEDIR = include
LIBDIR = lib

CFLAGS = -std=gnu99 -g -O3 -Wall -I$(INCLUDEDIR)
LFLAGS = -lm -pthread
ifeq ($(INCLUDE_SNAPSHOTS),"TRUE")
      CFLAGS += -DENABLE_SNAPSHOTS
endif
#CFLAGS += -DDEBUG_DRIVER
#CFLAGS += -DDEBUG_DCTCP
#CFLAGS += -DDEBUG_SNAPSHOTS
CFLAGS += -DWRITE_QUEUELENS
#CFLAGS += -DRECORD_PACKETS

INCLUDE = $(INCLUDEDIR)/*.h

OBJ = $(OBJDIR)/arraylist.o \
      $(OBJDIR)/buffer.o \
      $(OBJDIR)/flow.o \
      $(OBJDIR)/flowlist.o \
      $(OBJDIR)/link.o \
      $(OBJDIR)/links.o \
      $(OBJDIR)/memory.o \
      $(OBJDIR)/node.o \
      $(OBJDIR)/packet.o \
      $(OBJDIR)/routing_table.o \
      $(OBJDIR)/snapshot.o \
      $(OBJDIR)/spine.o \
      $(OBJDIR)/system_stats.o \
      $(OBJDIR)/timeseries.o \
      $(OBJDIR)/tor.o 

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
