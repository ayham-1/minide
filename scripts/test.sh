rm -rf .build/ 
cmake -B.build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -GNinja 
ninja -C .build -d stats
ctest --test-dir .build/tests --output-on-failure
cd ..
