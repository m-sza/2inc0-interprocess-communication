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
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>    
#include <unistd.h>    // for execlp
#include <mqueue.h>    // for mq

#include <signal.h>


#include "settings.h"  
#include "messages.h"

#define GROUP_NUMBER 101
#define MQ_SIZE 30

char client2dealer_name[MQ_SIZE];
char dealer2worker1_name[MQ_SIZE];
char dealer2worker2_name[MQ_SIZE];
char worker2dealer_name[MQ_SIZE];

mqd_t client2dealer_mq;
mqd_t dealer2worker1_mq;
mqd_t dealer2worker2_mq;
mqd_t worker2dealer_mq;

int clientPID;
int worker1PIDs[N_SERV1];
int worker2PIDs[N_SERV2];

void debug_mq_status() {
    printf("Request MQ: %d\n", client2dealer_mq);
    printf("Worker1 MQ: %d\n", dealer2worker1_mq);
    printf("Worker2 MQ: %d\n", dealer2worker2_mq);
    printf("Response MQ: %d\n", worker2dealer_mq);
}

void create_message_queues() {
  // set names
  sprintf (client2dealer_name, "/client2dealer_%d_%d", GROUP_NUMBER, getpid());
  sprintf (dealer2worker1_name, "/dealer2worker1_%d_%d", GROUP_NUMBER, getpid());
  sprintf (dealer2worker2_name, "/dealer2worker2_%d_%d", GROUP_NUMBER, getpid());
  sprintf (worker2dealer_name, "/worker2dealer_%d_%d", GROUP_NUMBER, getpid());

  // client mq
  struct mq_attr attr;
  attr.mq_maxmsg  = MQ_MAX_MESSAGES;
  attr.mq_msgsize = sizeof (MQ_REQUEST_MESSAGE);
  client2dealer_mq = mq_open (client2dealer_name, O_RDONLY | O_CREAT | O_EXCL, 0600, &attr);

  // workers mq
  attr.mq_maxmsg  = MQ_MAX_MESSAGES;
  attr.mq_msgsize = sizeof (MQ_RESPONSE_MESSAGE);

  dealer2worker1_mq = mq_open (dealer2worker1_name, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);
  dealer2worker2_mq = mq_open (dealer2worker2_name, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);
  worker2dealer_mq = mq_open (worker2dealer_name, O_RDONLY | O_CREAT | O_EXCL, 0600, &attr);
}

void create_child_processes() {

  // create client
  clientPID = fork();
  if (clientPID < 0)
  {
      perror("fork() failed");
      exit (1);
  }
  if (clientPID == 0)
  {
    printf ("child  pid:%d\n", getpid());
    execlp ("./client", "./client", client2dealer_name, NULL);
    perror ("execlp() failed");
  }

  // create worker1
  for (int i = 0; i < N_SERV1; i++) {
    worker1PIDs[i] = fork();
    if (worker1PIDs[i] < 0)
    {
        perror("fork() failed");
        exit (1);
    }

    if (worker1PIDs[i] == 0)
    {
      printf ("child  pid:%d\n", getpid());
      execlp ("./worker_s1", "./worker_s1", worker2dealer_name, dealer2worker1_name, NULL);
      perror ("execlp() failed");
    }

  }

  // create worker2
  for (int i = 0; i < N_SERV2; i++) {
    worker2PIDs[i] = fork();
    if (worker2PIDs[i] < 0)
    {
        perror("fork() failed");
        exit (1);
    }

    if (worker2PIDs[i] == 0)
    {
      printf ("child  pid:%d\n", getpid());
      execlp ("./worker_s2", "./worker_s2", worker2dealer_name, dealer2worker2_name, NULL);
      perror ("execlp() failed");
    }

  }
}



void handle_queues() {
  MQ_REQUEST_MESSAGE req;
  MQ_RESPONSE_MESSAGE rsp;
  int client_alive = 1;
  int received;

  while (client_alive) {
    int status;
    if (waitpid(clientPID, &status, WNOHANG) > 0) {
        client_alive = 0;
    }
    // get request from client
    received = mq_receive(client2dealer_mq, (char *)&req, sizeof(req), NULL);
    if (received != -1) {
      printf("Router received request: job %d, service %d\n", req.jobID, req.serviceID);

      if (req.serviceID == 1) {
        mq_send(dealer2worker1_mq, (char *)&req, sizeof(req), 0);
        printf("Router forwarded job %d to Service 1\n", req.jobID);
      } else if (req.serviceID == 2) {
        mq_send(dealer2worker2_mq, (char *)&req, sizeof(req), 0);
        printf("Router forwarded job %d to Service 2\n", req.jobID);
      } else {
        exit(1);
      }
    }

    while (mq_receive(worker2dealer_mq, (char *)&rsp, sizeof(rsp), NULL) != -1) {
      printf("%d -> %d\n", rsp.jobID, rsp.data);
    }
  }

  for (int i = 0; i < N_SERV1; i++) {
    kill(worker1PIDs[i], SIGKILL);
    waitpid(worker1PIDs[i], NULL, 0);
  }
  for (int i = 0; i < N_SERV2; i++) {
    kill(worker2PIDs[i], SIGKILL);
    waitpid(worker2PIDs[i], NULL, 0);
  }
}

void clean_up_message_queues() {
  // close the mqs
  mq_close (client2dealer_mq);
  mq_close (dealer2worker1_mq);
  mq_close (dealer2worker2_mq);
  mq_close (worker2dealer_mq);

  mq_unlink (client2dealer_name);
  mq_unlink (dealer2worker1_name);
  mq_unlink (dealer2worker2_name);
  mq_unlink (worker2dealer_name);
}

int main (int argc, char * argv[])
{
  if (argc != 1)
  {
    fprintf (stderr, "%s: invalid arguments\n", argv[0]);
  }
  
  
  // TODO:
    //  * create the message queues (see message_queue_test() in
    //    interprocess_basic.c)
    create_message_queues();

    debug_mq_status();

    //  * create the child processes (see process_test() and
    //    message_queue_test())
    create_child_processes();
    //  * read requests from the Req queue and transfer them to the workers
    //    with the Sx queues
    //  * read answers from workers in the Rep queue and print them
    //  * wait until the client has been stopped (see process_test())
    handle_queues();
    //  * clean up the message queues (see message_queue_test())
    clean_up_message_queues();

    // Important notice: make sure that the names of the message queues
    // contain your goup number (to ensure uniqueness during testing)
  
  return (0);
}
