@echo off
cd build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build . --parallel 4 
cd ..


