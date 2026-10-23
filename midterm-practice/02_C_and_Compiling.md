# Section 2 — C and Compiling (Q9–Q11)

## Q9. Compiled versus interpreted languages

1. A **compiled** implementation translates source code to another representation (typically machine code) before it is executed. An **interpreted** implementation executes instructions using an interpreter at runtime; the interpreter reads/parses source or some intermediate representation and performs the requested actions. These are implementation strategies, not absolute properties of all languages.
2. In the course's simplified model, C source is compiled into machine code for the target architecture before it runs; Bash executes scripts through the Bash interpreter. Python commonly executes bytecode using an interpreter. Real implementations may additionally use bytecode compilation or just-in-time compilation, so the boundary is not perfectly rigid.
3. A C compiler translates and checks source code, reporting syntactic/semantic diagnostics and producing object code. The build process typically includes preprocessing, compilation, assembly and **linking** (combining object files and resolving external references) to produce an executable. The CPU runs the generated machine instructions.

## Q10. GCC

Given:
```bash
gcc -Wall -g -o program main.c
```

1. `gcc` invokes the GCC compiler driver; `-Wall` enables a standard set of useful warnings (not literally every possible warning); `-g` includes debugging information; `-o program` names the output file `program`; `main.c` is the source input.
2. Without `-o program`, compiling and linking a normal executable typically gives the default filename **`a.out`** on Linux.
3. `-c` compiles/assembles source into an **object file** without linking an executable. For example:
   ```bash
   gcc -c main.c
   ```
   normally creates `main.o`.
4. Compile/link two source files into one executable while enabling warnings:
   ```bash
   gcc -Wall -o application main.c helper.c
   ```
   GCC compiles each translation unit and links their object code into `application`. Adding `-g` would also make debugging easier, but was not required by the question.

## Q11. Stack versus heap variables

```c
#include <stdlib.h>

void example(void) {
    int a = 10;
    int *b = malloc(sizeof(int));
    *b = 20;
}
```

1. `a` is an **automatic local variable**: conceptually it belongs to this invocation's stack frame. An optimiser might hold it entirely in a CPU register; the teaching-level distinction is automatic/stack storage.
2. `b` itself is also an automatic local pointer variable, conceptually in the stack frame (or a register after optimisation).
3. Assuming `malloc` succeeds, the integer that receives `20` is in the **heap allocation** to which `b` points. The pointer and the pointed-to integer have different storage durations.
4. On return, `a`'s lifetime ends. Its stack-frame storage can be reclaimed automatically; accessing it later through any retained pointer would be invalid.
5. The heap allocation **is not automatically released** when the function returns. The local pointer `b` ceases to exist, but the heap object remains allocated. Because no surviving pointer refers to it, the allocation has been leaked.
6. The error is a **memory leak**, and there is a second potential error: dereferencing `b` if `malloc` returns `NULL`. Fix both by checking the result and freeing the allocation before return:

   ```c
   #include <stdlib.h>

   void example(void) {
       int a = 10;
       int *b = malloc(sizeof *b);
       if (b == NULL) {
           return; // Allocation failed; there is nothing to free.
       }
       *b = 20;
       free(b);
       (void)a; // Optional: suppress an unused-variable warning.
   }
   ```

   `sizeof *b` is the size of the pointed-to integer, equivalent here to `sizeof(int)`. A real programme that needs the allocated data beyond the function's return could instead transfer ownership to a caller that later invokes `free`.
