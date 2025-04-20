# vnwhite

Von Neumann whitener

For each pair of input bits, produce 0 or 1 bits of output according to:

    0 0 ==> (output nothing)
    1 0 ==> output 0 bit
    0 1 ==> output 1 bit
    1 1 ==> (output nothing)

From:

    http://en.wikipedia.org/wiki/Hardware_random_number_generator#Software_whitening

John von Neumann invented a simple algorithm to fix simple bias, and
reduce correlation: it considers bits two at a time, taking one of
three actions: when two successive bits are the same, they are not used
as a random bit, a sequence of 0,1 becomes a 1, and a sequence of 1,0
becomes a 0. This eliminates simple bias, and is easy to implement as
a computer program or in digital logic. This technique works no matter
how the bits have been generated. It cannot assure randomness in its
output, however. What it can do is (with significant loss) transform a
random stream with a frequency of 1's different from 50% into a stream
with that frequency, which is useful with some physical sources. When
the random stream has a 50% frequency of 1's to begin with, it reduces
the bit rate available by a factor of four, on average.


# To install

```sh
make clobber all
sudo make install clobber
```


# To use

```
/usr/local/bin/vnwhite [-h] [-v level] [-V] < input > output
```


# Reporting Security Issues

To report a security issue, please visit "[Reporting Security Issues](https://github.com/lcn2/vnwhite/security/policy)".
