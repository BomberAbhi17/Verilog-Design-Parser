# verilog-design-parser

## Embedded Systems Project Topic : Enumeration of design behaviors from RTL-based datapath-intensive designs

Developed a Verilog code Parser from scratch which parses
Datapath designs or datapath components of a design only . By datapath I mean
how input data passes through complex connections of Gates and operations to
give the final output .
● Input : Give a single verilog file to the parser
● Output : Parser outputs line by line explanation of datapath operations along with
design overview of circuit and each module in the verilog file .
● Limitation : The parser cannot take multiple verilog files at once and connect
them together to give an overall circuit . Meaning the whole RTL design should be
in a single verilog file and should not broken in various components.

##  How to Run the code on a verilog file ?

Requirements / Dependencies : GCC compiler for compiling the c file 
                                                 Terminal 

Step 1 : Open terminal on your system 

Step 2 : GO (cd) to the directory where the projectfinal.c is located .


Step 3 : TO BUILD THE PROJECT RUN : gcc -o veri parseVerilog.c

Step 4 : To run the program on a verilog file (say sample.v) RUN:  ./veri "sample.v"


The output for the given verilog design file is shown on terminal as output .
