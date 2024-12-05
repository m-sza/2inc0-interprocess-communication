/* 
 * Operating Systems  (2INCO)  Practical Assignment
 * Interprocess Communication
 *
 * Maciej Bober 1809628
 * STUDENT_NAME_2 (STUDENT_NR_2)
 *
 * Grading:
 * Your work will be evaluated based on the following criteria:
 * - Satisfaction of all the specifications
 * - Correctness of the program
 * - Coding style
 * - Report quality
 * - Deadlock analysis
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>      // for perror()
#include <unistd.h>     // for getpid()
#include <mqueue.h>     // for mq-stuff
#include <time.h>       // for time()

#include "messages.h"
#include "service1.h"

static void rsleep (int t);


int main (int argc, char * argv[])
{
   // Check for required arguments
    if (argc < 3) {
        perror("Message queues weren't provided");
        exit(EXIT_FAILURE);
    }

    // Open the response and service message queues
    mqd_t response_queue = mq_open(argv[1], O_WRONLY);
    if (response_mq == (mqd_t)-1) {
        perror("Failed to open response message queue");
        exit(EXIT_FAILURE);
    }

    mqd_t service_mq = mq_open(argv[2], O_RDONLY);
    if (service_mq == (mqd_t)-1) {
        perror("Failed to open service message queue");
        mq_close(response_mq);
        exit(EXIT_FAILURE);
    }

    WorkerJobMessage workerJobMessage;
    WorkerJobMessage resultMessage;
    unsigned int priority;

    // Main loop to receive and process messages
    while (mq_receive(service_mq, (char *)&workerJobMessage, sizeof(WorkerJobMessage), &priority) > 0) {
        resultMessage.id = workerJobMessage.id;
        resultMessage.data = service(workerJobMessage.data);

        // Simulate work
        rsleep(10000);

        // Send the result back through the response message queue
        if (mq_send(response_mq, (char *)&resultMessage, sizeof(resultMessage), 1) == -1) {
            perror("Failed to send result message");
        }
    }

    // Cleanup
    mq_close(response_mq);
    mq_close(service_mq);

    return 0;
}

/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time
 * between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
    static bool first_call = true;
    
    if (first_call == true)
    {
        srandom (time (NULL) % getpid ());
        first_call = false;
    }
    usleep (random() % t);
}
