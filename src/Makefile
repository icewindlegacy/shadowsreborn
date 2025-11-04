### Shadows Reborn - v1.0
### Makefile - November 3, 2025
### This Makefile contains rules for compiling even when header files change.
##############################################################################
CC      = gcc
PROF    = -O -ggdb
NOCRYPT =

C_FLAGS =  -Wall  $(PROF) -Dunix -DNOCRYPT
L_FLAGS = $(PROF) -DNOCRYPT
LIBS	= -lcrypt -lz
TARGET	= rom

SRC_FILES := $(wildcard *.c)
O_FILES := $(patsubst %.c, obj/%.o, $(SRC_FILES))
H_FILES := $(wildcard *.h)

rom: $(O_FILES)
	rm -f $(TARGET)
	$(CC) $(L_FLAGS) -o $(TARGET) $(O_FILES) $(LIBS)

obj/%.o: %.c $(H_FILES)
	$(CC) -c $(C_FLAGS) $< -o $@


clean:
	rm -f $(O_FILES) $(TARGET)
	rm -f *.~
#	rm -f "#*#"

bzip:
	rm -f $(O_FILES)
	rm -f *.~
#	rm -f "#*#"
	#rm -f $(TARGET)
	tar -jcvf ../../$(TARGET).tar.bz2 ../../QuickMUD

gzip:	
	rm -f $(O_FILES)
	rm -f *.~
#	rm -f "#*#"
	#rm -f $(TARGET)
	tar -zcvf ../../$(TARGET).tar.gz -"Xexclude.dat" ../QuickMUD


	
