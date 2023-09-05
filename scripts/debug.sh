rm -rf .build/
cmake -B.build -DCMAKE_BUILD_TYPE=Debug -GNinja
ninja -C .build -j 8
./.build/minide
cd ..
