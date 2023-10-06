rm -r .build/perf
mkdir -p .build/perf/
./scripts/prep_directory.sh ./../../ ./.build/perf/
cd ./.build/perf/
perf stat ../../$1
cd ../../
