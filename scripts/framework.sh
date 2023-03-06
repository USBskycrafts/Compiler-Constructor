#!/bin/bash

dirName="322_framework" ;
origDir=`pwd` ;

# Compile all compilers
make -j ;

# Copy all files to an empty directory
mkdir ../$dirName ;
cd ../$dirName ;
cp -r "$origDir"/* ./ ;

# Remove compiler sources
rm */src/* ;

# Remove object files
rm -r */obj ;

# Copy the binary into a hidden directory
for i in L1 L2 L3 IR LA LB LC LD ; do
  cd $i ;
  mkdir -p .bin ;
  cp bin/* .bin/ ;
  cd ../ ;
done

# Create the package
cd ../ ;
tar cfj ${dirName}.tar.bz2 ${dirName} ;
mv ${dirName}.tar.bz2 src ;
rm -r ${dirName} ;
