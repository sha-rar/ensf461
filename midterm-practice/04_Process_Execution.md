# Section 4 — Process Execution (Q17–Q23)

## Q17. Process states

| Event | Transition | Explanation |
|---|---|---|
| 1. Scheduler selects a process | **Ready → Running** | It receives CPU execution time. |
| 2. Running process waits for disk I/O | **Running → Blocked** | It cannot proceed until the requested event completes. |
| 3. Timer preempts the running process | **Running → Ready** | The scheduler can choose another process; the preempted process remains runnable. |
| 4. I/O completes | **Blocked → Ready** | The process is eligible for scheduling but does not necessarily run immediately. |
| 5. Process exits | **Running → Terminated** | It stops executing. The OS performs exit bookkeeping and eventually reaps remaining process metadata. |

Only **ready** processes are candidates for the CPU. A blocked process typically needs an external event before it becomes ready. More elaborate operating-system models may include additional states.

## Q18. Programmes and processes

1. A **programme** is a stored set of instructions/data, such as an executable file. A **process** is a running or runnable instance of a programme together with an execution context and OS-managed resources. Several processes can execute the same programme at once.
2. Process state includes the **program counter**, CPU registers, stack contents, memory mappings/contents, scheduling status, open file descriptors and other kernel-maintained execution information. A context switch saves/restores the CPU execution state needed to continue the process.
3. A **PID** is a process identifier used by the OS to identify and manage a process. PIDs distinguish currently existing processes within the relevant PID namespace; numbers may be reused after processes exit.
4. The **program counter (PC)** / instruction pointer identifies the instruction address at which execution is to proceed. In x86-64 this register is called `RIP`.
5. Under the usual **System V AMD64** ABI used by 64-bit Linux, the first six integer/pointer arguments use `RDI`, `RSI`, `RDX`, `RCX`, `R8`, `R9` (subject to ABI classification); floating-point/vector arguments typically use XMM registers; other arguments may go on the stack. **Function-calling conventions and system-call conventions are not identical.**
6. Even if two processes originate from the same executable, they have distinct PIDs, register contexts, stacks, scheduling states and virtual-memory contexts. They may share read-only physical code pages or intentionally shared resources without becoming the same process.

## Q19. Understanding `fork()`

```c
pid_t pid = fork();
if (pid == 0) {
    printf("Child\n");
} else {
    wait(NULL);
    printf("Parent\n");
}
```

Assume the `fork()` succeeds and the terminal is used for output.

1. There are **two processes**: the original parent and one child.
2. In the **child**, `fork()` returns **0**, selecting the child branch.
3. In the **parent**, `fork()` returns the positive PID of the new child, selecting the `else` branch. On failure, `fork()` returns `-1` and no child is created; this question assumes success.
4. Guaranteed terminal output:
   ```text
   Child
   Parent
   ```
   The parent blocks in `wait(NULL)` until the child terminates. The child prints its line, exits, and then the parent prints its line.
5. Without `wait(NULL)`, the parent and child are not ordered by this synchronisation. **Either** `Child` or `Parent` may be printed first, depending on CPU scheduling. A parent that does not wait may also exit before its child.

## Q20. Process tree tracing

Programme:
```c
fork();
fork();
printf("Hello\n");
```

1. Immediately after both `fork()` statements have been completed by every participating process, **four processes** exist.
2. `Hello` is printed **four times**, once by each process (assuming successful calls and terminal output).
3. Tree, with symbolic process IDs:

   ```text
   P0  (original process)
   ├── P1  (created by P0's first fork)
   │   └── P3  (created by P1's second fork)
   └── P2  (created by P0's second fork)
   ```

4. After the first `fork`, **both P0 and P1** continue from the next statement. Each executes the second `fork`: P0 creates P2, and P1 creates P3. Thus the process count doubles **1 → 2 → 4**. There is no waiting, so the four output lines can arrive in any interleaved order, although they are identical.

## Q21. Understanding `exec()`

1. The `exec` family (such as `execl`, `execvp` and underlying `execve`) **replaces the calling process's programme image** with a newly loaded executable. Programmes often call `fork` first and invoke `exec` in the child.
2. **No.** A successful `exec` does **not** itself create an additional process; it reuses the calling process. Creating another process is `fork`'s role in the familiar UNIX pattern.
3. **No.** The PID normally stays the same because it is still the same process identity.
4. The process's previous code, data, heap and user stack are replaced by the new programme image, including a new initial stack. Details such as open file descriptors are inherited or closed according to rules including close-on-exec flags.
5. **No.** On success, control starts in the newly loaded programme; the old instruction after `exec` is never reached. On failure, an `exec` function returns `-1` and sets `errno`, allowing error handling.

Example usage:
```c
pid_t pid = fork();
if (pid == 0) {
    execlp("ls", "ls", "-l", (char *)NULL);
    perror("execlp");  // Reached only on failure.
    _exit(127);
} else if (pid > 0) {
    wait(NULL);
} else {
    perror("fork");
}
```

## Q22. Understanding `wait()`

```c
pid_t pid = fork();
if (pid == 0) {
    sleep(2);
    printf("C\n");
} else {
    printf("P1\n");
    wait(NULL);
    printf("P2\n");
}
```

Assume successful calls, ordinary terminal line-buffered output, and that `wait` completes by observing the child exit.

1. The **guaranteed constraints** are `P1` **before** `P2`, and `C` **before** `P2`. `P1` versus `C` is not strictly guaranteed by the programme: although the child's `sleep(2)` makes `P1` first likely, a delayed parent could print `P1` after the child prints `C`. Valid outputs therefore include:
   ```text
   P1              C
   C               P1
   P2              P2
   ```
   The left column is one possible output (`P1`, `C`, `P2`); the right column is another (`C`, `P1`, `P2`).
2. `wait(NULL)` waits for a child to change to a waitable terminated state, collecting its exit status without storing it because the argument is `NULL`. If a child has already terminated, the call can return immediately; if no children exist it fails. It can also return early with an error if interrupted by a signal, which robust production code would check.
3. **No, not after a successful wait for this child.** The child's `printf("C\n")` happens before it returns from `main` and exits. The parent must observe termination before printing `P2`.
4. Waiting synchronises the parent with child termination and reaps a terminated child's status so that the child's zombie process entry does not remain indefinitely.

## Q23. Process trees and pipelines

Command:
```bash
ls -1 | grep '[.]c$'
```

1. A typical Bash shell creates a pipe and forks **two child processes**. In one child it arranges stdout to reference the pipe's write end and uses an `exec` function to run `ls -1`; in the other it arranges stdin to reference the read end and executes `grep '[.]c$'`. Unused pipe ends must be closed in each process (and in the parent).
2. Typical process tree:
   ```text
   bash (parent)
   ├── ls -1 (child 1)
   └── grep '[.]c$' (child 2)
   ```
   The children are siblings; `grep` is not the child of `ls` in this typical construction.
3. `ls` writes directory entry names to stdout, which goes into the pipe buffer; `grep` reads this data from stdin and prints lines whose names **end with `.c`**. The regex `[.]` means a literal dot (whereas a bare `.` means any single character).
4. **No.** The commands can run concurrently, with the pipe coordinating production and consumption. If the pipe buffer becomes full, the writer can block; if empty while writers still exist, the reader can block.
5. The OS provides the system calls for pipe creation, process creation, descriptor duplication/closure, and executable replacement; it also schedules the processes and performs data transfers and blocking/waking as required. Bash orchestrates those system calls and normally waits for the pipeline's completion.
