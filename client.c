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
#include "request.h"

static void rsleep (int t);


int main (int argc, char * argv[])
{
    // TODO:
    // (see message_queue_test() in interprocess_basic.c)
    //  * open the message queue (whose name is provided in the
    //    arguments)
    //  * repeatingly:
    //      - get the next job request 
    //      - send the request to the Req message queue
    //    until there are no more requests to send
    //  * close the message queue

    if (argc != 2)  // 2 arguments (program name, queue name)
    {
        fprintf(stderr, "%s: requires queue name as argument\n", argv[0]);
        exit(1);
    }

    mqd_t mq_fd_request;  // Message queue file descriptor

    // Open the queue for writing only
    mq_fd_request = mq_open(argv[1], O_WRONLY);

    // Check message queue
    if (mq_fd_request == (mqd_t)-1)
    {
        perror("mq_open() failed");
        exit(1);
    }

    MQ_REQUEST_MESSAGE msg;
    int result;

    // Send requests loop
    while ((result = getNextRequest(&msg.jobID, &msg.data, &msg.serviceID)) != NO_REQ)
    {
        if (mq_send(mq_fd_request, (char *)&msg, sizeof(msg), 0) == -1)
        {
            perror("mq_send() failed");
            break;
        }
    }

    // Close message queue
    mq_close(mq_fd_request);
    
    return (0);
}
