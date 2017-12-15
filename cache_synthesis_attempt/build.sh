echo "Building project..."
make
mkdir -p testdir
cp ./simulator/bin/simRISCV ./testdir/
cp ./core/bin/*.sim ./testdir/
cp ./core/verify_simulation.py ./testdir/
mkdir ./testdir/benchmarks
cp -r ./benchmarks/build ./testdir/benchmarks/
cp -r ./benchmarks/simulator_output ./testdir/benchmarks/
cp ./core/regtracker.py ./testdir/
cp ./core/testscript.sh ./testdir/
