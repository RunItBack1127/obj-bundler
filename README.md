# OBJ Bundler
A C executable program for minifying and reducing the size of OBJ files, and for bundling all of the original associated OBJ data. The purpose of this program is to demonstrate the ability to compress OBJ files for reducing project size, along with maintaining the human readability of the original OBJ file.

***NOTE*** - This project is NOT meant to be used for production applications or projects - the application is only meant to be used for demonstrative purposes.

## Installation
* Requires GCC and Make (optional).

Pull the repo into your local directory or download the ZIP file and run the following command in the root folder.
```
make
```

Or without Make
```
gcc -o objbundler src/obj_bundler.c
```

## Use
In the root folder, add any OBJ files to be bundled, and run the following command.
```
objbundler --in [NAME_OF_OBJ]
```

**Example**
```
objbundler --in asteroid.obj
```
If no `--out` flag is specified, the program defaults to the name of the input file, with the `.objb` extension appended.

### Parameters
```
--in                                       Specifies input file
--out                                      Specifies output file, must end with .objb extension
--preserve-header-comments                 Bundles OBJ with the comments at the top of the OBJ file only
--include-w-coords                         Includes any optional fourth coordinates associated with vertex data (x, y, z, w)
--set-data-limit=[LIMIT]                   Specifies maximum number of data points to read from file (between 10000000 and 99999999)
```
