#include"types.h"
#include"user.h"
#include"stat.h"

Lock P2_Perm, P1_Perm;
Lock Print_Lock;

void safe_printf(const char *msg) {
    Lock_Acquire(&Print_Lock);
    printf(1, "%s\n", msg);
    Lock_Release(&Print_Lock);
}

void p1(void *arg1, void *arg2) {
    safe_printf("Thread 1: Starting");

 
    if (REQUEST(0) == 0) {
        safe_printf("Thread 1: Acquired Resource 0");
    } else {
        safe_printf("Thread 1: Deadlock detected while requesting Resource 0");
        exit();
    }

    safe_printf("Thread 1: Holding Resource 0, attempting to acquire Resource 1");

    // Signal p2 to start
    Lock_Release(&P2_Perm);
    // Wait for p2 to proceed
    Lock_Acquire(&P1_Perm);

    // Attempt to request Resource 1
    if (REQUEST(1) == 0) {
        safe_printf("Thread 1: Acquired Resource 1");
    } else {
        safe_printf("Thread 1: Deadlock detected while requesting Resource 1");
        exit();
    }

    safe_printf("Thread 1: Completed execution");
    RELEASE(0);
    RELEASE(1);

    exit();
}

void p2(void *arg1, void *arg2) {
    safe_printf("Thread 2: Starting");

    // Request Resource 1
    if (REQUEST(1) == 0) {
        safe_printf("Thread 2: Acquired Resource 1");
    } else {
        safe_printf("Thread 2: Deadlock detected while requesting Resource 1");
        
        
        exit();
    }

    safe_printf("Thread 2: Holding Resource 1, attempting to acquire Resource 0");

    // Wait for p1 to proceed
    Lock_Acquire(&P2_Perm);
    // Signal p1 to proceed
    Lock_Release(&P1_Perm);

    // Attempt to request Resource 0
    if (REQUEST(0) == 0) {
        safe_printf("Thread 2: Acquired Resource 0");
    } else {
        safe_printf("Thread 2: Deadlock detected while requesting Resource 0");
        exit();
    }

    safe_printf("Thread 2: Completed execution");
    RELEASE(1);
    RELEASE(0);

    exit();
}

int main() {

    Lock_Init(&P2_Perm);
    Lock_Init(&P1_Perm);
    Lock_Init(&Print_Lock);  

    Lock_Acquire(&P2_Perm);
    Lock_Acquire(&P1_Perm);

    int id1 = thread_create(&p1, 0, 0);
    int id2 = thread_create(&p2, 0, 0);

    join(id1);
    join(id2);

    safe_printf("Main: Program completed");
    exit();
}
