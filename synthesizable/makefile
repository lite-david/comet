AC=/opt/Catapult-10.0b/Mgc_home/shared/include
AP=/opt/Xilinx/Vivado_HLS/2015.4/include
INC=$(AC) $(AP) ./include
LINC=./include
INC_PARAMS=$(foreach d, $(INC), -I$d)
VARS_CAT=-D__DEBUG__=1 -D__CATAPULT__=1
VARS_VIV=-D__DEBUG__=1 -D__VIVADO__=1
FILES=core.cpp elfFile.cpp reformeddm_sim.cpp syscall.cpp
S=./src
S_FILES=$(foreach f, $(FILES), $(S)/$f)
HEADER=core.h debug.h elf.h elfFile.h portability.h registers.h riscvISA.h syscall.h DataMemory.h
I_HEADER=$(foreach f, $(HEADER), $(INC)/$f)

catapult.sim: $(S_FILES) $(I_HEADER)
	g++ -o catapult.sim $(INC_PARAMS) $(S_FILES) $(VARS_CAT)

vivado.sim: $(S_FILES) $(I_HEADER) 
	g++ -o vivado.sim $(INC_PARAMS) $(S_FILES) $(VARS_VIV)

elfFile.o: $(S)/elfFile.cpp $(LINC)/elf.h $(LINC)/elfFile.h
	g++ $(INC_PARAMS) $(S)/elfFile.cpp -c 

syscall.o: $(S)/syscall.cpp $(LINC)/riscvISA.h $(LINC)/portability.h
	g++ $(INC_PARAMS) $(S)/syscall.cpp -c

core_catapult.o: $(S)/core.cpp $(LINC)/riscvISA.h $(LINC)/registers.h $(LINC)/portability.h $(LINC)/debug.h $(LINC)/syscall.h syscall.o
	g++ $(INC_PARAMS) -o core_catapult.o $(S)/core.cpp syscall.o -c $(VARS_CAT) -lm

core_vivado.o: $(S)/core.cpp $(LINC)/riscvISA.h $(LINC)/registers.h $(LINC)/portability.h $(LINC)/debug.h $(LINC)/syscall.h syscall.o
	g++ $(INC_PARAMS) $(S)/core.cpp -c $(VARS_VIV) -o core_vivado.o

clean:
	rm -rf *.o catapult.sim vivado.sim 
