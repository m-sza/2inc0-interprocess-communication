/* 
 * Operating Systems  (2INCO)  Practical Assignment
 * Interprocess Communication
 *
 * STUDENT_NAME_1 (STUDENT_NR_1)
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
#include "service2.h"

static void rsleep (int t);

char* name = "NO_NAME_DEFINED";
mqd_t dealer2worker;
mqd_t worker2dealer;


int main (int argc, char * argv[])
{

    // Check for required arguments.
    if (argc < 3) {
        perror("Message queues weren't provided");
        exit(EXIT_FAILURE);
    }

    // Open the response and service message queues
    mqd_t response_mq = mq_open(argv[1], O_WRONLY);
    if (response_mq == (mqd_t)-1) {
        perror("Failed to open response message queue");
        exit(EXIT_FAILURE);
    }

    mqd_t service_mq = mq_open(argv[2], O_RDONLY);
    if (service_mq == (mqd_t)-1) {
        perror("Failed to open service message queue");
        mq_close(response_mq); // Clean up response_mq if service_mq fails to open
        exit(EXIT_FAILURE);
    }

    MQ_REQUEST_MESSAGE reqMsg;
    MQ_RESPONSE_MESSAGE respMsg;
    unsigned int priority;

    // Receive messages and do jobs
    while (1) {  // Run indefinitely until killed by router
        ssize_t bytes_read = mq_receive(service_mq, (char *)&reqMsg, sizeof(MQ_REQUEST_MESSAGE), &priority);

        if (bytes_read == -1) {
            if (errno == EAGAIN || errno == EINTR) continue;
            perror("mq_receive failed");
            break;
        }

        printf("Worker received job: %d\n", reqMsg.jobID);
    
        // Process the message and send response
        respMsg.jobID = reqMsg.jobID;
        respMsg.data = service(reqMsg.data);
        
        rsleep(10000);

        if (mq_send(response_mq, (char *)&respMsg, sizeof(respMsg), 1) == -1) {
            perror("Failed to send result");
            continue;  // Try next message even if send fails
        }
    }

    // Close queues when done
    mq_close(response_mq);
    mq_close(service_mq);

    // TODO:
    // (see message_queue_test() in interprocess_basic.c)
    //  * open the two message queues (whose names are provided in the
    //    arguments)
    //  * repeatedly:
    //      - read from the S2 message queue the new job to do
    //      - wait a random amount of time (e.g. rsleep(10000);)
    //      - do the job 
    //      - write the results to the Rsp message queue
    //    until there are no more tasks to do
    //  * close the message queues

    return(0);
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
