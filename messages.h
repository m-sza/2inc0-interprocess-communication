/* 
 * Operating Systems  (2INCO)  Practical Assignment
 * Interprocess Communication
 *
 * Maciej Bober (1809628)
 * Tomasz Soróbka (1809892)
 * Matyas Szabolcs (1835521)
 *
 * Grading:
 * Your work will be evaluated based on the following criteria:
 * - Satisfaction of all the specifications
 * - Correctness of the program
 * - Coding style
 * - Report quality
 * - Deadlock analysis
 */

#ifndef MESSAGES_H
#define MESSAGES_H

// define the data structures for your messages here
typedef struct {
    int jobID;
    int data;
    int serviceID;
} MQ_REQUEST_MESSAGE;

typedef struct {
    int jobID;
    int data;
} MQ_RESPONSE_MESSAGE;

#endif
