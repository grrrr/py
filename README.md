# py/pyext for Python 3

## Credits

### py/pyext - python script objects for PD and Max/MSP

Copyright (c)2002-2020 Thomas Grill (gr@grrrr.org)

For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

Donations for further development of the package are highly appreciated.
Visit https://www.paypal.com/xclick/business=gr%40grrrr.org&item_name=pyext&no_note=1&tax=0&currency_code=EUR

More info in the previous `readme.txt`.
## Installation

### Pre-requisite
* A python3 environment;
* An installation of [flext](https://github.com/grrrr/flext/);
* cmake ;
* ninja (optional).
### Build with cmake

Thanks to the work of @fferri, the `CMakeLists.txt` has been largely purified.
The library can be build with wathever python3 environment ; to build with a conda environment for example, first activate that environment before running the following commands:
```bash
mkdir cmake-build
cd cmake-build
cmake .. \
-GNinja \ # optional, builds faster 
-DFLEXT_BUILD_TYPE=[SINGLE | MULTI | SHARED] # SINGLE is default
```

The variable `FLEXT_BUILD_TYPE` allows the user to choose which version of flext to use for the build. `MULTI` and `SHARED` enable thread usage.

### Build with pd-lib-builder

See `readme.txt`.
## Pd configuration & usage

### Configuration

You should create a folder to hold your python scripts - we'll call it the pd script folder; ideally, place it alongside the folder that natively contains the pd externals (for example at `~/Documents/Pd/externals`).

* Open puredata and go to `File > Preferences > Edit Preferences` (*might be something along the lines of `Paths` for older pd versions*);
* Add the script folder to your search path.

If you want to work with scripts that are located somewhere else on your machine, you can create a link to that file in the pd script folder. That way, you can modify your file in place and the changes will be taken into account in puredata without having to move your file over and over again.

### Usage

There are plenty examples of patches the `pd` folder of this repo. Try them out !