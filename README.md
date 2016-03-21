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
