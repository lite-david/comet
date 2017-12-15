build="./benchmarks/build"
evaluate="./benchmarks/simulator_output"
mkdir -p dump
dump="./dump"
./catapult.sim $build/matmul4_4.out > $dump/matmul4_4.txt
python verify_simulation.py $evaluate/matmul4_4.debug.out $dump/matmul4_4.txt

./catapult.sim $build/matmul8_8.out > $dump/matmul8_8.txt
python verify_simulation.py $evaluate/matmul8_8.debug.out $dump/matmul8_8.txt

./catapult.sim $build/matmul16_16.out > $dump/matmul16_16.txt
python verify_simulation.py $evaluate/matmul16_16.debug.out $dump/matmul16_16.txt

./catapult.sim $build/qsort_10.out > $dump/qsort_10.txt
python verify_simulation.py $evaluate/qsort_10.debug.out $dump/qsort_10.txt

./catapult.sim $build/qsort_50.out > $dump/qsort_50.txt
python verify_simulation.py $evaluate/qsort_50.debug.out $dump/qsort_50.txt

./catapult.sim $build/qsort_100.out > $dump/qsort_100.txt
python verify_simulation.py $evaluate/qsort_100.debug.out $dump/qsort_100.txt

./catapult.sim $build/qsort.out > $dump/qsort.txt
python verify_simulation.py $evaluate/qsort.debug.out $dump/qsort.txt

./catapult.sim $build/multiply.out > $dump/multiply.txt
python verify_simulation.py $evaluate/multiply.debug.out $dump/multiply.txt

./catapult.sim $build/towers.out > $dump/towers.txt
python verify_simulation.py $evaluate/towers.debug.out $dump/towers.txt

./catapult.sim $build/median.out > $dump/median.txt
python verify_simulation.py $evaluate/median.debug.out $dump/median.txt

./catapult.sim $build/vvadd.out > $dump/vvadd.txt
python verify_simulation.py $evaluate/vvadd.debug.out $dump/vvadd.txt
