rm -rf .build/
cmake -B.build -DCMAKE_BUILD_TYPE=Release -GNinja 
ninja -C .build
./.build/minide
cd ..
