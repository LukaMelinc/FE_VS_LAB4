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
    int d = 1280, v = 1024;
    int SIZEI = 640 * 480 * 3;  // Assuming RGB format (3 bytes per pixel)
    int SIZE_SCREEN = d * 2 - 1280;  // Not used in this example
    char *buf;

    // System V IPC keys and IDs
    key_t shm_key = ftok("shmfile",65);  // Same key as in the producer
    key_t sem_key = ftok("semfile",75);  // Same key as in the producer
    int shm_id = shmget(shm_key, SIZEI, 0666);
    int sem_id = semget(sem_key, 1, 0666);

    // Attach to shared memory
    buf = (char *) shmat(shm_id, NULL, 0);

    // Semaphore setup for synchronization
    struct sembuf sb = {0, -1, 0};  // P (wait) operation setup

    int fo = open("/dev/fb0", O_WRONLY);  // Framebuffer device file

    while(1) {
        semop(sem_id, &sb, 1);  // Wait for semaphore signal
        for (int i = 0; i < 480; i++) {
            // Write to framebuffer device
            int bytesWritten = write(fo, buf + i * 640 * 3, 640 * 3);  // Assuming each row is 640 pixels
            if (bytesWritten != 640 * 3) {
                fprintf(stderr, "Write error: Expected %d bytes, wrote %d bytes\n", 640 * 3, bytesWritten);
            }
            lseek(fo, 2 * 1280 * i, SEEK_SET);  // Adjust position for next write
        }
        lseek(fo, 0, SEEK_SET);  // Reset to beginning of framebuffer

        sb.sem_op = 1;  // V (signal) operation to let producer know we are done
        semop(sem_id, &sb, 1);  // Signal semaphore
    }

    close(fo);
    shmdt(buf);  // Detach from shared memory

    return 0;
}
