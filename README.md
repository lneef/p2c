# P2C-LLVM
### Data Generation:
```bash
cd data-generator
./generate-data.sh
```

This creates scale factor 1 TPC-H data in `data-generator/output/`.
The script first uses the `dbgen` tool to generate csv files, then reads and converts them to binary data.
## How to use
Make sure that you have installed llvm 21. 
Create a build-directory with `mkdir build` and run `cd build`. To generate the Makefile run `cmake ..`.  Simply running `make` compiles the query in `main.cc`. 

If you want to inspect the inspected code, make sure to compile the query engine in Debug mode. In this case the generated LLVM-IR is printed to stderr.

Before running any query make sure to set the environment variable `tpchpath` to the absolute path to the data set, e.g. `export tpchpath=/opt/tpch-hpqp/sf1/`. Otherwise the engine will be unable to access the dataset. The number of runs is adjusted via the environment variable `runs` and defaults to 3.

Currently it can compile an execute q01, q05, q06, q07, q08, q09, q12, q14, q17 and q19. A definition for q09 is given in `main.cc`.
