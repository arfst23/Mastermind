################################################################################

CC		= gcc
WARNINGS	= -Wall -Wno-parentheses -Wno-missing-braces -Wextra \
		  -Winit-self -Wmissing-include-dirs -Wunused-parameter \
		  -Wno-pragmas -Wundef -Wno-endif-labels -Wpointer-arith \
		  -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion \
		  -Werror -Wunreachable-code
OPTIMIZATION	= -O3 -funroll-loops \
		  -finline-functions -finline-limit=1200 -finline-functions-called-once \
		  -mtune=core2 -march=core2 -mfpmath=sse -msse4.2 -mssse3 -m64 \
		  -fopenmp
CFLAGS		= -std=c99 -pedantic -pipe $(WARNINGS)  $(OPTIMIZATION) \
		  # -fstack-protector-all -g # -fmudflap
CCFLAGS		= -std=c++0x -pedantic -pipe $(WARNINGS) $(OPTIMIZATION)
LDLIBS		= # -lmudflap

#CC		= clang-3.5
#WARNINGS	= -pedantic-errors -Werror \
#		  -Weverything -Wextra-tokens \
#		  -Wno-padded -Wno-missing-prototypes -Wno-shadow -Wno-logical-op-parentheses
#OPTIMIZATION	= -march=native -m64 -O3 \
#		  -funroll-loops -ffast-math -fvectorize
#CFLAGS		= -std=c11 -pipe $(WARNINGS) $(OPTIMIZATION) # -fopenmp
#CCFLAGS		=
#LDLIBS		=

TAGS		= ctags
TAGSFLAGS	= -i =cdefgstuCFS --format=1
PODTOMAN	= pod2man
REASON		= @if [ -f $@ ]; then echo "[$@: $?]"; else echo "[$@]"; fi

.SUFFIXES: .c .i .o .h .1 .3

.c.i:
	$(REASON)
	$(COMPILE.c) -E $< > $@

.c.o:
	$(REASON)
	$(COMPILE.c) $< $(OUTPUT_OPTION)

.c.1:
	$(REASON)
	pod2man -s 1 -c 'Executables' $< $@

.h.3:
	$(REASON)
	pod2man -s 1 -c 'Library Functions' $< $@

################################################################################

all: mastermind

mastermind: mastermind.o
	$(REASON)
	$(LINK.c) -o $@ $^ $(LDLIBS)

mastermind-parallel: mastermind-parallel.o
	$(REASON)
	$(LINK.c) -o $@ $^ $(LDLIBS)

mastermind-enum: mastermind-enum.o
	$(REASON)
	$(LINK.c) -o $@ $^ $(LDLIBS)

dp: dp.o
	$(REASON)
	$(LINK.c) -o $@ $^ $(LDLIBS)

mastermind-unique: mastermind-unique.o
	$(REASON)
	$(LINK.c) -o $@ $^ $(LDLIBS)

deps depend:
	$(REASON)
	$(CC) -MM $(CFLAGS) *.c > deps

tags:
	$(TAGS) $(TAGSFLAGS) -o $@ *.h *.cc

clean:
	$(RM) *.o core err deps tags *~

-include deps

################################################################################
