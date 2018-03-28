# WARPED [![Build Status](http://img.shields.io/travis/wilseypa/warped2/master.svg)](https://travis-ci.org/wilseypa/warped2)

A Parallel & Distributed Discrete Simulation Library

## Building

WARPED is built with Autotools, Python and a C++11 compiler.

### Prerequisites

WARPED requires an MPI implementation such as [MPICH](http://www.mpich.org/) or
[OpenMPI](http://www.open-mpi.org/).

If building from the git repository instead of a tarball, you  will need the GNU
Autotools tool-chain (including Automake, Autoconf, and Libtool).

WARPED supports `profile-guided partitioning` for which Python implementation is
needed along with the following packages:

*   [NumPy](http://www.numpy.org/)
*   [NetworkX](https://networkx.github.io/)
*   [Louvain Method](https://sites.google.com/site/findcommunities/)


### Building from the Git repository

To build from the git repository, first clone a local copy.

```
git clone https://github.com/wilseypa/warped2.git
```

You can run the Autotools build without any options, although specifying a prefix (install
location) is recommended.

```
autoreconf -i
./configure --prefix=$HOME/lib/warped2
make && make install
```

##### MPI
You may choose to specify the path to the MPI headers and libraries manually in the
configuration step. You can specify the path to the library file with the
`--with-mpi-libdir` configure option, and the header location with the `--with-mpi-includedir`
option.

```
./configure --with-mpi-includedir=/usr/include/mpich --with-mpi-libdir=/usr/lib/mpich --prefix=$HOME/lib/warped2
```

Replace the paths in the above example with the locations of the MPI libraries and headers
on your machine.

##### Python
You may choose to specify the path to the Python headers and libraries manually in the
configuration step. You can specify the path to the library file with the
`--with-python-libdir` configure option, and the header location with the `--with-python-includedir`
option.

```
./configure --with-python-includedir=/usr/include/python2.7 --with-python-libdir=/usr/bin/python2.7 --prefix=$HOME/lib/warped2
```

Replace the paths in the above example with the locations of the Python libraries and headers
on your machine.


### Building from a tarball

To build from a source tarball, first download and extract [the latest release from
GitHub](https://github.com/wilseypa/warped/releases). `cd` into the directory you
extracted the tarball to, and run the following commands:

```
export CXX=mpicxx
./configure --prefix=$HOME/lib/warped2 && make && make install
```

This will build and install the warped library to the path specified by the `--prefix`
configuration option. If you omit the prefix, the library will be installed to `/usr`.

### Silent Build Rules

Because the normal output of `make` is very verbose, WARPED is configured to use silent
build rules by default. To disable silent rules, pass the `--disable-silent-rules` flag to
`configure` or the `V=1` flag to `make`.

```
./configure --prefix=$HOME/lib/warped2 --disable-silent-rules
```
or

```
make V=1
```

### Debug Build Rules

WARPED can be configured to build in debug mode using the `--enable-debug` flag.

```
./configure --prefix=$HOME/lib/warped2 --enable-debug
```

## License

The WARPED code in this repository is licensed under the MIT license, unless otherwise
specified. The full text of the MIT license can be found in the `LICENSE.txt` file.

WARPED depends on some third party libraries which are redistributed in the `deps/`
folder. Each third party library used is licensed under a license compatible with the MIT
license. The licenses for each third party library can be found in their respective
directories.

