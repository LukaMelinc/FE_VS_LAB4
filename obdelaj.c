#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

int main() {
    key_t shm_key = ftok("shmfile",65);
    key_t sem_key = ftok("semfile",75);
    int shm_id = shmget(shm_key, 640 * 480 * 3, 0666);
    int sem_id = semget(sem_key, 1, 0666);

    // Attach to shared memory
    char *buffer = (char *) shmat(shm_id, (void*)0, 0);

    // Semaphore lock operation setup
    struct sembuf sb = {0, 1, 0}; // Set to wait (P operation)

    // Display loop
    while(1) {
        semop(sem_id, &sb, 1); // Wait for semaphore signal
        // ... existing display code ...
        sb.sem_op = -1; // Set to signal (V operation)
        semop(sem_id, &sb, 1); // Signal semaphore
    }

    // Cleanup
    shmdt(buffer);

    return 0;
}
