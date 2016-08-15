#!/bin/sh
set -e
if [ ! -d libgit2/build ]; then
	echo "Fetching libgit2..."
	git submodule init -q
	git submodule update -q
	mkdir libgit2/build
fi
if [ ! -f libgit2/build/libgit2.a ]; then
	echo "Building libgit2..."
	cd libgit2/build
	cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_C_FLAGS=-fPIC -DUSE_SSH=ON .. >libgit2_cmake_init.log 2>&1
	cmake --build . >libgit2_cmake_compile.log 2>&1
fi

echo "libgit2 looks fine. make clean in the PHP extension dir will usually delete the build library, so run this script again after."
