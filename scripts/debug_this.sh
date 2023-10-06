rm -r .build/gdb
mkdir -p .build/gdb/
./scripts/prep_directory.sh ./../../ ./.build/gdb/
cp .gdbinit ./.build/gdb/.gdbinit
cd ./.build/gdb/
gdb ../../$1
cd ../../
