path-manager
============

A CLI utility for managing your PATH environment variable

[![Build Status](https://travis-ci.org/gregory144/path-manager.png)](https://travis-ci.org/gregory144/path-manager)


Usage (bash):

Install:

1. `curl http://path.gtgross.com/install.sh | bash`
1. Open up a new shell (changes won't apply to your current shell on
installation): `bash`

List all directories in your path:

`path`

List all files in your path:

`path -l`

Add a directory to your path:

`` `path -a /path/to/directory -e` ``

Remove a directory from your path:

`` `path -r /path/to/directory -e` ``

Search your path:

`path -s gcc`

COMPATIBILITY
============

Tested on Linux Mint 13 Maya & Fedora 18 (Spherical Cow)
GNU bash, version 4.2.24, 4.2.39

BUILDING
============

Debian/Ubuntu/Mint:

`sudo apt-get install build-essential autoconf automake`

Fedora

`yum install gcc autoconf automake`

Then:

    ./configure
    make

    make install

    # or
 
    sudo make install

TODO
============

1. Add functionality to add a single file to the path (using symbolic link to
~/.path/exename -> existing file) instead of a whole directory
1. Add functionality to manage multiple files of the same name (i.e. promote 1
file to be first in line)
1. Improve algorithm for fuzzy search
1. Add functionality to detect current shell (using getppid and
/proc/$pid/cmdline, or getpwuid returns user's default shell)
1. Add warnings for duplicate executable file names (i.e. a file is on
the path, but it can't be used because it's being 'hidden' by another
file of the same name)
1. Add append functionality (add to the end of PATH)
1. Add support for other env variables (MANPATH, COWPATH)
1. Research options for modifying current shell's path without having to
use source/backticks (/dev/path? - device driver, . /dev/path)
