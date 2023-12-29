rm -rf .build/
meson setup .build/ --optimization=g --buildtype debug
meson compile -C .build 
./scripts/prep_directory.sh ../ .build/
cd .build
meson test $1
cd ..
