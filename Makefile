# Marcelo Barros de Almeida
# 15/09/2002

# ---------------  MACROS -------------------------------
# Directories 
PROGRAM = brtos
INCLUDE =

# set up compiler and options
CXX      = msp430-gcc
OBJDUMP  = msp430-objdump
OBJCOPY  = msp430-objcopy
CXXFLAGS = -mmcu=msp430x149 -O2 

# include files

# source files to compile
SRC = brtos.c app.c

# All OBJ files will have the same base name but with
# extension .o
OBJ = $(addsuffix .obj, $(basename $(SRC)))

# --------------- TARGETS ---------------------------------

all: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(PROGRAM).elf $(OBJ)
	$(OBJDUMP) -tDShpGt $(PROGRAM).elf > $(PROGRAM).map
	$(OBJCOPY) -O ihex $(PROGRAM).elf $(PROGRAM).dhex

%.obj: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

clean:
	del *.obj *.map *.?hex *.elf
