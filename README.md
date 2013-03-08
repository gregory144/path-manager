path-manager
============

A CLI utility for managing your PATH environment variable

Usage (bash):

Add a directory to your path:

`` `path -a /path/to/directory -e` ``

or

`source <(path -a /path/to/directory -e)`

Remove a directory from your path:

`` `path -r /path/to/directory -e` ``

or

`source <(path -r /path/to/directory -e)`

List all files in your path:

`path -l`

Search your path:

`path -s gcc`

TODO
============

1. Add ability to add a single file to the path (using symbolic link to
~/.path/exename -> existing file) instead of a whole directory
1. Add ability to manage multiple files of the same name (i.e. promote 1
file to be first in line)
1. Provide more options for search (only display exact matches/include
fuzzy search matches even if exact match is found)
1. Improve algorithm for fuzzy search
1. Add functionality to detect current shell (using getppid and
/proc/$pid/cmdline
1. Add warnings for duplicate executable file names (i.e. a file is on
the path, but it can't be used because it's being 'hidden' by another
file of the same name)
