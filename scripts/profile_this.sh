rm -rf .build/profiling
mkdir -p .build/profiling/
./scripts/prep_directory.sh ./.build/profiling/
cd ./.build/profiling/
kcachegrind &
valgrind --tool=callgrind ../../$1
cd ../../
