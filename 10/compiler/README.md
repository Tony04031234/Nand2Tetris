# Compiler #
Compiles programs written in the Jack programming language to the intermediatory Virtual Machine Language. Represents the projects for Week 10 and 11 of the NAND2Tetris course (see repo root for more info).

(✓ means that the test stage has been passed)

# Test-driven Development Process #

## Stage 1 : *Syntax Analysis*
The goal for week 10 is a compiler which analyses syntax via tokenising and parsing but does not yet produce Virtual Machine language output. To verify that the compiler is correct, it outputs to XML which can be compared to test files.

####Step 1 : *Tokeniser* 
* Create a Tokeniser which splits Jack files into their tokens *

* Test file 1 (Square Dance)

* Test file 2 (Test Array)

####Step 2 : *Expressionless Parser (XML output)* 
* Create a compilation engine, outputting to XML, which handles all input except expressions *

* Test file 1 (Expressionless Square-Dance)

####Step 3 : *Parser (XML output)* 
* Create a compilation engine, outputting to XML, which handles all input *

* Test file 1 (Square Dance)

* Test file 2 (Test Array)


## Stage 2 : *Code Generation*
