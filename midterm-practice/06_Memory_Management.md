# Section 6 — Memory Management (Q33–Q38)

## Q33. Virtual address space

1. A **virtual address space** is the set of virtual memory addresses a process can use. The process issues virtual addresses; the processor's memory-management hardware and operating system translate/protect them using memory mappings. Not every virtual address is necessarily mapped.
2. Separate address spaces let each process maintain independent code, globals, heap and stack without arbitrary interference from other processes. The OS can explicitly arrange shared mappings when processes need them.
3. A **virtual address** is the address used by an application. A **physical address** identifies a location in actual physical memory. The OS and memory-management hardware establish translations. Different processes may use the *same virtual address* while mapping to different physical memory; several virtual addresses may also map to shared physical pages.
4. Typical regions:
   - **Code/text:** executable machine instructions, often read-only to the process and potentially shared across processes.
   - **Data:** static/global objects, including initialised data and uninitialised (`BSS`) data.
   - **Heap:** dynamically managed storage allocated using facilities such as `malloc`.
   - **Stack:** function activation records, local automatic variables and calling context.
5. Hardware access protection prevents a process from reading or writing memory to which its address space does not grant access. This both isolates processes and helps detect invalid accesses with faults. It is a protection mechanism, not an assertion that all address spaces have identical layouts.

A common *conceptual* layout for a process, from lower to higher addresses, is:

```text
Higher addresses
+------------------------------+
| Stack (commonly grows down)  |
|                 ↓            |
|          unused gap          |
|                 ↑            |
| Heap (commonly grows up)     |
+------------------------------+
| Static/global data           |
+------------------------------+
| Programme code               |
+------------------------------+
Lower addresses
```

Actual Linux address layouts include additional mappings and address-space randomisation; this is a teaching model rather than a fixed physical layout.

## Q34. Stack frames

```c
int add(int a, int b) {
    int result = a + b;
    return result;
}

int main(void) {
    int x = add(5, 7);
    return 0;
}
```

1. A **stack frame** (activation record) represents the stack storage associated with a particular function invocation. Calls create a logical calling context and returns remove it.
2. Depending on compiler, calling convention and optimisation, a frame can contain local variables, spilled register values, saved frame-pointer information, a return address, passed stack arguments, temporary storage and a stack canary. On x86-64, the first several arguments are commonly passed in registers, so `a` and `b` do not necessarily reside on the stack.
3. The **return address** identifies the place in the caller to resume after the called function finishes. On ordinary x86-64 `call` instructions, the CPU pushes a return address on the stack; `ret` uses it to return.
4. `rsp` points to the current **top of the stack**. When a typical downward-growing stack reserves space, `rsp` decreases; when that space is released, it increases. `rbp` is conventionally used as a relatively stable **frame pointer** to locate items in a frame. With frame-pointer omission/optimisation, a compiler may use `rbp` for another purpose instead.
5. The ordinary x86-64 process stack grows **towards lower addresses**.
6. After `add()` returns, its own activation record is no longer active and any stack space reserved for it becomes available for reuse. Control returns to `main()`, which can use the returned integer to initialise `x`. In a fully optimised build, the compiler might inline the function or keep everything in registers, so the physical frame may not exist.
7. A **stack canary** is a guard value placed near control data in some instrumented stack frames. Before returning, generated code checks whether the value has been corrupted, which can detect certain stack buffer overflows. A canary does not prevent or detect every memory error.

Conceptual unoptimised x86-64 frame:

```text
Higher addresses
| Caller-passed stack arguments (if any) |
| Return address                         |  <-- pushed by call
| Saved old rbp                          |  <-- when frame pointer used
| Optional canary                        |
| Local vars / spills / padding          |
| ...                                    |
Lower addresses (stack grows down ↓)
```

**Walk-through of this call:** `main` prepares arguments 5 and 7 according to the calling convention; `call add` establishes a return address; `add` computes `5+7=12`; it returns 12 in the ABI-specified return register (typically `eax` for an `int`); `main` continues after the call.

## Q35. Stack allocation

1. A **bump allocator** allocates by adjusting a pointer that marks the currently used boundary of a region. A stack allocation can commonly be implemented by moving its stack pointer; deallocation moves it back when the function returns.
2. Adjusting a register/pointer is typically very fast: unlike a general heap allocator, no search among arbitrary free blocks is necessary for each ordinary stack-frame allocation. The exact instructions and optimisations vary.
3. Stack allocation follows **last-in, first-out (LIFO)** lifetime rules across active frames. If `main` calls `f`, which calls `g`, `g` returns first, then `f`, and eventually `main`: the latest frame is removed first.
4. Arbitrary individual frees would leave holes among still-active frames and undermine the simple contiguous, pointer-based structure. A traditional stack therefore does not support `free(pointer)` for any arbitrary local allocation. Instead, space is reclaimed as the corresponding scope/frame unwinds.
5. Comparison:

   | Feature | Stack (automatic/call-frame storage) | Heap (dynamic storage) |
   |---|---|---|
   | Typical management | Compiler/runtime manages frames | Programmer/allocator manages allocations |
   | Typical lifetime | Until scope/function ends | Until explicitly freed (or process exits) |
   | Access | Local variables, return state, call context | Pointers to allocated objects |
   | Allocation/deallocation order | Normally LIFO | Arbitrary order possible |
   | Common error | Returning a pointer to expired local storage | Leak, double free, use-after-free, invalid free |
   | Speed | Usually very fast frame adjustment | Potentially more allocator bookkeeping |

## Q36. Dynamic memory allocation

A complete, compilable C solution:

```c
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int *numbers = malloc(5 * sizeof(int));

    if (numbers == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    for (int i = 0; i < 5; i++) {
        numbers[i] = i + 1;
    }

    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += numbers[i];
    }

    printf("Sum: %d\n", sum);

    free(numbers);
    numbers = NULL; // Optional: avoids leaving a dangling value locally.
    return 0;
}
```

**Step-by-step:**

1. `malloc(5 * sizeof(int))` requests enough bytes for **five integers**.
2. `numbers == NULL` detects allocation failure, so the programme does not dereference a null pointer.
3. The first loop sets elements to `1, 2, 3, 4, 5`.
4. The second loop computes `0+1+2+3+4+5 = 15`; output:
   ```text
   Sum: 15
   ```
5. `free(numbers)` releases the heap allocation after its last use.

`sizeof(int)` evaluates the number of bytes required for one `int` on this platform; **that size is not universally fixed at four bytes**. `5 * sizeof(int)` requests enough space for five objects regardless of the implementation's `int` size. An equally idiomatic expression is `malloc(5 * sizeof *numbers)`. `malloc` returns a `void *` in C, so no cast is needed.

## Q37. Memory-management errors

### Fragment A

```c
int *p = malloc(sizeof(int));
*p = 10;
free(p);
printf("%d\n", *p);
```

**Error: use-after-free.** After `free(p)`, the allocation no longer belongs to the programme; dereferencing `*p` has **undefined behaviour**, even if the bytes happen to still contain `10`. This example also needs a `NULL` check before the first dereference if allocation failure is possible.

**Fix:** Use the value **before** freeing and check the allocation:

```c
int *p = malloc(sizeof *p);
if (p == NULL) return;
*p = 10;
printf("%d\n", *p);
free(p);
p = NULL;
```

This correction is a snippet to place inside a `void` function; `return;` would need adapting in an `int`-returning function.

### Fragment B

```c
int *p = malloc(sizeof(int));
free(p);
free(p);
```

**Error: double free.** Assuming `malloc` succeeds, the second `free(p)` attempts to release an allocation that has already been freed. This is undefined behaviour and can damage allocator state or abort the programme.

**Fix:** Free once:

```c
free(p);
p = NULL;
```

`free(NULL)` is defined as a no-op, but setting the *one local pointer* to `NULL` does not repair any other aliases to the same freed object.

### Fragment C

```c
void function(void) {
    int *p = malloc(100);
    p = NULL;
}
```

**Error: memory leak** (assuming `malloc(100)` succeeds). `p = NULL` throws away the only pointer to a 100-byte allocation **without freeing it**. Losing a pointer is not the same as releasing the allocation.

**Fix:** If the allocation is not needed beyond the function:

```c
void function(void) {
    int *p = malloc(100);
    if (p == NULL) return;
    /* Use the allocation here if needed. */
    free(p);
    p = NULL;
}
```

You could also avoid allocating memory that you never intend to use.

### Fragment D

```c
void function(void) {
    int x = 5;
    free(&x);
}
```

**Error: invalid free.** `x` has automatic storage duration, not storage allocated using `malloc`/related allocator functions. Calling `free(&x)` has undefined behaviour because the address is **not a pointer returned by a dynamic allocation function**.

**Fix:** Remove the `free`:

```c
void function(void) {
    int x = 5;
    /* Use x; its lifetime ends automatically when the block ends. */
    (void)x;
}
```

If heap storage were required instead, dynamically allocate it and then free the returned heap pointer exactly once.

## Q38. Integrating stack and heap concepts

Original:
```c
int *create_number(void) {
    int number = 42;
    return &number;
}
```

1. `number` is an **automatic local variable** inside `create_number()`, conceptually allocated in the function's stack frame (or a register under optimisation).
2. Its lifetime ends **when the function returns**. Its storage may be reused for subsequent calls and is no longer a valid live `int` object.
3. `return &number` returns a pointer to that expired automatic variable. Dereferencing it later (such as through `p` in `main`) produces **undefined behaviour**. A compiler will often warn about returning the address of a local variable. The pointer being returned does **not** prolong the variable's lifetime.
4–5. Allocate an integer on the heap, check for failure, initialise it, pass its pointer back, and have the caller release it:

```c
#include <stdio.h>
#include <stdlib.h>

int *create_number(void) {
    int *p = malloc(sizeof *p);
    if (p == NULL) {
        return NULL;
    }
    *p = 42;
    return p;
}

int main(void) {
    int *p = create_number();
    if (p == NULL) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }

    printf("%d\n", *p); // Prints 42.
    free(p);            // Caller releases the heap object.
    p = NULL;           // Optional defensive reset.
    return 0;
}
```

6. `malloc()` and `free()` are commonly **C-library allocator functions**, not one-to-one wrappers that necessarily make a kernel system call every time. The allocator manages a heap area and may request or return virtual-memory regions from/to the OS using mechanisms such as `brk`/`sbrk` or `mmap`/`munmap`. A `free()` may merely make a block available for **reuse within the same process**, without immediately returning physical memory to the kernel. This is how user-managed heap allocation connects to OS virtual-memory management.

**Core lifetime contrast:** Returning a pointer to a local automatic variable is invalid after that function returns; returning a pointer to a successfully allocated heap object is valid until it is freed, and the caller must manage that lifetime.
