/*
 * uxos.h
 *
 * Created: 2/25/2017 9:39:29 AM
 *  Author: A42739
 */ 


#ifndef UXOS_H_
#define UXOS_H_

#include <stdint.h>

#include "uxos_settings.h"

typedef enum { TASK_READY, TASK_SEMAPHORE, TASK_QUEUE } UXOS_TaskStatus;

typedef struct UXOS_Tsk {
    void *sp;
    UXOS_TaskStatus status;
    struct UXOS_Tsk *next;
    void *status_pointer;
}UXOS_Task;

typedef void (*UXOS_TaskFn)(void);

extern UXOS_Task *uxos_current_task;


/**
 * \brief  initializes the uXOS Kernel
 * 
 * \param 
 * 
 * \return void
 */
void uxos_init(void);

/**
 * \brief Creates a new task, not safe?
 * 
 * \param task
 * \param sp
 * 
 * \return void
 */
void uxos_new_task(UXOS_TaskFn task, void *sp);

/**
 * \brief Puts uXOS in ISR mode.
 *        This assumes non-nested ISRs
 * 
 * \param 
 * 
 * \return void
 */
void uxos_isr_enter(void);

/**
 * \brief Leaves ISR mode, executes the dispatcher
 *        This assumes non-nestest ISRs
 * 
 * \param 
 * 
 * \return void
 */
void uxos_isr_exit(void);

#ifdef UXOS_SEMAPHORE

typedef struct {
    int8_t value;
}UXOS_Semaphore;

/**
 * \brief initializes a new semaphore
 * 
 * \param value
 * 
 * \return UXOS_Semaphore *
 */
UXOS_Semaphore *uxos_semaphore_init(int8_t value);

/**
 * \brief posts to a semaphore
 * 
 * \param sem
 * 
 * \return void
 */
void uxos_semaphore_post( UXOS_Semaphore *sem );

/**
 * \brief Pends from a semaphore
 * 
 * \param sem
 * 
 * \return void
 */
void uxos_semaphore_pend( UXOS_Semaphore *sem );

#endif //UXOS_SEMAPHORE

#ifdef UXOS_QUEUE

typedef struct {
    void **messages;
    uint8_t pendIndex;
    uint8_t postIndex;
    uint8_t size;
}UXOS_Queue;

/**
 * \brief Initialize a new Queue
 * 
 * \param messages
 * \param size
 * 
 * \return UXOS_Queue *
 */
UXOS_Queue *uxos_queue_init( void **messages, uint8_t size );

/**
 * \brief Posts to a queue
 * 
 * \param queue
 * \param message
 * 
 * \return void
 */
void uxos_queue_post( UXOS_Queue *queue, void *message );

/**
 * \brief Pends from a queue
 * 
 * \param queue
 * 
 * \return void *
 */
void *uxos_queue_pend( UXOS_Queue *queue );

#endif //UXOS_QUEUE

/**
 * \brief Runs the Kernel
 * 
 * \param 
 * 
 * \return void
 */
void uxos_run(void);

/**
 * \brief Runs the scheduler
 * 
 * \param 
 * 
 * \return void
 */
void uxos_schedule(void);

/**
 * \brief Dispatch the next task passed
 *        saving the context of the current task
 * 
 * \param next
 * 
 * \return void
 */
void uxos_dispatch(UXOS_Task *next);

#endif /* UXOS_H_ */