# apfs-check-clones
An utility to check if two files are identical clones in macOS APFS.


## How it works and inspiration

[This repository](https://github.com/dyorgio/apfs-clone-checker) by Dyorgio Nascimento checks if, given two paths, the files happen to be APFS clones.

I used [hoakley's research](https://eclecticlight.co/2021/04/02/how-can-you-tell-whether-a-file-has-been-cloned-in-apfs/) to try my hand at solving a slightly different problem: How can we find _which_ files are the same in a given folder?  This repository contains a litle C tool called `check_clones` that can be used in combination of some shell scripting.

## Usage
```.sh
./check_clones path_of_file
```

If the file has a clone reference > 1, then some useful information about the file is displayed in a line:

```
Clone ID:  21BC2DE ./cloned3.c: clones: 2, size(logical): 6.9 KiB, size(physical): 8 KiB, size (private): 0
```

The key info is the 'Clone ID', which is shared across cloned files and being the first thing in the line means it can be sorted to group the files with the same Clone IDs. The name and size is also displayed to help a human identify which are the important files.

For a full directory scan, some shell scripting can be used:

```.sh
find $1 -exec ./check_clones \{\} \; 2>/dev/null > clone_report.txt
sort clone_report.txt
```

It is recommended to add 'check_clones' to the Full Disk Access section in macOS Security Preferences panel.

A handy sample script that does this is in `./clone_check.sh`
Warning: running this on `$HOME` can take more than an hour depending on the numbers of files you have.

## Compilation (tested in Sonoma 14.2.1 on ARM)

Have gcc installed (XCode), download this repository and run make:

```.sh
make
```

Or just copy and paste `check_clones.c` to your editor and figure things out:

```.sh
gcc -o check_clones check_clones.c
```

## Limitations

Note that if the sorted report mentions, for example, 2 clone IDs for a given file but the 'clones:' number is bigger than that, it means that there are other clones in another directory that wasn't scanned by 'find', so beware!

* Although the tool is safe as it does not execute changes in the filesystem, you use it at your own risk, results can be misleading
* I haven't tested it thoroughly, not sure if it works within macOS package files.  

