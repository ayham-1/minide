rm -rf .build/ 
cmake -B.build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -GNinja  &&
ninja -C .build -d stats -j 8 &&
mkdir -p .build/tests &&
ln -s ../../assets/ .build/tests/assets &&
ctest --test-dir .build/tests --output-on-failure -j 8 -R $1
cd ..
