######## Build options ########

verbose = 0

######## Build setup ########

# SRCROOT should always be the current directory
SRCROOT         = $(CURDIR)

# .o directory
ODIR            = obj

# Source VPATHS
VPATH           += $(SRCROOT)/FreeRTOS
VPATH	        += $(SRCROOT)/FreeRTOS/portable/MemMang
VPATH	        += $(SRCROOT)/FreeRTOS/portable/GCC/POSIX

# FreeRTOS Objects
C_FILES			+= croutine.c
C_FILES			+= event_groups.c
C_FILES			+= list.c
C_FILES			+= queue.c
C_FILES			+= tasks.c
C_FILES			+= timers.c

# portable Objects
C_FILES			+= heap_3.c
C_FILES			+= port.c

# Main Object
C_FILES			+= helpers.c
C_FILES			+= main.c
C_FILES			+= constants.c
C_FILES			+= controller.c
C_FILES			+= filter.c
C_FILES			+= reference_generator.c

# Include Paths
INCLUDES        += -I$(SRCROOT)
INCLUDES        += -I$(SRCROOT)/FreeRTOS/include
INCLUDES        += -I$(SRCROOT)/FreeRTOS/portable/GCC/POSIX/

# Generate OBJS names
OBJS = $(patsubst %.c,%.o,$(C_FILES))

######## C Flags ########

# Warnings
CWARNS += -W
#CWARNS += -Wall
#CWARNS += -Werror
CWARNS += -Wextra
CWARNS += -Wformat
CWARNS += -Wmissing-braces
CWARNS += -Wno-cast-align
CWARNS += -Wparentheses
CWARNS += -Wshadow
CWARNS += -Wno-sign-compare
CWARNS += -Wswitch
CWARNS += -Wuninitialized
CWARNS += -Wunknown-pragmas
#CWARNS += -Wunused-function
#CWARNS += -Wunused-label
#CWARNS += -Wunused-parameter
#CWARNS += -Wunused-value
#CWARNS += -Wunused-variable
#CWARNS += -Wmissing-prototypes

#CWARNS += -Wno-unused-function

#CFLAGS += -std=c99 -D__EXTENSIONS__
CFLAGS += -m32
CFLAGS += -DDEBUG=1
#CFLAGS += -g -DUSE_STDIO=1 -D__GCC_POSIX__=1
CFLAGS += -g -UUSE_STDIO -D__GCC_POSIX__=1
ifneq ($(shell uname), Darwin)
CFLAGS += -pthread
endif

# MAX_NUMBER_OF_TASKS = max pthreads used in the POSIX port.
# Default value is 64 (_POSIX_THREAD_THREADS_MAX), the minimum number required by POSIX.
CFLAGS += -DMAX_NUMBER_OF_TASKS=300

CFLAGS += $(INCLUDES) $(CWARNS) -O2

LIBS += -lm

######## Makefile targets ########

# Rules
.PHONY : all
all: setup crazyflie

.PHONY : setup
setup:
# Make obj directory
	@mkdir -p $(ODIR)

# Fix to place .o files in ODIR
_OBJS = $(patsubst %,$(ODIR)/%,$(OBJS))

$(ODIR)/%.o: %.c
# If verbose, print gcc execution, else hide
ifeq ($(verbose),1)
	@echo ">> Compiling $<"
	$(CC) $(CFLAGS) -c -o $@ $<
else
	@echo ">> Compiling $(notdir $<)"
	@$(CC) $(CFLAGS) -c -o $@ $<
endif

crazyflie: $(_OBJS)
	@echo ">> Linking $@..."
ifeq ($(verbose),1)
	$(CC) $(CFLAGS) $^ $(LINKFLAGS) $(LIBS) -o $@
else
	@$(CC) $(CFLAGS) $^ $(LINKFLAGS) $(LIBS) -o $@
endif

	@echo "-------------------------"
	@echo "BUILD COMPLETE: $@"
	@echo "-------------------------"

.PHONY : clean
clean:
	@-rm -rf $(ODIR) crazyflie
	@echo "--------------"
	@echo "CLEAN COMPLETE"
	@echo "--------------"
