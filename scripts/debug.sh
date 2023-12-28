rm -rf .build/
meson setup .build/ 
cd .build/
meson compile
./scripts/prep_directory.sh ../ .build/
./minide
cd ..
