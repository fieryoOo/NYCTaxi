BIN1 = PointDensity_GaussianModel

BINall = $(BIN1) 
all : $(BINall)

# --- compiliers --- #
CC := g++

# --- flags --- #
cflags = -O3 -std=c++11 -fopenmp
LIBS = -lstdc++ -lX11 -lm -fopenmp -rdynamic -O3
#OBJS := $(addsuffix .o,$(BINall))
OBJS := $(OBJS) Map.o

# --- main rules --- #
define make-bin
$(1) : $(OBJS) $(1).o
	$(CC) $$^ -o $$@ $(LIBS)
endef
$(foreach bin,$(BINall),$(eval $(call make-bin,$(bin))))

# --- .cpp rules with dependencies assembled by gcc --- #
define make-cpp
$(shell $(CC) $(cflags) -MM -MT $(patsubst %.cpp,%.o,$(1)) $(1) | tr -d '\\\n' | awk '{print}' )
	$(CC) $(cflags) -c $$< -o $$@
endef
FSRC = $(wildcard *.cpp)
$(foreach fcpp,$(FSRC),$(eval $(call make-cpp,$(fcpp))))

#%.o : %.cpp
#	$(CC) $(cflags) -c $< -o $@

.PHONY : clean
clean :
	rm -f $(BIN1) $(BIN1).o $(OBJS)
