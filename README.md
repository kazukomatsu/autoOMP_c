Automatic OpenMP Insertion Tool -- AutoOMP
=================================

 AutoOMP is a tool for automatic insertion of OpenMP directives using
compiler messages of automatic optimizations. The compiler messages of
a particular compiler are helpful even for the optimizations on other
systems. AutoOMP realizes high performance portability with low
programming efforts by keeping these compiler messages as OpenMP
directives into a source code.

Description
------------

 Automatic OpenMP Insertion Tool (AutoOMP) is one of software products
developed by the Xevolver project supported by JST "Post-Peta" CREST.
The purpose of this project is to help migration of legacy HPC
applications to new-generation systems by improving their performance
portabilities across system generations.

AutoOMP is developed for automatic insertion of OpenMP directives to
keep compiler messages as OpenMP directives into a source code.  The
implementation of AutoOMP is built on top of the ROSE compiler
framework and uses compile messages of NEC SX compiler.  The compiler
messages from the particular compiler of the application could be
informative to improve the performance portability because clues for
the OpenMP parallelization obtained from the messages are often
effective for different systems as well as the target system. Thus,
AutoOMP utilizes the compiler messages to help a user easily identify
parallelizable loop nests.

Requirements
------------

* ROSE compiler infrastructure -- http://rosecompiler.org/

Usage
-----

 To use AutoOMP, firstly compiler messages should be obtained by using
NEC SX compiler.  Note that the name of compiler messages needs to be
compile_log.txt in the current version. For example, compiler messages
of `sample.c` can be obtained as follows.

```
    % sxcc sample.c > compile_log.txt
```

Then, the `autoOMP` command inserts OpenMP parallel directives into a
source code using the compile messages.  `autoOMP` generates the
inserted source file named as rose_(original_source_code). For
example, the automatic OpenMP insertion of `sample.c` can be performed
as follows.

```
    % autoOMP sample.c
    % ls rose_sample.c
```


License
-------
This project is released under the 3-clause BSD license.

Copyright (c) 2016, Kazuhiko Komatsu. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the
   distribution.

 - Neither the name of the <ORGANIZATION> nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
