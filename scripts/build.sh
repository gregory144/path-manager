#!/usr/bin/env sh

autoreconf --install

./configure

make

./bin/path
