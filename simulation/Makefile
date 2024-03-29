CC = gcc

OBJDIR = obj
SRCDIR = src
BINDIR = bin
INCLUDEDIR = include
LIBDIR = lib

CFLAGS = -std=gnu99 -g -Wall -I$(INCLUDEDIR)
LFLAGS = -lm -pthread

#CFLAGS += -DDEBUG_DRIVER
#CFLAGS += -DDEBUG_DCTCP
#CFLAGS += -DDEBUG_SNAPSHOTS
# CFLAGS += -DWRITE_QUEUELENS
# CFLAGS += -DRECORD_PACKETS
# CFLAGS += -DSJF

INCLUDE = $(INCLUDEDIR)/*.h

OBJ = $(OBJDIR)/arraylist.o \
      $(OBJDIR)/buffer.o \
      $(OBJDIR)/flow.o \
      $(OBJDIR)/flowlist.o \
      $(OBJDIR)/link.o \
      $(OBJDIR)/links.o \
      $(OBJDIR)/node.o \
      $(OBJDIR)/packet.o \
      $(OBJDIR)/routing_table.o \
      $(OBJDIR)/system_stats.o \
      $(OBJDIR)/timeseries.o \
      $(OBJDIR)/tor.o 

default: edm

# list of all the targets to be generated
edm: $(BINDIR)/driver-edm
ndp: $(BINDIR)/driver-ndp
pfc: $(BINDIR)/driver-pfc
pfc-alone: $(BINDIR)/driver-pfc-obslt
dctcp: $(BINDIR)/driver-dctcp
pfabric: $(BINDIR)/driver-pfabric
fastpass: $(BINDIR)/driver-fastpass

batch: $(BINDIR)/driver-batch-edm
test: $(BINDIR)/test

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
