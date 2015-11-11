BIN1 = MapPointDensity

BINall = $(BIN1) 
all : $(BINall)

# --- compiliers --- #
CC := g++

# --- flags --- #
cflags = -O3 -std=c++11 
LIBS = -lstdc++ -lX11 -lm -rdynamic -O3
OBJS := $(addsuffix .o,$(BINall))
OBJS := $(OBJS) Map.o

# --- main rules --- #
define make-bin
$(1) : $(OBJS) $(1).o
	$(CC) $$^ -o $$@ $(LIBS)
endef
$(foreach bin,$(BINall),$(eval $(call make-bin,$(bin))))

%.o : %.cpp
	$(CC) $(cflags) -c $< -o $@
