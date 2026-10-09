#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define min(a,b) (((a)<(b))?(a):(b))

// Total number of jobs read from the input file.
int numofjobs = 0;

struct job {
    int id;
    int arrival;
    int length;
    int tickets;
    int remaining;  // Time left to execute; 0 means completed.
    struct job *next;
};

// Linked list containing all jobs, in input-file order.
struct job *head = NULL;

void append_to(struct job **head_pointer, int arrival, int length, int tickets)
{
    struct job *new_job = malloc(sizeof(struct job));
    if (new_job == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    new_job->id = numofjobs++;
    new_job->arrival = arrival;
    new_job->length = length;
    new_job->tickets = tickets;
    new_job->remaining = length;
    new_job->next = NULL;

    if (*head_pointer == NULL) {
        *head_pointer = new_job;
        return;
    }

    struct job *current = *head_pointer;
    while (current->next != NULL)
        current = current->next;
    current->next = new_job;
}

void read_job_config(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    int tickets = 0;

    while ((nread = getline(&line, &len, fp)) != -1) {
        if (nread > 0 && line[nread - 1] == '\n')
            line[nread - 1] = '\0';

        char *arrival = strtok(line, ",");
        char *length = strtok(NULL, ",");
        if (arrival == NULL || length == NULL) {
            fprintf(stderr, "Invalid job entry in %s\n", filename);
            free(line);
            fclose(fp);
            exit(EXIT_FAILURE);
        }

        tickets += 100;
        append_to(&head, atoi(arrival), atoi(length), tickets);
    }

    free(line);
    fclose(fp);

    if (numofjobs == 0) {
        fprintf(stderr, "Job file is empty: %s\n", filename);
        exit(EXIT_FAILURE);
    }
}

// Return the earliest arrival time of any unfinished job that has
// not yet arrived. Returns INT_MAX if no such job exists.
int next_arrival_after(int time)
{
    int next_time = INT_MAX;
    for (struct job *j = head; j != NULL; j = j->next) {
        if (j->remaining > 0 && j->arrival > time && j->arrival < next_time)
            next_time = j->arrival;
    }
    return next_time;
}

// Select an arrived, unfinished job.
// shortest == 0: FIFO (earliest arrival first).
// shortest == 1: SJF/STCF (shortest remaining time first).
// Ties are resolved by arrival time, then job ID.
struct job *select_job(int time, int shortest)
{
    struct job *best = NULL;

    for (struct job *j = head; j != NULL; j = j->next) {
        if (j->arrival > time || j->remaining <= 0)
            continue;

        if (best == NULL ||
            (shortest && j->remaining < best->remaining) ||
            (!shortest && j->arrival < best->arrival) ||
            ((shortest ? (j->remaining == best->remaining) :
                         (j->arrival == best->arrival)) &&
              (j->arrival < best->arrival ||
               (j->arrival == best->arrival && j->id < best->id)))) {
            best = j;
        }
    }

    return best;
}

void print_run(int start, const struct job *j, int duration)
{
    printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n",
           start, j->id, j->arrival, duration);
}

void policy_FIFO(void)
{
    printf("Execution trace with FIFO:\n");

    int time = 0;
    int completed = 0;

    while (completed < numofjobs) {
        struct job *j = select_job(time, 0);
        if (j == NULL) {
            time = next_arrival_after(time);  // CPU is idle until then.
            continue;
        }

        print_run(time, j, j->remaining);
        time += j->remaining;
        j->remaining = 0;
        completed++;
    }

    printf("End of execution with FIFO.\n");
}

void policy_SJF(void)
{
    printf("Execution trace with SJF:\n");

    int time = 0;
    int completed = 0;

    while (completed < numofjobs) {
        struct job *j = select_job(time, 1);
        if (j == NULL) {
            time = next_arrival_after(time);
            continue;
        }

        // Non-preemptive: run this job to completion.
        print_run(time, j, j->remaining);
        time += j->remaining;
        j->remaining = 0;
        completed++;
    }

    printf("End of execution with SJF.\n");
}

void policy_STCF(void)
{
    printf("Execution trace with STCF:\n");

    int time = 0;
    int completed = 0;
    struct job *running = NULL;
    int run_start = 0;
    int run_duration = 0;

    while (completed < numofjobs) {
        // Reconsider which job should run after each arrival/completion.
        struct job *chosen = select_job(time, 1);

        if (chosen == NULL) {
            time = next_arrival_after(time);  // CPU is idle.
            continue;
        }

        // Output only when the running job changes or completes.
        // Do not split a continuous run just because a longer job arrives.
        if (running != chosen) {
            if (running != NULL)
                print_run(run_start, running, run_duration);
            running = chosen;
            run_start = time;
            run_duration = 0;
        }

        int duration = chosen->remaining;
        int next_time = next_arrival_after(time);
        if (next_time != INT_MAX)
            duration = min(duration, next_time - time);

        chosen->remaining -= duration;
        run_duration += duration;
        time += duration;

        if (chosen->remaining == 0) {
            print_run(run_start, chosen, run_duration);
            running = NULL;
            run_duration = 0;
            completed++;
        }
    }

    printf("End of execution with STCF.\n");
}

// Not implemented in this stage of the lab.
void policy_RR(int slice)
{
    (void)slice;
    printf("Execution trace with RR:\n");
    // TODO: implement Round Robin.
    printf("End of execution with RR.\n");
}

// Not implemented in this stage of the lab.
void policy_LT(int slice)
{
    (void)slice;
    printf("Execution trace with LT:\n");
    srand(42);
    // TODO: implement Lottery Scheduling.
    printf("End of execution with LT.\n");
}

void free_jobs(void)
{
    while (head != NULL) {
        struct job *next = head->next;
        free(head);
        head = next;
    }
}

int main(int argc, char **argv)
{
    static char usage[] = "usage: %s analysis policy slice trace\n";

    if (argc < 5) {
        fprintf(stderr, "missing variables\n");
        fprintf(stderr, usage, argv[0]);
        return EXIT_FAILURE;
    }

    int analysis = atoi(argv[1]);
    char *pname = argv[2];
    int slice = atoi(argv[3]);
    char *tname = argv[4];

    read_job_config(tname);

    if (strcmp(pname, "FIFO") == 0) {
        policy_FIFO();
    } else if (strcmp(pname, "SJF") == 0) {
        policy_SJF();
    } else if (strcmp(pname, "STCF") == 0) {
        policy_STCF();
    } else if (strcmp(pname, "RR") == 0) {
        policy_RR(slice);
    } else if (strcmp(pname, "LT") == 0) {
        policy_LT(slice);
    } else {
        fprintf(stderr, "Unknown scheduling policy: %s\n", pname);
        free_jobs();
        return EXIT_FAILURE;
    }

    if (analysis == 1) {
        // TODO: implement policy analysis (response, turnaround, wait).
    }

    free_jobs();
    return EXIT_SUCCESS;
}
