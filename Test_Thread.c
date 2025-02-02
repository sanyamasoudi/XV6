#include "types.h"
#include "user.h"
#include "stat.h"

#define NUM_THREADS 4
Lock My_Lock;
Lock Resource_Locks[NUM_THREADS];


void function(void *arg1, void *arg2) {
    int *ofs = (int *)arg1;  
    int *tid = (int *)arg2;  

    int resource_id = *tid;                
    int previous_resource = (resource_id == 0) ? NUM_THREADS - 1 : resource_id - 1;
    int buffer;                          
    

    // Request the current resource
    Lock_Acquire(&Resource_Locks[resource_id]);
    if (REQUEST(resource_id) == 0) {
        Lock_Acquire(&My_Lock);
        printf(1, "Thread %d: Resource %d granted\n", resource_id, resource_id);
        Lock_Release(&My_Lock);

        // Write the value X (2*thread ID+1) to the current resource
        buffer = 2*resource_id + 1;
        if(WRITE(resource_id, (char*)&buffer, *ofs,sizeof(int))==0){
            Lock_Acquire(&My_Lock);
            printf(1, "Thread %d: Wrote value %d to Resource %d\n", resource_id, buffer, resource_id);
            Lock_Release(&My_Lock);
        }
        else{
            Lock_Acquire(&My_Lock);
            printf(1, "Thread %d: FAILED Writing Resource %d\n", resource_id, resource_id); 
            Lock_Release(&My_Lock);
        }

        RELEASE(resource_id);
        Lock_Release(&Resource_Locks[resource_id]);
        // Synchronize with the previous thread before reading from the previous resource
        Lock_Acquire(&Resource_Locks[previous_resource]);

        // Read from the previous resource
        if (REQUEST(previous_resource) == 0) {
            buffer = 0;
            if(READ(previous_resource, *ofs, sizeof(int), (char *)&buffer)==0){
                Lock_Acquire(&My_Lock);
                printf(1, "Thread %d: Read value %d from Resource %d\n", resource_id, buffer, previous_resource);
                Lock_Release(&My_Lock);
            }
            else{
                Lock_Acquire(&My_Lock);
                printf(1, "Thread %d: FAILED reading from Resource %d\n", resource_id, previous_resource);
                Lock_Release(&My_Lock);
            }

            RELEASE(previous_resource);
        } else {
            Lock_Acquire(&My_Lock);
            printf(1, "Thread %d: Failed to request Resource %d for reading\n", resource_id, previous_resource);
            Lock_Release(&My_Lock);
        }

        Lock_Release(&Resource_Locks[previous_resource]);

    } else {
        Lock_Acquire(&My_Lock);
        printf(1, "Thread %d: Failed to request Resource %d\n", resource_id, resource_id);
        Lock_Release(&My_Lock);
    }
    Lock_Acquire(&My_Lock);
    printf(1, "Thread %d Finished\n", resource_id);
    Lock_Release(&My_Lock);
    Lock_Release(&Resource_Locks[resource_id]);
    exit();
}

int main() {

    int list[NUM_THREADS];     // List of thread IDs
    int tids[NUM_THREADS];     // Thread identifiers

    printf(1, "***This Program will calculate 2x+1 for 3 threads where x is the thread ID***\n");
    Lock_Init(&My_Lock);
    for (int i = 0; i < NUM_THREADS; i++) {
        Lock_Init(&Resource_Locks[i]);
    }

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        list[i] = i;  // Thread IDs (1, 2, 3)
        tids[i] = thread_create(&function, (void *)0, (void *)&list[i]);
    }

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        join(tids[i]);
    }

    exit();
}
