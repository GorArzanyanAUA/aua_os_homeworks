
#include<stdio.h>
#include<stdlib.h>

struct Process
{
    int pid;
    int arrival_time;
    int burst_time;
    int turnaround_time;       // Time taken from process arrival to its completion.
    int waiting_time;
    int response_time;         // Time from process arrival to the first execution
};

// order[] holds indices into processes[] in the order they were executed.
void print_results(const char *name, struct Process processes[], int n, int order[]){
    printf("=== %s ===\n", name);

    printf("Gantt Chart: ");
    for (int i = 0; i < n; i++) {
        printf("|  P%d  ", processes[order[i]].pid);
    }
    printf("|\n");

    printf("PID     AT     BT     WT     TAT    RT\n");
    double total_wt = 0, total_tat = 0, total_rt = 0;
    for (int i = 0; i < n; i++) {
        struct Process p = processes[order[i]];
        printf("%-7d %-6d %-6d %-6d %-6d %-6d\n", p.pid, p.arrival_time, p.burst_time,
               p.waiting_time, p.turnaround_time, p.response_time);
        total_wt += p.waiting_time;
        total_tat += p.turnaround_time;
        total_rt += p.response_time;
    }

    printf("\nAverage Waiting Time: %.2f\n", total_wt / n);
    printf("Average Turnaround Time: %.2f\n", total_tat / n);
    printf("Average Response Time: %.2f\n\n", total_rt / n);
}

// qsort comparators can't take extra arguments, so the array being sorted is shared here.
static struct Process *sort_base;

static int by_arrival(const void *a, const void *b){
    int i = *(const int *)a, j = *(const int *)b;
    if (sort_base[i].arrival_time != sort_base[j].arrival_time)
        return sort_base[i].arrival_time - sort_base[j].arrival_time;
    return i - j;   // qsort is not stable, so break ties by input order
}

void fcfs_scheduler(struct Process processes[], int n){
    int order[n];
    for (int i = 0; i < n; i++) order[i] = i;

    sort_base = processes;
    qsort(order, n, sizeof(int), by_arrival);   // O(n log n)

    int time = 0;
    for (int i = 0; i < n; i++) {
        struct Process *p = &processes[order[i]];
        int start = time > p->arrival_time ? time : p->arrival_time;   // CPU may sit idle
        p->waiting_time = start - p->arrival_time;
        p->response_time = p->waiting_time;     // non-preemptive
        p->turnaround_time = p->waiting_time + p->burst_time;
        time = start + p->burst_time;
    }

    print_results("First Come First Served (FCFS)", processes, n, order);
}


void sjf_scheduler(struct Process processes[], int n){
    int order[n];
    int done[n];
    for (int i = 0; i < n; i++) done[i] = 0;

    int time = 0;
    for (int k = 0; k < n; k++) {
        // among processes that have already arrived, pick shortest burst (ties: earlier arrival)
        int best = -1;
        for (int i = 0; i < n; i++) {
            if (done[i] || processes[i].arrival_time > time) continue;
            if (best == -1
                || processes[i].burst_time < processes[best].burst_time
                || (processes[i].burst_time == processes[best].burst_time
                    && processes[i].arrival_time < processes[best].arrival_time))
                best = i;
        }

        if (best == -1) {   // nothing has arrived: CPU idles until the next arrival
            for (int i = 0; i < n; i++)
                if (!done[i] && (best == -1 || processes[i].arrival_time < processes[best].arrival_time))
                    best = i;
            time = processes[best].arrival_time;
        }

        struct Process *p = &processes[best];
        p->waiting_time = time - p->arrival_time;
        p->response_time = p->waiting_time;     // non-preemptive
        p->turnaround_time = p->waiting_time + p->burst_time;
        time += p->burst_time;

        done[best] = 1;
        order[k] = best;
    }

    print_results("Shortest Job First (SJF)", processes, n, order);
}

int main(void){
    int n;
    printf("Enter the number of processes: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "Number of processes must be a positive integer.\n");
        return 1;
    }

    struct Process processes[n];
    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;
        printf("Enter the arrival time and burst time for process %d: ", processes[i].pid);
        if (scanf("%d %d", &processes[i].arrival_time, &processes[i].burst_time) != 2
            || processes[i].arrival_time < 0 || processes[i].burst_time <= 0) {
            fprintf(stderr, "Arrival time must be >= 0 and burst time must be > 0.\n");
            return 1;
        }
    }
    printf("\n");

    fcfs_scheduler(processes, n);
    sjf_scheduler(processes, n);

    return 0;
}