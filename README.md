Implemented engine.c to match limit orders. The project comes from http://www.quantcup.org/home/spec.

Call:
$ make
$ ./test

to test
and

$ make all
$ taskset 2 ./score

to score.

Need some more optimization, some orders are taking a much longer time than mostothers.
