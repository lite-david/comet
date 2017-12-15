OUT_DIR = ./build
OBJDUMP_DIR = ./dump
INCLUDE = ./include
REFERENCE_DIR = ./reference_output

CCX = riscv32-unknown-elf-gcc
OBJDUMP = riscv32-unknown-elf-objdump -D
SIM = ../simulator/bin/simRISCV

executables = $(OUT_DIR)/multiply.out $(OUT_DIR)/median.out $(OUT_DIR)/qsort.out $(OUT_DIR)/towers.out $(OUT_DIR)/vvadd.out
diassembled = $(OBJDUMP_DIR)/multiply.dump $(OBJDUMP_DIR)/median.dump $(OBJDUMP_DIR)/qsort.dump $(OBJDUMP_DIR)/towers.dump $(OBJDUMP_DIR)/vvadd.dump 
reference = $(REFERENCE_DIR)/multiply.ref $(REFERENCE_DIR)/median.ref $(REFERENCE_DIR)/qsort.ref $(REFERENCE_DIR)/towers.ref $(REFERENCE_DIR)/vvadd.ref

OPT = -mcmodel=medany -static -std=gnu99 -O2 -ffast-math -fno-common -fno-builtin-printf -lm -lgcc

all: directories $(SIM) $(executables) $(diassembled) $(reference)

$(OUT_DIR)/multiply.out:
	$(CCX) $(OPT) -I $(INCLUDE) multiply/multiply_main.c multiply/multiply.c -o $(OUT_DIR)/multiply.out 
$(OUT_DIR)/median.out:
	$(CCX) $(OPT) -I $(INCLUDE) median/median_main.c median/median.c -o $(OUT_DIR)/median.out 
$(OUT_DIR)/qsort.out:
	$(CCX) $(OPT) -I $(INCLUDE) qsort/qsort_main.c -o $(OUT_DIR)/qsort.out 
$(OUT_DIR)/towers.out:
	$(CCX) $(OPT) -I $(INCLUDE) towers/towers_main.c -o $(OUT_DIR)/towers.out 
$(OUT_DIR)/vvadd.out:
	$(CCX) $(OPT) -I $(INCLUDE) vvadd/vvadd_main.c -o $(OUT_DIR)/vvadd.out 

$(OBJDUMP_DIR)/multiply.dump: $(OUT_DIR)/multiply.out
	$(OBJDUMP) $(OUT_DIR)/multiply.out > $(OBJDUMP_DIR)/multiply.dump
$(OBJDUMP_DIR)/median.dump: $(OUT_DIR)/median.out 
	$(OBJDUMP) $(OUT_DIR)/median.out > $(OBJDUMP_DIR)/median.dump
$(OBJDUMP_DIR)/qsort.dump: $(OUT_DIR)/qsort.out
	$(OBJDUMP) $(OUT_DIR)/qsort.out > $(OBJDUMP_DIR)/qsort.dump
$(OBJDUMP_DIR)/towers.dump: $(OUT_DIR)/towers.out
	$(OBJDUMP) $(OUT_DIR)/towers.out > $(OBJDUMP_DIR)/towers.dump
$(OBJDUMP_DIR)/vvadd.dump:  $(OUT_DIR)/vvadd.out
	$(OBJDUMP) $(OUT_DIR)/vvadd.out > $(OBJDUMP_DIR)/vvadd.dump

$(REFERENCE_DIR)/multiply.ref: $(OUT_DIR)/multiply.out
	$(SIM) -v -f $(OUT_DIR)/multiply.out &> $(REFERENCE_DIR)/multiply.ref
$(REFERENCE_DIR)/median.ref: $(OUT_DIR)/median.out
	$(SIM) -v -f $(OUT_DIR)/median.out &> $(REFERENCE_DIR)/median.ref
$(REFERENCE_DIR)/qsort.ref: $(OUT_DIR)/qsort.out
	$(SIM) -v -f $(OUT_DIR)/qsort.out &> $(REFERENCE_DIR)/qsort.ref
$(REFERENCE_DIR)/towers.ref:  $(OUT_DIR)/towers.out
	$(SIM) -v -f $(OUT_DIR)/towers.out &> $(REFERENCE_DIR)/towers.ref
$(REFERENCE_DIR)/vvadd.ref:  $(OUT_DIR)/vvadd.out
	$(SIM) -v -f $(OUT_DIR)/vvadd.out &> $(REFERENCE_DIR)/vvadd.ref

directories: $(OUT_DIR) $(OBJDUMP_DIR) $(REFERENCE_DIR)
$(OUT_DIR):
	mkdir -p ${OUT_DIR}
$(OBJDUMP_DIR):
	mkdir -p ${OBJDUMP_DIR}
$(REFERENCE_DIR):
	mkdir -p ${REFERENCE_DIR}
$(SIM):
	make -C ../simulator
clean:
	rm -rf $(OUT_DIR) 
	rm -rf $(OBJDUMP_DIR) 
	rm -rf $(REFERENCE_DIR)

.PHONY: all clean directories
