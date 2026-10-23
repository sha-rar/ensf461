# Section 5 — CPU Scheduling (Q24–Q32)

**Conventions used in these solutions:** Times are in CPU-time units. Intervals are half-open: `[start, end)`. Arrival time and total required CPU length are given. Assume zero context-switch overhead and a single CPU. When a job arrives during an RR quantum, it joins the back of the ready queue. In Q27 equal remaining times do not cause a preemption. In Q31 ties are broken alphabetically as required.

Useful definitions:

- **Completion time** (`C`): clock time when the job finishes.
- **Turnaround time** (`T`): `C − arrival`.
- **Response time** (`R`): `first CPU start − arrival`.
- **Waiting time** (`W`): for CPU-only jobs, `T − total CPU length`.

## Q24. Scheduling terminology

1. **Turnaround time:** time between arrival and completion, `completion − arrival`. It includes time executing, ready-queue time, and any I/O/blocking time.
2. **Response time:** time from arrival to **first** CPU execution, `first start − arrival`. It does not measure how soon the job finishes.
3. **Waiting time:** time a job spends in the ready queue waiting for CPU access. For the question's CPU-only jobs, `turnaround − CPU length`. For jobs involving I/O, that shortcut may subtract other non-wait time incorrectly if interpreted as ready-queue waiting.
4. **Context switch:** saving relevant execution state of one running process/thread and restoring another's so the processor can resume it. Switching has some overhead.
5. A **mechanism** is how the OS performs a switch (interrupts, save/restore registers, dispatcher); a **policy** is the rule choosing which runnable job executes next (FIFO, SJF, RR and so on).
6. A short average turnaround does not guarantee quick **first response**. For example, non-preemptive SJF might make an interactive job wait while an already-running long job finishes; STCF may also keep postponing a long job when short new jobs arrive.

## Q25. FIFO scheduling

| Job | Arrival | CPU length |
|---|---:|---:|
| A | 0 | 6 |
| B | 2 | 3 |
| C | 4 | 2 |
| D | 9 | 4 |

FIFO is **non-preemptive**: when the CPU is free, take the earliest-arrived waiting job; run it to completion.

**Trace:**

| Interval | Job | Reason |
|---|---|---|
| `[0, 6)` | A | Only job available at 0. |
| `[6, 9)` | B | Arrived at 2, ahead of C. |
| `[9, 11)` | C | Waiting since 4; D arrives at 9. |
| `[11, 15)` | D | Last job in arrival order. |

Gantt chart: `0 ─ A ─ 6 ─ B ─ 9 ─ C ─ 11 ─ D ─ 15`

**Calculations:**

| Job | First start | Completion C | Turnaround `C−arrival` | Response `start−arrival` |
|---|---:|---:|---:|---:|
| A | 0 | 6 | `6−0 = 6` | `0−0 = 0` |
| B | 6 | 9 | `9−2 = 7` | `6−2 = 4` |
| C | 9 | 11 | `11−4 = 7` | `9−4 = 5` |
| D | 11 | 15 | `15−9 = 6` | `11−9 = 2` |

- **Average turnaround:** `(6+7+7+6)/4 = 26/4 = 6.5`.
- **Average response:** `(0+4+5+2)/4 = 11/4 = 2.75`.

The CPU is never idle after time 0 because another job is available whenever the running job finishes.

## Q26. Non-preemptive SJF

| Job | Arrival | CPU length |
|---|---:|---:|
| A | 0 | 6 |
| B | 2 | 3 |
| C | 4 | 2 |
| D | 9 | 4 |

SJF selects the **shortest total CPU length among jobs currently ready** and runs that job to completion.

1. At `t=0` only A is ready, so A runs to 6. At `t=6`, B (3) and C (2) are ready, so choose C. At `t=8` choose B; the later arrival of D at 9 does not interrupt B. At 11 run D.

   | Interval | Job |
   |---|---|
   | `[0, 6)` | A |
   | `[6, 8)` | C |
   | `[8, 11)` | B |
   | `[11, 15)` | D |

2–3. Results:

   | Job | First start | C | Turnaround | Response |
   |---|---:|---:|---:|---:|
   | A | 0 | 6 | `6−0 = 6` | `0−0 = 0` |
   | B | 8 | 11 | `11−2 = 9` | `8−2 = 6` |
   | C | 6 | 8 | `8−4 = 4` | `6−4 = 2` |
   | D | 11 | 15 | `15−9 = 6` | `11−9 = 2` |

   - **Average turnaround:** `(6+9+4+6)/4 = 25/4 = 6.25`.
   - **Average response:** `(0+6+2+2)/4 = 10/4 = 2.5`.
4. Compared with FIFO, SJF reduces average turnaround from **6.5 to 6.25** and average response from **2.75 to 2.5** on *this dataset*. However, B now waits longer, showing that averages can improve even when an individual job does worse.
5. The scheduler cannot pick C at time 0 because C **has not arrived**. A policy can choose only among currently available jobs, not future jobs it might wish it had. SJF also depends on knowing/estimating lengths.

## Q27. Preemptive STCF

| Job | Arrival | CPU length |
|---|---:|---:|
| A | 0 | 9 |
| B | 2 | 3 |
| C | 3 | 1 |
| D | 11 | 2 |

STCF always prefers the job with the **smallest remaining CPU time**, checking whenever a job arrives or completes. On a tie, keep the current job as specified.

**Reason through each event:**

- `t=0`: A begins (9 units remaining).
- `t=2`: A has `9−2 = 7` left. B arrives needing 3; B is shorter, so **preempt A**.
- `t=3`: B has `3−1 = 2` left. C arrives needing 1; **preempt B**.
- `t=4`: C finishes. B (2 remaining) beats A (7), so resume B.
- `t=6`: B finishes. Resume A, which needs 7 more.
- `t=11`: A has run 5 more units, leaving `7−5 = 2`. D arrives needing 2. **Tie: do not preempt**, so A continues.
- `t=13`: A finishes; D runs until 15.

**Trace:**

| Interval | Job |
|---|---|
| `[0, 2)` | A |
| `[2, 3)` | B |
| `[3, 4)` | C |
| `[4, 6)` | B |
| `[6, 13)` | A |
| `[13, 15)` | D |

CPU lengths check: A `2+7=9`; B `1+2=3`; C `1`; D `2`. Total 15 units, matching the trace.

| Job | First start | C | Turnaround | Response |
|---|---:|---:|---:|---:|
| A | 0 | 13 | `13−0 = 13` | `0−0 = 0` |
| B | 2 | 6 | `6−2 = 4` | `2−2 = 0` |
| C | 3 | 4 | `4−3 = 1` | `3−3 = 0` |
| D | 13 | 15 | `15−11 = 4` | `13−11 = 2` |

- **Average turnaround:** `(13+4+1+4)/4 = 22/4 = 5.5`.
- **Average response:** `(0+0+0+2)/4 = 2/4 = 0.5`.
- **At `t=11`:** No preemption. A and D each have exactly 2 units remaining; the prescribed tie rule keeps A running. A different tie rule could change the later order, so the tie rule matters.

## Q28. Round Robin (quantum = 3)

| Job | Arrival | CPU length |
|---|---:|---:|
| A | 0 | 7 |
| B | 1 | 4 |
| C | 4 | 2 |
| D | 10 | 3 |

**Process the FIFO ready queue:** At the end of a full quantum, requeue the unfinished running process at the back; arrivals during the interval are already queued. The queue shown below is the state **before dispatching** the next job.

| Interval | Running job | CPU left afterwards | Ready queue after interval |
|---|---|---:|---|
| `[0, 3)` | A | A: 4 | `[B, A]` (B arrived at 1) |
| `[3, 6)` | B | B: 1 | `[A, C, B]` (C arrived at 4) |
| `[6, 9)` | A | A: 1 | `[C, B, A]` |
| `[9, 11)` | C | C: 0, finished | `[B, A, D]` (D arrived at 10) |
| `[11, 12)` | B | B: 0, finished | `[A, D]` |
| `[12, 13)` | A | A: 0, finished | `[D]` |
| `[13, 16)` | D | D: 0, finished | `[]` |

**Quantum-expiration queues:** after `t=3`: `[B,A]`; after `t=6`: `[A,C,B]`; after `t=9`: `[C,B,A]`. At `t=16`, D uses its full quantum **and finishes**, so it is not requeued (`[]`). The `[9,11)`, `[11,12)` and `[12,13)` intervals end on job completion, not quantum expiry.

Gantt chart: `0 A 3 B 6 A 9 C 11 B 12 A 13 D 16`.

**Completion and performance:**

| Job | First start | C | Turnaround | Response |
|---|---:|---:|---:|---:|
| A | 0 | 13 | `13−0 = 13` | `0−0 = 0` |
| B | 3 | 12 | `12−1 = 11` | `3−1 = 2` |
| C | 9 | 11 | `11−4 = 7` | `9−4 = 5` |
| D | 13 | 16 | `16−10 = 6` | `13−10 = 3` |

- **Average turnaround:** `(13+11+7+6)/4 = 37/4 = 9.25`.
- **Average response:** `(0+2+5+3)/4 = 10/4 = 2.5`.
- **If a process finishes early:** it releases the CPU **immediately**, is not requeued, and the scheduler can dispatch the next ready process. The scheduler does not waste the unused remainder of the quantum.

**Check:** Total requested CPU `7+4+2+3 = 16`, and the CPU is continuously busy from 0 to 16.

## Q29. Preemption and scheduling policies

1. **Preemptive** scheduling can suspend a running job before it finishes its current CPU work and give the CPU to another. **Non-preemptive** scheduling lets the chosen job retain the CPU until completion, blocking or voluntary relinquishment (under the simplified CPU-burst model).
2. Under the standard versions taught here: **FIFO and SJF are non-preemptive; STCF and RR are preemptive.**
3. **Cooperative scheduling** relies on a process voluntarily yielding or entering the OS (for example via `yield()` or a blocking system call); a fault can also return control to the kernel. A CPU-bound job that never yields can be problematic.
4. The kernel configures a **hardware timer** to interrupt CPU execution periodically. When the timer fires, the CPU enters an authorised kernel handler; the OS can account for CPU usage and reschedule a different process even if the current one did not yield.
5. The **convoy effect** arises when a long-running job at the front of a non-preemptive queue holds up many short jobs, producing unnecessary waiting and poor response/turnaround for them.
6. Exact CPU duration is often unknown until the job runs: input-dependent control flow, I/O, contention for resources, unpredictable interaction and varying workloads all affect completion time. SJF/STCF therefore often require predictions rather than perfect knowledge.

## Q30. Multi-Level Feedback Queue (MLFQ)

Given Q0: high priority, quantum 2; Q1: low priority, quantum 4. New jobs start in Q0; consuming an entire Q0 quantum demotes to Q1. Blocking for I/O before using all of Q0's quantum retains Q0 priority.

1. **Q0.** Every newly arriving job enters the highest priority queue under the given rules.
2. A **CPU-bound** job that consumes its full 2-unit quantum in Q0 is demoted to **Q1**. It then competes with lower-priority jobs for 4-unit time slices. (The question defines only two queues.)
3. An **interactive/I/O-bound** job often blocks before using a full high-priority quantum and so remains in Q0, giving it quicker access when it becomes ready. This allows the scheduler to approximate job behaviour **without knowing total job length in advance**.
4. A ready Q0 job takes precedence: the currently running Q1 job is **preempted** when Q0 becomes runnable, so Q0 can run. Details about whether the Q1 job resumes the unused portion of its quantum depend on additional policy rules, which the question does not specify.
5. If Q0 continues receiving runnable jobs, Q1 jobs may receive **very little or no CPU time**. That is starvation.
6. A periodic **priority boost** moves older jobs (or all jobs) back into higher-priority queues so that jobs stranded in Q1 eventually get opportunities to run. The exact frequency/triggers are implementation choices.

## Q31. Lottery and stride scheduling

| Job | Tickets | Share |
|---|---:|---:|
| A | 50 | 50/100 = **50%** |
| B | 30 | 30/100 = **30%** |
| C | 20 | 20/100 = **20%** |

**Part A — Lottery:**

1. There are `50+30+20=100` tickets total. In a fair draw, A wins with probability **0.50**, B **0.30**, C **0.20**.
2. Over many independent scheduling lotteries, the **expected** shares are approximately 50%, 30%, 20% of the slots (and CPU time if slots are equally long).
3. **No exact short-run guarantee.** A sequence of random draws can deviate substantially from expected proportions; the approximation improves as the number of lotteries grows.

**Part B — Stride:** With stride constant **600**:

4. `stride = 600 / tickets`, hence:

   | Job | Stride | Initial pass |
   |---|---:|---:|
   | A | `600/50 = 12` | 0 |
   | B | `600/30 = 20` | 0 |
   | C | `600/20 = 30` | 0 |

5–6. Choose the job with the **lowest current pass value**, alphabetically on ties, then increment **only that job's** pass by its stride:

   | Decision | Pass before `(A,B,C)` | Selected | Pass afterwards `(A,B,C)` |
   |---|---|---|---|
   | 1 | `(0,0,0)` | **A** | `(12,0,0)` |
   | 2 | `(12,0,0)` | **B** | `(12,20,0)` |
   | 3 | `(12,20,0)` | **C** | `(12,20,30)` |
   | 4 | `(12,20,30)` | **A** | `(24,20,30)` |
   | 5 | `(24,20,30)` | **B** | `(24,40,30)` |
   | 6 | `(24,40,30)` | **A** | `(36,40,30)` |

   **First six selections: A, B, C, A, B, A.** Count A=3, B=2, C=1 in these first six; these totals need not be exactly 50:30:20 at every prefix.
7. **Lottery** scheduling is probabilistic and aims at proportional CPU shares *in expectation*. **Stride** scheduling chooses the lowest pass value deterministically and keeps the allocation close to the ticket proportions with bounded short-term discrepancy (given equal-length quanta and the defined rules).

## Q32. Scheduling and I/O

1. Processes need more than CPU time. They may block for **disk/network I/O**, wait for synchronisation, and compete for other resources. Their CPU needs and arrivals can be unknown or variable, so the scheduler must respond to changes in which jobs are *ready*.
2. A process that requests blocking disk I/O normally moves **Running → Blocked** and stops using CPU while waiting for the operation.
3. The scheduler can execute **another ready process**, allowing CPU execution to overlap with the blocked process's I/O activity. When I/O completes, the blocked process becomes ready again.
4. **CPU-intensive** jobs execute for long stretches and can dominate a non-preemptive CPU. **I/O-intensive / interactive** jobs typically have shorter CPU bursts punctuated by waits and often benefit from prompt dispatch to reduce response latency. MLFQ attempts to distinguish these patterns from observed behaviour.
5. Comparison:

   | Policy | Preemptive? | Selection rule | Typical strengths/trade-offs | Length knowledge? |
   |---|---|---|---|---|
   | FIFO | No | Earliest ready arrival | Simple, but long jobs can create a convoy | No |
   | SJF | No | Shortest total CPU job **among ready jobs** | Low average turnaround when durations are known; poor responsiveness if long job already running | Yes/estimate |
   | STCF | Yes | Shortest **remaining** CPU time | Often strong average turnaround; a job may be repeatedly preempted; response can be poor for long jobs | Yes/estimate |
   | RR | Yes | Rotate FIFO queue after quantum | Distributes CPU opportunities and tends to improve interactive response; may add context-switch overhead | No |

   **Nuance:** No policy guarantees it beats every other policy on every workload or metric. A very small RR quantum incurs many context switches; a very large quantum starts resembling FIFO. STCF decisions must be reconsidered on arrivals and completions.
