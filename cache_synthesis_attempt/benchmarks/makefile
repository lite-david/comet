objects = matmul4_4.o matmul8_8.o matmul16_16.o matmul32_32.o 
executables = $(OUT_DIR)/matmul4_4.out $(OUT_DIR)/matmul8_8.out $(OUT_DIR)/matmul16_16.out $(OUT_DIR)/qsort_10.out $(OUT_DIR)/qsort_50.out $(OUT_DIR)/qsort_100.out $(OUT_DIR)/multiply.out $(OUT_DIR)/median.out $(OUT_DIR)/qsort.out $(OUT_DIR)/towers.out $(OUT_DIR)/vvadd.out
dependancies = src/matmul.c include/matmul.h src/qsort.c include/qsort.h src/multiply.c src/multiply_main.c include/multiply.h include/dataset1.h
CCX = riscv32-unknown-elf-gcc
INCLUDE = ./include
OUT_DIR = ./build
SIM = ../simulator/bin/simRISCV
SIM_OUT = ./simulator_output
OPT = -mcmodel=medany -static -std=gnu99 -O2 -ffast-math -fno-common -fno-builtin-printf -lm -lgcc
all: directories $(SIM) $(executables)
$(OUT_DIR)/matmul4_4.out : $(dependancies) directories 
	$(CCX) $(OPT) -I $(INCLUDE) src/matmul.c -o $(OUT_DIR)/matmul4_4.out -D SIZE=4
	$(SIM) -v -f $(OUT_DIR)/matmul4_4.out &> $(SIM_OUT)/matmul4_4.debug.out
$(OUT_DIR)/matmul8_8.out : $(dependancies) directories 
	$(CCX) $(OPT) -I $(INCLUDE) src/matmul.c -o $(OUT_DIR)/matmul8_8.out -D SIZE=8
	$(SIM) -v -f $(OUT_DIR)/matmul8_8.out &> $(SIM_OUT)/matmul8_8.debug.out
$(OUT_DIR)/matmul16_16.out : $(dependancies) directories 
	$(CCX) $(OPT) -I $(INCLUDE) src/matmul.c -o $(OUT_DIR)/matmul16_16.out -D SIZE=16
	$(SIM) -v -f $(OUT_DIR)/matmul16_16.out &> $(SIM_OUT)/matmul16_16.debug.out
$(OUT_DIR)/qsort_10.out : src/qsort.c include/qsort.h directories 
	$(CCX) $(OPT) -I $(INCLUDE) src/qsort.c -o $(OUT_DIR)/qsort_10.out -D SIZE=10
	$(SIM) -v -f $(OUT_DIR)/qsort_10.out &> $(SIM_OUT)/qsort_10.debug.out
$(OUT_DIR)/qsort_50.out : src/qsort.c include/qsort.h directories 
	$(CCX) $(OPT) -I $(INCLUDE) src/qsort.c -o $(OUT_DIR)/qsort_50.out -D SIZE=50
	$(SIM) -v -f $(OUT_DIR)/qsort_50.out &> $(SIM_OUT)/qsort_50.debug.out
$(OUT_DIR)/qsort_100.out : src/qsort.c include/qsort.h directories 
	$(CCX) $(OPT) -I $(INCLUDE) src/qsort.c -o $(OUT_DIR)/qsort_100.out -D SIZE=100
	$(SIM) -v -f $(OUT_DIR)/qsort_100.out &> $(SIM_OUT)/qsort_100.debug.out
$(OUT_DIR)/qsort_1000.out : src/qsort.c include/qsort.h directories 
	$(CCX) $(OPT) -I $(INCLUDE) src/qsort.c -o $(OUT_DIR)/qsort_1000.out -D SIZE=1000
	$(SIM) -v -f $(OUT_DIR)/qsort_1000.out &> $(SIM_OUT)/qsort_1000.debug.out
$(OUT_DIR)/multiply.out: directories 
	$(CCX) $(OPT) -I $(INCLUDE) src/multiply_main.c src/multiply.c -o $(OUT_DIR)/multiply.out 
	$(SIM) -v -f $(OUT_DIR)/multiply.out &> $(SIM_OUT)/multiply.debug.out
$(OUT_DIR)/median.out:directories 
	$(CCX) $(OPT) -I $(INCLUDE) median/median_main.c median/median.c -o $(OUT_DIR)/median.out 
	$(SIM) -v -f $(OUT_DIR)/median.out &> $(SIM_OUT)/median.debug.out
$(OUT_DIR)/qsort.out:directories 
	$(CCX) $(OPT) -I $(INCLUDE) qsort/qsort_main.c -o $(OUT_DIR)/qsort.out 
	$(SIM) -v -f $(OUT_DIR)/qsort.out &> $(SIM_OUT)/qsort.debug.out
$(OUT_DIR)/towers.out:directories 
	$(CCX) $(OPT) -I $(INCLUDE) towers/towers_main.c -o $(OUT_DIR)/towers.out 
	$(SIM) -v -f $(OUT_DIR)/towers.out &> $(SIM_OUT)/towers.debug.out
$(OUT_DIR)/vvadd.out:directories 
	$(CCX) $(OPT) -I $(INCLUDE) vvadd/vvadd_main.c -o $(OUT_DIR)/vvadd.out 
	$(SIM) -v -f $(OUT_DIR)/vvadd.out &> $(SIM_OUT)/vvadd.debug.out
directories: ${OUT_DIR}
${OUT_DIR}:
	mkdir -p ${OUT_DIR}
	mkdir -p $(SIM_OUT)
$(SIM):
	make -C ../simulator
clean:
	rm -rf $(OUT_DIR) 
	rm -rf $(SIM_OUT)
.PHONY: all clean directories
