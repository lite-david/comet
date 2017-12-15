all:
	make -C ./common
	make catapult -C ./core
	make -C ./simulator
	make -C ./benchmarks
	
clean:
	make clean -C ./core
	make clean -C ./simulator
	make clean -C ./benchmarks
	make clean -C ./common
	rm -rf testdir

