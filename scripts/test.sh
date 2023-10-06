rm -rf .build/ 
cmake -B.build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -GNinja  &&
ninja -C .build -d stats -j 8 &&
mkdir -p .build/tests &&
#ln -s ../../assets/ .build/tests/assets &&
./scripts/prep_directory.sh ./../../ .build/tests &&
ctest --output-on-failure --test-dir .build/tests -j 8 -R $1
cd ..
