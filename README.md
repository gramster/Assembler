An assembler and linker for a RISC processor. When I started my Masters
in 1986 the plan was that we would build this chip and port UNIX to it.
At the time South Africa was under heavy sanctions and there was a 
concern at the university that we would lose access to American CPUs,
so the idea was to design and build a locally designed and manufactured
RISC microcomputer capable of running UNIX.

I designed the instruction set and encoding and wrote the assembler and
linker. Another person was building the C compiler and a third person
was supposed to do the silicon in an industry partnership, but they
dropped out, which meant the hardware would not materialize, and the
project was cancelled. After spending a year teaching compiler construction
to 3rd year and compiler optimization to 4th year students, I started a 
new graduate program building what became the 
[Protocol Engineering Workbench](https://github.com/gramster/PEW).

I used what I learned to create an assembler for 80386, also included here.
Note that I don't remember what state the latter is in, and if it is complete
or not. I wrote a simulator too, so that we could bootstrap the project 
while we waited for the silicon, but I don't seem to have a copy of that
code any more.