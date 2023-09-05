rm -rf .build/
cmake -B.build -DCMAKE_BUILD_TYPE=Release -GNinja 
ninja -C .build -j 8
./.build/minide
cd ..
