echo "Building project..."
make
mkdir -p testdir
cp ./simulator/bin/simRISCV ./testdir/
cp ./core/bin/*.sim ./testdir/
cp ./util/verify_simulation.py ./testdir/
mkdir ./testdir/benchmarks
cp -r ./benchmarks/build ./testdir/benchmarks/
cp -r ./benchmarks/reference_output ./testdir/benchmarks/
cp ./util/testscript.sh ./testdir/
cp ./util/regtracker.py ./testdir/
