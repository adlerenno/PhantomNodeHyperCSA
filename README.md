Phantom-Node HyperCSA (HyperRDF) Implementation
=========

This is the implementation of Phantom-Node HyperCSA by Cedric H. and ennoadler.


Installation
-----------

Follow the installation process of Succinct Data Structure Library SDSL (https://github.com/simongog/sdsl-lite).


Example data set
----------

The dataset used in the paper is 
```
1 2
2
1 3 5
3 4 3
5 5 5 5
```
and can also be found in ```data_sets/paper_example.raw```.


Compiling and using main (construction)
-----------
```console
mint@mint:~$ g++ -std=c++17 -O3 -DNDEBUG -I ~/include -L ~/lib main.cpp construct.cpp parser.cpp -o main -lsdsl -ldivsufsort -ldivsufsort64
mint@mint:~$ ./main data_sets/paper_example.raw paper_example.hrdf
Using values:
 - Input:  data_sets/paper_example.raw
 - Output: paper_example.hrdf
 - Data percentage: 100

reading 5 out of 5 total lines...
 - Parsing time: 0 ms
 - Linearization time: 0 ms
 - Construction time: 24 ms
 - Writing time: 0 ms
-> Total time: 24 ms
Output written to paper_example.hrdf
mint@mint:~$ 
```

Compiling and using query
-----------
```console
mint@mint:~$ g++ -std=c++17 -O3 -DNDEBUG -I ~/include -L ~/lib query.cpp parser.cpp -o query -lsdsl -ldivsufsort -ldivsufsort64
mint@mint:~$ ./query paper_example.hrdf 3
3 3 4
3 5 1
mint@mint:~$ ./query paper_example.hrdf 1 3 4
mint@mint:~$ 
```