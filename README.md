path-manager
============

A CLI utility for managing your PATH environment variable

TODO
============

1. Fuzzy search path to find a specific executable
1. Report which executable files are on the path
1. Add ability to add a single file to the path (using symbolic link to
~/.path/exename -> existing file) instead of a whole directory
1. Add ability to manage multiple files of the same name (i.e. promote 1
file to be first in line)
1. Add warnings for duplicate executable file names (i.e. a file is on
the path, but it can't be used because it's being 'hidden' by another
file of the same name)
