@echo off
cd build
cmake --build . --parallel 4
app
cd ..
