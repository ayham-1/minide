rm -rf .build/
cmake -B.build -DCMAKE_BUILD_TYPE=Debug -GNinja
ninja -C .build
./.build/minide
cd ..
