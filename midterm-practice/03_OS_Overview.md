# Section 3 — Operating Systems Overview (Q12–Q16)

## Q12. Operating system structure

1. Three responsibilities are **process/CPU management**, **memory management**, and **persistent storage/file management**. The OS also provides device I/O, access control and system-call interfaces.
2. Applications generally run with restricted privileges and ask the kernel to perform protected operations. The kernel manages hardware resources, so applications can use abstractions such as processes, virtual memory, files and sockets without manipulating hardware directly.
3. A **device driver** is software that understands and operates a particular class of hardware, translating kernel I/O requests into device-specific operations and handling device responses/interrupts.
4. Hardware differs in its registers, protocols, commands and timing. Drivers encapsulate those specifics, letting higher OS layers provide comparatively uniform interfaces to applications. Depending on the OS architecture, drivers may run in kernel space or user space.

## Q13. Resource virtualisation

1. **CPU:** The OS can time-share one physical CPU among multiple runnable processes. By scheduling and context switching, each process experiences its own sequence of execution, giving the *illusion* of an independently available CPU. On multicore systems, some processes actually execute concurrently.
2. **Memory:** Each process sees a **virtual address space** rather than freely accessing physical RAM. The OS and hardware memory-management unit arrange translations/protections, creating the illusion of private, contiguous address ranges regardless of how backing memory is physically organised.
3. **Persistent storage:** The OS provides abstractions such as **files and directories** for data residing on disks/SSDs, instead of requiring applications to address individual device sectors. A shared filesystem lets multiple processes access common named data, subject to permissions and synchronisation.

The central theme is that virtualisation turns limited, complex hardware resources into manageable interfaces; it does **not** create unlimited physical capacity.

## Q14. User mode, kernel mode and traps

1. User code cannot directly execute privileged instructions or access protected kernel/device memory because doing so would undermine system isolation and stability. The CPU enforces privilege levels and memory protection.
2. A **trap** transfers execution through a defined entry mechanism to a kernel handler. A system call is commonly requested through such a controlled transition; the exact instruction and mechanics depend on the architecture.
3. Conceptual sequence:

   ```text
   User process executing
       |
       | requests system call (arguments placed as required by ABI)
       v
   CPU/system-call entry mechanism
       | saves necessary return state; enters privileged context
       v
   Kernel entry / trap handler
       | validates request and performs authorised OS work
       v
   Return-from-trap / system-call return mechanism
       | restores required saved state and user privilege
       v
   User process continues at the instruction after the call
   ```

   The exact division of register saving between hardware and kernel entry code differs by machine and syscall mechanism. Some calls block, permitting another process to run before the original returns.
4. Registers contain part of the process's execution state (including program counter and general-purpose values). Saving and restoring the relevant state allows execution to resume correctly instead of corrupting the user's computation.
5. **No.** The application requests an operation and passes parameters, but the CPU transfers control to a kernel-controlled entry point. The kernel chooses which privileged code runs and checks the request; the app cannot simply supply its own code to run with kernel privileges.

## Q15. Limited direct execution

1. **Limited direct execution (LDE)** lets ordinary user instructions run **directly on the CPU**, rather than having the OS interpret every instruction, while restricting privileged operations and retaining the means for the kernel to regain control.
2. Direct execution is efficient: normal arithmetic, branches and most memory operations can proceed at hardware speed without entering the kernel for each instruction.
3. Protection comes from user/kernel modes, controlled system-call entry, virtual memory protection and interrupts (especially the timer interrupt) that let the OS regain control. Faults and device interrupts can also invoke kernel handlers.
4. An **ordinary function call** branches to a function under normal application execution and normally retains the same privilege mode. A **system call** crosses into kernel execution through an OS-defined interface to request a privileged service. A library function may wrap a system call, but not every library function needs one.

## Q16. Virtual memory versus shared disk

1. Processes usually need independent memory for their code/data/stack and should not accidentally interfere with each other. Private virtual address spaces allow the OS to isolate those accesses, with deliberate shared mappings where required.
2. Files are intended to be persistent, named resources that can be read or updated by many programmes, including across separate executions. A shared filesystem therefore makes cooperative access normal, while managing permissions and consistency.
3. Unrestricted memory access would allow one buggy or malicious programme to corrupt another's data, read private information, overwrite control state or crash services. Protection and explicit sharing mechanisms reduce these risks.

**Comparison:** Private memory by default, shared memory deliberately; shared files by design, access-controlled by the OS. Neither model implies unrestricted access to everything.
