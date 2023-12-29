rm -rf .build/
meson setup .build/ 
meson compile -C .build
./scripts/prep_directory.sh ../ .build/
cd .build
./minide
cd ..
