CC=gcc
LIBRARIES = -lwiringPi
HEADERS = main.h parser.h gcodefunctions.h Config.h screen.h wifi.h
SOURCE = main.c parser.c gcodefunctions.c screen.c wifi.c
OBJECTS = $(SOURCE:.c=.o)
EXE = 3DPrinter
RM = rm -f

# Compile C files into Object files
%.o: %.c
	$(CC) -c $*.c
	
	
# Link objects files with libraries	
&(EXE): $(OBJECTS)
	$(CC) -o $(EXE) $(OBJECTS) $(LIBRARIES) 
	
# Objects depend on headers
$(OBJECTS): $(HEADERS)

# Remove object files
remove:
	$(RM) $(OBJECTS)
