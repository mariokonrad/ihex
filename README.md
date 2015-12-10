ihex
====

Intel Hex fiie handling.

For a developer of embedded software, the Intel HEX format files are not unknown.
This tool manipulates the file as well as dumps information about it.

Use this software at your own risk.


Usage
=====

Show help information:
~~~~~~~~~~~~~~
	ihex --help
~~~~~~~~~~~~~~

Dump the file test.hex:
~~~~~~~~~~~~~~
	ihex --input test.hex --dump
~~~~~~~~~~~~~~

Dump the file test.hex to a specific file:
~~~~~~~~~~~~~~
	ihex --input test.hex --dump --output test.dump.txt
~~~~~~~~~~~~~~

Output the file test.hex as hex format file:
~~~~~~~~~~~~~~
	ihex --input test.hex --ihex
~~~~~~~~~~~~~~

Display information about its content:
~~~~~~~~~~~~~~
	ihex --input test.hex --info
~~~~~~~~~~~~~~

Erase memory region and output as hex file:
~~~~~~~~~~~~~~
	ihex --input test.hex --erase-region 0x00002000 --ihex
~~~~~~~~~~~~~~

Move memory region and output as hex file:
~~~~~~~~~~~~~~
	ihex --input test.hex --move-region 0x2000-0x4000 --ihex
~~~~~~~~~~~~~~

Move memory region and output as specific hex file:
~~~~~~~~~~~~~~
	ihex --input test.hex --move-region 0x2000-0x4000 --ihex --output test-new.hex
~~~~~~~~~~~~~~


Build
=====

With gcc (4.8 or newer):
~~~~~~~~~~~~~~
	g++ -o ihex ihex.cpp -Wall -Wextra -pedantic -O2 --std=c++11
	strip -s ihex
~~~~~~~~~~~~~~


LICENSE
=======

GPL 2 (see: http://www.gnu.org/licenses/gpl-2.0.html)

