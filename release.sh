#!/bin/bash

rm -rf release
mkdir release
mkdir release/Frodo2Go
#mkdir release/menu

cd src
make clean&&make -f Makefile.bittboy 
cd ..

cp src/Frodo release/Frodo2Go/

cp controls.man.txt release

cp c64_icon.png release

cd release
zip Frodo2Go.zip -r Frodo2Go/ -r menu/ controls.man.txt c64_icon.png 
cd ..
cp release/Frodo2Go.zip .





