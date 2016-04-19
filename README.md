# vcdiff
(c) CERN 2016

### Description
vcdiff is the ultimate VCD ([Value Change Dump][wiki_vcd]) file comparator. VCD
files store waveforms produced by HDL simulators (e.g. [GHDL][ghdl] or [Icarus
Verilog][icarus]).

The main reason to create another tool to perform the task, was to improve name
matching algorithm for variables. It turns out that different simulators may
produce not consistent naming schemes, and as the result, the variables cannot
be easily matched and compared.

### Installation
```
$ make
# make install
```

### Usage
See `vcdiff --help` for more details.

### FAQ
#### What is the difference about the variable matching algorithm?
The most common solution is to match variables by name. It is fine for the
majority of cases, especially for files that are created by the same simulator.

Problems start if simulators treat buses in a not consistent way. You may treat
`wire[7:0] var` (`var : std_logic_vector(7:0)` for VHDL people) either as a
single 8-bit bus or eight 1-bit wires. Depending on the convention your
variable will be named `var[7:0]` or `var[x]` where x=7..0. As the names differ,
the matching algorithm will fail to pair them.

vcdiff solves the problem by grouping wires that match the bus naming scheme
into a single variable. This way, regardles of the applied naming scheme, the
wires are always treated (and matched) as a bus.

#### What is the difference between 'state comparison' and 'transition comparison'?
The first method compares exclusively variable states at a certain moment, the
latter checks if the same transitions occur as well. The transition comparison
method shows more differences and is used by default.

Consider an example:
```
       time units: 012345689
(file1.vcd).var_a: _________
(file2.vcd).var_a: __/---\__
```

Variable (file1.vcd).var_a contains an impulse (lasting from #2 to #6), whereas
(file2.vcd).var_b stays at 0 all the time.

In the state comparison mode, variables state is checked after every transition
and marked as changed only if the variables are in different state. In the
example above, variable state differs at time unit #2 (file1.var_a = 0,
file2.var_a = 1), but at time unit #6 the variables are equal again, so they
are not displayed in the diff output.

See the vcdiff output (-s for enabling state comparison):
```
$ ./vcdiff -Wall -s file1.vcd file2.vcd
diff #2
==================
(file1.vcd).var_a   = 0
(file2.vcd).var_a   = 1
```

When we compare transitions, we will notice both the rising and the falling edge
of the impulse, as the transitions are different (to be more exact, there is no
transition in case of the first variable).

See the vcdiff output:
```
$ ./vcdiff -Wall file1.vcd file2.vcd
diff #2
==================
(file1.vcd).var_a   = 0
(file2.vcd).var_a   = 0 -> 1

diff #6
==================
(file1.vcd).var_a   = 0
(file2.vcd).var_a   = 1 -> 0
```

#### Why do I get question marks in certain transitions?
Transitions like the one below:
```
(file1.vcd).var_a   = ? -> 0
```
indicate the first signal assignment (initialization).

### Problems?
If you find a file that is not correctly handled by vcdiff, please let me know.

### Licence
GPLv3+

### Other tools
* [vcddiff](https://github.com/veripool/vcddiff)
* [vcddiff (another one)](https://github.com/palmer-dabbelt/vcddiff)
* [Compare VCD](https://sourceforge.net/p/comparevcd/comparevcdwiki/Home)

[wiki_vcd]: https://en.wikipedia.org/wiki/Value_change_dump
[ghdl]: http://ghdl.free.fr
[icarus]: http://iverilog.icarus.com
