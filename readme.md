# The Feeny Programming Language #

Feeny is a small programming language designed to include the semantics of many popular dynamically-typed programming languages, such as Javascript, Ruby, and Python, and remain simple enough to implement in one semester. 

It was first introduced in the University of California, Berkeley graduate course *Virtual Machines and Managed Runtimes* taught by Mario Wolczko and Patrick S. Li. 

The course walks the class through a from-scratch implementation of Feeny, starting with a simple abstract syntax tree interpreter and ending with a just-in-time compiler. 

## Installing Stanza

There is an included Feeny parser and implementation written in L.B. Stanza. To compile the system from its sources, you will need to install Stanza. Please go to [lbstanza.org](http://lbstanza.org) to download it. Feeny has been verified to work with version `0.13.44` . Installation instructions for Stanza can be found [here](http://lbstanza.org/chapter1.html).

## Compiling the System

Run `scripts/make-feeny.sh` to compile the system. It produces two files: `bin/feeny` and `bin/cfeeny`. 

## Included Documentation

The course teaches students to ultimately implement a just-in-time compiler for Feeny through a series of exercises. The exercises are kept in the `docs` folder. 

## How to Run the Example System

**Evaluating**: The following command interprets the included sudoku program.

```
bin/feeny -e tests/sudoku.feeny
```

**Parsing:** The following command reads in the included sudoku program, and outputs the parse tree as a `.ast` file. The included C files are able to read in `.ast` files into an in-memory AST datastructure.

```
bin/feeny -i tests/sudoku.feeny -oast sudoku.ast
```

**Compiling:** The following command reads in the included sudoku program, and compiles it to a bytecode `.bc` format. The included C files are able to read in `.bc` files.

```
bin/feeny -i tests/sudoku.feeny -o sudoku.bc
```

**Display an AST File:** The following command reads in a `.ast` file and prints it to the screen.

```
bin/feeny -past sudoku.ast
```

**Display a Bytecode File:** The following command reads in a `.bc` file and prints it to the screen.

```
bin/feeny -pb sudoku.bc
```

**Execute a Bytecode File:** The following command executes a `.bc` file.

```
bin/feeny -rb sudoku.bc
```

**Example AST Interpreter**: The example C implementation of the Feeny AST interpreter can be executed using the following command:

```
bin/cfeeny -ast sudoku.ast
```

**Example Bytecode Interpreter:** The example C implementation of the Feeny bytecode interpreter can be executed using the following command:

```
bin/cfeeny -bc sudoku.bc
```

