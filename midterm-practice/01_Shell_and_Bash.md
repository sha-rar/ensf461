# Section 1 — Shell and Bash (Q1–Q8)

These are worked solutions to the **Comprehensive Midterm Practice Quiz**. Code blocks can be run in a typical Bash/Linux environment unless otherwise noted.

## Q1. Working directory and command execution

1. **Print the working directory:**
   ```bash
   pwd
   ```
   This prints the shell's current working directory (initially `/home/student/projects` in the question).
2. **Navigate to the downloads directory:**
   ```bash
   cd /home/student/downloads
   ```
3. **Run the programme named `test` in the current directory:**
   ```bash
   ./test
   ```
   The file must exist and be executable (or be invoked using an appropriate interpreter).
4. **Why `test` differs from `./test`:** When a name has no slash, Bash resolves it using its command-resolution rules, including shell functions, builtins and directories in `PATH`. In particular, `test` is normally a shell builtin. `./test` contains a slash, so it explicitly refers to the executable file `test` in the current working directory rather than searching `PATH` or using that builtin.

## Q2. Environment variables and PATH

1. An **environment variable** is a named string value available in a process's environment, often used to configure programmes; examples include `HOME`, `PATH` and `LANG`. The shell can also hold ordinary shell variables that have **not** been exported into the environment.
2. `PATH` is a colon-separated list of directories to search for executable commands entered without a slash. For example, `/usr/bin:/bin` tells the shell where to look for external programmes.
3. On entering `ls`, Bash resolves the command (checking relevant shell constructs and then searching the `PATH` for an executable named `ls`) and launches it. Normally this runs the `ls` programme located in a directory such as `/usr/bin`.
4. Set a shell variable:
   ```bash
   COURSE=ENSF461
   ```
   **No spaces** are permitted around `=` in this assignment syntax.
5. Make it available to subsequently launched child processes:
   ```bash
   export COURSE
   ```
   Alternatively, `export COURSE=ENSF461` both assigns and exports it. A child inherits a *copy* of its parent's exported environment; changing a child's copy does not change the parent's variable.

## Q3. Standard streams and redirection

1. Standard streams:

   | Stream | File descriptor | Default use |
   |---|---:|---|
   | Standard input (`stdin`) | 0 | Reads input, often from the terminal |
   | Standard output (`stdout`) | 1 | Ordinary output, often to the terminal |
   | Standard error (`stderr`) | 2 | Error/diagnostic output, often to the terminal |

2. For each command:
   - `./program > output.txt`: redirects **stdout** into `output.txt`, creating or truncating the file. `stderr` is unchanged.
   - `./program 2> errors.txt`: redirects **stderr** into `errors.txt`. `stdout` is unchanged.
   - `./program > output.txt 2>&1`: first directs stdout to `output.txt`, then makes stderr refer to the **same open output destination** as stdout. Both streams are collected there.
   - `./program < input.txt`: sends bytes from `input.txt` to the programme's standard input.
3. In `./program > log.txt 2> log.txt`, each `>` opens `log.txt` **separately**, typically with truncation. The two file descriptors then have independent file offsets, so writes can overwrite one another or produce missing/corrupted output. It is not equivalent to making them share one destination.
4. Use:
   ```bash
   ./program > log.txt 2>&1
   ```
   Or, in Bash, `./program &> log.txt`. **Order matters:** `2>&1 > log.txt` first points stderr at the *old* stdout destination and only then redirects stdout, so stderr need not go into the file.

## Q4. Pipes

Command:
```bash
ls -1 /dev | grep tty | wc -l
```

1. `ls -1 /dev` lists directory entries in `/dev`, one per line. `grep tty` keeps lines containing the substring `tty`. `wc -l` counts newline characters in its input, giving a line count for conventional line-oriented output.
2. Each `|` connects **stdout of the left command** to **stdin of the right command** through a pipe. It does not, by default, pipe standard error.
3. The final number is the count of `/dev` entries whose displayed names contain `tty` (subject to what `ls` actually lists). It is not the total number of entries in `/dev`.
4. One suitable command is:
   ```bash
   grep 'error' server.log | wc -l
   ```
   This counts *lines* containing lowercase `error`, not the total number of occurrences. An alternative is `grep -c 'error' server.log`.

## Q5. Regular-expression construction

Assume **extended regular expressions** and a whole-string match. The answers below use the contiguous groups described by the question; there are no unspecified characters between required groups.

1. One or more digits then exactly three uppercase letters:
   ```regex
   ^[0-9]+[A-Z]{3}$
   ```
   `[0-9]+` means at least one digit; `[A-Z]{3}` means exactly three uppercase letters.
2. `M`, two to four digits, then `Z`:
   ```regex
   ^M[0-9]{2,4}Z$
   ```
3. The sequence `ab` repeated two to four times:
   ```regex
   ^(ab){2,4}$
   ```
   Parentheses group the two characters so the repetition applies to the entire sequence.
4. Exactly two uppercase letters, zero or more digits, and `xyz`:
   ```regex
   ^[A-Z]{2}[0-9]*xyz$
   ```

The anchors `^` and `$` prevent extra characters at the beginning/end. With GNU `grep -E`, braces and `+` have these extended-regex meanings without additional escaping.

## Q6. Regular-expression evaluation

Pattern:
```regex
^[A-Z]{2}[0-9]{2,4}$
```

It requires **exactly two uppercase letters**, followed by **between two and four digits**, and nothing else.

| String | Match? | Reason |
|---|---|---|
| `AB12` | **Yes** | Two uppercase letters and two digits. |
| `A123` | **No** | Only one uppercase letter. |
| `XY12345` | **No** | Five digits; maximum is four. |
| `ZZ999` | **Yes** | Two uppercase letters and three digits. |
| `AB12CD` | **No** | Trailing letters appear after the digits. |

- `^`: beginning of the line/string being matched.
- `$`: end of the line/string being matched.
- `+`: one or more occurrences of the preceding pattern.
- `*`: zero or more occurrences of the preceding pattern.
- `{2,4}`: between two and four occurrences, inclusive, of the preceding pattern.

## Q7. Manual pages

1. `man` displays reference documentation for commands, library functions, system calls and other interfaces. Manual pages commonly contain **NAME**, **SYNOPSIS**, **DESCRIPTION** and **OPTIONS** sections.
2. Open the `grep` manual:
   ```bash
   man grep
   ```
3. `man 2 fork` opens the **section 2** entry for `fork` (system calls); `man 3 malloc` opens the **section 3** entry for `malloc` (library functions). The numeric section selects the category of documentation. On Linux, `malloc` is provided by the C library, though the allocator may request additional memory from the kernel.
4. Read `man command` and check **SYNOPSIS** for argument structure and **OPTIONS** for command flags. `command --help` is also useful when supported; `man -k keyword` searches manual-page descriptions.

## Q8. Bash scripting

One complete solution:

```bash
#!/bin/bash

NUM=1

while [ "$NUM" -le 10 ]
do
    REMAINDER=$(expr "$NUM" % 2)

    if [ "$REMAINDER" -eq 0 ]; then
        echo "$NUM is even"
    else
        echo "$NUM is odd"
    fi

    NUM=$(expr "$NUM" + 1)
done
```

**How it works:** `NUM=1` is the starting value. The condition `[ "$NUM" -le 10 ]` checks whether the current value is at most 10, so the loop includes 10. `expr "$NUM" % 2` returns remainder zero for even integers and one for odd integers. The `if` chooses the corresponding output. `expr "$NUM" + 1` advances the counter, guaranteeing eventual termination.

Expected output:
```text
1 is odd
2 is even
3 is odd
4 is even
5 is odd
6 is even
7 is odd
8 is even
9 is odd
10 is even
```

The spaces inside `[ ... ]`, and between operands to `expr`, are syntactically important. `$(...)` is command substitution and captures the command's output. Bash does **not** require indentation to define blocks; `do`/`done` and `then`/`fi` do that.
