// Write a program that:
// Calls multiple  fork syscalls one after another. Check what the process tree looks like.
// Pay attention to the hierarchy of processes, how many parents and children are created
// Add necessary checkings
#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <sys/wait.h>


void plain_forks (){
        
        //  So EVERY process runs every
        //  fork that is left : the population doubles 3 times. Letters are
        //  birth order, P0 is the original process :
        //
        printf("Start : pid %d, parent %d\n", getpid(), getppid());
        fflush(stdout);              // else this line is copied into every child

        int ret1 = fork();           // 1 -> 2 processes
        if (ret1 == -1) {
                perror("fork");
        }
        int ret2 = fork();           // 2 -> 4 processes
        if (ret2 == -1) {
                perror("fork");
        }
        int ret3 = fork();           // 4 -> 8 processes
        if (ret3 == -1) {
                perror("fork");
        }

        printf("Alive : pid %d, parent %d\n", getpid(), getppid());

        while (wait(NULL) > 0);      // every process reaps whatever it forked
}

void one_parent_multiple_childs () {

        //  time ------------------------------------------------>
        //
        //                 f1        f2        f3
        //  parent   ------+---------+---------+---[wait]--->
        //                 |         |         |
        //  child 3        |         |         \----------->
        //  child 2        |         \-------------------->
        //  child 1        \--------------------------->
        //
        //  All 3 forks run in the SAME process, so the 3 children are
        //  siblings : 4 processes, tree depth 1, one parent with 3 kids.
        printf("Process pid : %d\n", getpid());
        int ret1 = fork();
        if (ret1 > 0) {                       // parent
                int ret2 = fork();
                if (ret2 > 0) {              // parent
                        int ret3 = fork();
                        if (ret3 == 0) {
                                printf("Third child pid : %d, parent : %d\n", getpid(), getppid());
                        } else if (ret3 == -1) {
                                perror("fork");
                                while (wait(NULL) > 0);   // still reap the first two
                        } else {             // original process : has all 3 children
                                while (wait(NULL) > 0);   // reap them all
                        }
                } else if (ret2 == 0) {
                        printf("Second child pid : %d, parent : %d\n", getpid(), getppid());
                } else {
                        perror("fork");
                        while (wait(NULL) > 0);           // still reap the first child
                }
        } else if (ret1 == 0){               // child 
                printf("First child pid : %d, parent : %d\n", getpid(), getppid());
        } else {                             // error : no children to reap
                perror("fork");
        }
}

void grandpa_son_grandson_ggrandson (){

        //  time ------------------------------------------------>
        //
        //                 f1        f2        f3
        //  grandpa  ------+----------------------------->
        //                 |
        //  son            \---------+--------------------->
        //                           |
        //  grandson                 \---------+----------->
        //                                     |
        //  ggrandson                          \----------->
        //
        //  Each fork runs in the CHILD made by the previous one, so the
        //  processes form a line : 4 processes, tree depth 3, every
        //  process has exactly 1 child.
        printf("Grandpa pid : %d\n", getpid());
        int ret1 = fork();
        if (ret1 == 0) {                       // son
                printf("Son pid : %d, parent : %d\n", getpid(), getppid());
                int ret2 = fork();
                if (ret2 == 0) {               // grandson
                        printf("Grandson pid : %d, parent : %d\n", getpid(), getppid());
                        int ret3 = fork();
                        if (ret3 == 0) {       // great-grandson
                                printf("Great-grandson pid : %d, parent : %d\n", getpid(), getppid());
                        } else if (ret3 == -1) {
                                perror("fork");
                        } else {               // grandson waits for great-grandson
                                wait(NULL);
                        }
                } else if (ret2 == -1) {
                        perror("fork");
                } else {                       // son waits for grandson
                        wait(NULL);
                }
        } else if (ret1 == -1){                // error
                perror("fork");
        } else {                               // grandpa waits for son
                wait(NULL);
        }
}


int main(){
        // one_parent_multiple_childs();
        // grandpa_son_grandson_ggrandson();
        plain_forks();
        return 0; 
} 