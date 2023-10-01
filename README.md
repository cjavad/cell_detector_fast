# cell_detector_fast

Fastest implementation of Assignment 1 for Computer Systems (02132) for Fall 2023 DTU

## Basic usage

Before running the make file run either `setup_win.sh` or `setup_lin.sh`, note the windows build targets WSL, and is not supported with windows make, but you can compile targetting a windows binary. We recommond compiling for and in a linux enviroment with GCC or Clang.

The entire build process is managed by the Makefile. We support the following make routines:

- `build`
  - Builds entire program with cached build objects

- `clean`
  - Cleans build objects

- `release`
  - Rebuilds entire program without using build objects

- `run`
  - Runs the program with no parameters.

Once the program is compiled it can be located in either `dist/linux/cells.exe` or `dist/windows/cells.exe` with its appropriate result folder.

The intended way to run this program to achieve functionality is using the simple command line interface we have provided. If the program is run with no parameters, nothing happens to the sample data.

## Command line interface

```sh
Usage: dist/linux/cells.exe [options]
Options:
  -h --help      Show this help message
  -s --sample-type       Set sample type
  -k --kernel    Set kernel type
  -z --kernel-size       Set kernel size
  -a --kernel-arg        Set kernel argument
  -b --kernel-arg2       Set kernel argument 2
  -m --method    Set method
  -i --input     Set input file
  -o --output    Set output file
  -p --pass-dir  Set pass directory
  -d --sample-dir        Set sample directory
  -r --result-dir        Set result directory
Sample types:
  0      EASY
  1      MEDIUM
  2      HARD
  3      EXTREME
Kernel types:
  0      NONE
  1      GAUSSIAN
  2      LAPLACIAN
  3      LoG
  4      DoG
Method types:
  0      NONE
  1      ERODE
  2      PEEKPOINTS
  3      GRADE
```

The program assumes the current working directory is in the same directory as the binary so you can run the program with minimal arguments as so:

```sh
cd dist/linux
./cells.exe -m 1 # Method one processes all the default samples (EASY) and outputs them in the default result directory "./res/<sample type>"
```

You can specify the sample type and method

```sh
cd dist/linux
./cells.exe -m 3 -s 3 # Use our best algoritm for counting cells on the IMPOSSIBLE samples. 
```

You can specify a single input and output that will be processed according to the mode.

```sh
./cells.exe -m 3 -i ../../samples/hard/1HARD.bmp -o ./output.bmp
```

Prefixing all the arguments with the `-p` flag produces debug output such as the erode passes for the erode method, the kernel passes for the default or specified kernels or the grade output for the grade method.

```sh
./cells.exe -p test -m 1 # Produces debug output for the erode method
```

By specifiying the `-d` and `-r` flag you can select where the samples are located and where the results folder that will mirror the sample folder structure will be located. This way you can run it outside the binary directory.

The kernel flags are for debugging and testing purposes, we specify the recommended kernel settings per default in the `main.c` file, but for testing pass them as so:

```sh
# note we do not specify a method as we want to process all the default samples with this kernel, this does not perform any detection.
./cells.exe -k 1 -z 5 -a 1 # Gaussian kernel with size 5 and sigma 1

# Here we specify the method for detection (peekpoints) which uses the output from after the kernel passes 
./cells.exe -m 2 -k 1 -z 7 -a 10 -k 2 -z 3 -a 1 -b 1 # Laplacian kernel with size 3 and sigma 1
```

All the methods have default kernel passes and the method specifies whether the counting and detection needs to happen.

> Note if no method is selected NO detection will take place, so remember to specify a method.

## Recommend use flow

1. Build the binary with make build
2. CD into the dist/linux directory
3. Run the following commands:
    - `./cells.exe -m 1 -s 0`
    - `./cells.exe -m 1 -s 1`
    - `./cells.exe -m 1 -s 2`
    - `./cells.exe -m 1 -s 3`
    - Observe the output of method 1
    - `./cells.exe -m 2 -s 0`
    - `./cells.exe -m 2 -s 1`
    - `./cells.exe -m 2 -s 2`
    - `./cells.exe -m 2 -s 3`
    - Observe the output of method 2
    - `./cells.exe -m 3 -s 0`
    - `./cells.exe -m 3 -s 1`
    - `./cells.exe -m 3 -s 2`
    - `./cells.exe -m 3 -s 3`
    - Observe the output of method 3

4. Play around with different kernel settings and method combinations on singular images, enable the `-p` flag in some of the previous commands to get more more debug output including snapshots of erode, debug passes of the kernel, a small thumbnail of the kernel and more.

This way you can observe how the program performs on all the samples at once, getting the count of cells in the terminal, and seeing the precision of that result by looking at the output images.
