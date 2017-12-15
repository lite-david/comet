build="./benchmarks/build"
ref="./benchmarks/reference_output"
mkdir -p logs
log="./logs"

echo "Running quicksort benchmark..."
./catapult.sim $build/qsort.out > $log/qsort.log
python regtracker.py $ref/qsort.ref $log/qsort.log
tail -n14 $log/qsort.log
printf "\n\n\n "

echo "Running multiplication benchmark..."
./catapult.sim $build/multiply.out > $log/multiply.log
python regtracker.py $ref/multiply.ref $log/multiply.log
tail -n14 $log/multiply.log
printf "\n\n\n "

echo "Running towers of hanoi benchmark..."
./catapult.sim $build/towers.out > $log/towers.log
python regtracker.py $ref/towers.ref $log/towers.log
tail -n14 $log/towers.log
printf "\n\n\n "

echo "Running median benchmark..."
./catapult.sim $build/median.out > $log/median.log
python regtracker.py $ref/median.ref $log/median.log
tail -n14 $log/median.log
printf "\n\n\n "

echo "Running vector addition benchmark..."
./catapult.sim $build/vvadd.out > $log/vvadd.log
python regtracker.py $ref/vvadd.ref $log/vvadd.log
tail -n14 $log/vvadd.log
printf "\n\n\n "
