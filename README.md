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
(BSD)

```
Copyright (c) 2015, Mario Konrad
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software
   must display the following acknowledgement:
   This product includes software developed by Mario Konrad.
4. Neither the name of Mario Konrad nor the
   names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY MARIO KONRAD ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL MARIO KONRAD BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```
