rm -rf .build/perf
mkdir -p .build/perf/
./scripts/prep_directory.sh ./../../ ./.build/perf/
cd ./.build/perf/
../../$1
cd ../../
