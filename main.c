#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "diag/trace.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#define CCM_RAM __attribute__((section(".ccmram")))
#define QUEUE_SIZE 3//you can change the queue size from here
#define SENDER_TASKS_COUNT 3
#define HIGH_PRIORITY_TASK_INDEX 0
#define LOW_PRIORITY_TASK_INDEX 1
#define LOWEST_PRIORITY_TASK_INDEX 2

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

// Sender task parameters
static const int32_t SENDER_TIMER_PERIODS_LOW[] = { 50, 80, 110, 140, 170, 200 };
static const int32_t SENDER_TIMER_PERIODS_HIGH[] = { 150, 200, 250, 300, 350, 400 };

// Receiver task parameters
#define RECEIVER_TIMER_PERIOD 100

// Global variables
SemaphoreHandle_t senderSemaphore[SENDER_TASKS_COUNT];
SemaphoreHandle_t RecieverSemaphore;
TimerHandle_t senderTimersHandlers[SENDER_TASKS_COUNT];
TimerHandle_t receiverTimerHandler;
TimerHandle_t senderTimers[SENDER_TASKS_COUNT];
TimerHandle_t receiverTimer;
int32_t sentMessagesCount[SENDER_TASKS_COUNT];
int32_t blockedMessagesCount[SENDER_TASKS_COUNT];
int32_t receivedMessagesCount;
int32_t totalSentMessagesCount;
int32_t totalBlockedMessagesCount;
QueueHandle_t Communication_Queue;//global queue
int task1_time_summation=0;
int task1_count=0;
int task2_time_summation=0;
int task2_count=0;
int task3_time_summation=0;
int task3_count=0;
int average_task1_time;
int average_task2_time;
int average_task3_time;

// Sender task function
void senderTask(void *parameters) {
	int32_t taskIndex = (uint32_t)parameters;
    char message[25];
    BaseType_t currentTime;

    while (1) {
    	xSemaphoreTake(senderSemaphore[taskIndex],portMAX_DELAY);
            currentTime = xTaskGetTickCount();

            // Compose the message with the current time
            sprintf(message, "Time is %lu", currentTime);
            // Send the message to the queue
                        if (xQueueSend(Communication_Queue, message, 0) == pdPASS) {
                            sentMessagesCount[taskIndex]++;
                            totalSentMessagesCount++;
                        }
                        else {
                            blockedMessagesCount[taskIndex]++;
                            totalBlockedMessagesCount++;
                        }
                }
            }

// Receiver task function
void receiverTask(void *param) {
    char message[25];

    while (1) {
    	xSemaphoreTake(RecieverSemaphore,portMAX_DELAY);
            // Read the message from the queue
            if (xQueueReceive(Communication_Queue, message, 0) == pdPASS) {
                receivedMessagesCount++;
            }
    }
}

int iterations_counter=-1;

// Reset function
void resetFunction(void) {
    int32_t i;

    if(iterations_counter==-1){
        	totalSentMessagesCount = 0;
        	    totalBlockedMessagesCount = 0;
        	    receivedMessagesCount = 0;
        	    xQueueReset(Communication_Queue);
        	    for (i = 0; i < SENDER_TASKS_COUNT; i++) {
        	            xTimerChangePeriod(senderTimers[i], pdMS_TO_TICKS(rand() % (SENDER_TIMER_PERIODS_HIGH[0] - SENDER_TIMER_PERIODS_LOW[0]) + SENDER_TIMER_PERIODS_LOW[0]), 0);
        	        }
        	        iterations_counter++;
        	    return;}

    int average_task_timer[SENDER_TASKS_COUNT]={(task1_time_summation/task1_count),(task2_time_summation/task2_count),(task3_time_summation/task3_count)};

    task1_time_summation=0;
    task1_count=0;
    task2_time_summation=0;
    task2_count=0;
    task3_time_summation=0;
    task3_count=0;
    // Print the total number of sent, blocked, and received messages

    printf("Total sent messages: %lu\n", totalSentMessagesCount);
    printf("Total blocked messages: %lu\n", totalBlockedMessagesCount);
    // Print statistics per sender task
    for (i = 0; i < SENDER_TASKS_COUNT; i++) {
        printf("Sender task %lu :-  Sent: %lu, Blocked: %lu\n", i, sentMessagesCount[i], blockedMessagesCount[i]);
    }
    printf("\n");
    if(iterations_counter>=5){
    	xTimerStop( receiverTimer,0 );
    	xTimerDelete( receiverTimer,0 );

    	for (i = 0; i < SENDER_TASKS_COUNT; i++){
    		xTimerStop( senderTimers[i],0 );
    		xTimerDelete( senderTimers[i],0 );
    	}

    	printf("Game Over\n");
    	exit(0);
    }
    // Reset counters
    totalSentMessagesCount = 0;
    totalBlockedMessagesCount = 0;
    receivedMessagesCount = 0;
    for (i = 0; i < SENDER_TASKS_COUNT; i++) {
            sentMessagesCount[i]=0;
            blockedMessagesCount[i]=0;
        }
    // Clear the queue
    xQueueReset(Communication_Queue);

    // Configure sender timer periods for the next values
    for (i = 0; i < SENDER_TASKS_COUNT; i++) {
        xTimerChangePeriod(senderTimers[i], pdMS_TO_TICKS(rand() % (SENDER_TIMER_PERIODS_HIGH[iterations_counter] - SENDER_TIMER_PERIODS_LOW[iterations_counter]) + SENDER_TIMER_PERIODS_LOW[iterations_counter]), 0);
    }
    iterations_counter++;
}

static void Sender1TimerCallback( TimerHandle_t xTimer ){

	int Random_period_sender1;
		Random_period_sender1 = SENDER_TIMER_PERIODS_LOW[iterations_counter] + rand()%(SENDER_TIMER_PERIODS_HIGH[iterations_counter]-SENDER_TIMER_PERIODS_LOW[iterations_counter]+1);
	    xTimerChangePeriod(xTimer,pdMS_TO_TICKS(Random_period_sender1),portMAX_DELAY);
	    xSemaphoreGive(senderSemaphore[0]);
	    task1_count++;
	    task1_time_summation+=Random_period_sender1;
}

static void Sender2TimerCallback( TimerHandle_t xTimer ){

	int Random_period_sender2;
		Random_period_sender2 = SENDER_TIMER_PERIODS_LOW[iterations_counter] + rand()%(SENDER_TIMER_PERIODS_HIGH[iterations_counter]-SENDER_TIMER_PERIODS_LOW[iterations_counter]+1);
	    xTimerChangePeriod(xTimer,pdMS_TO_TICKS(Random_period_sender2),portMAX_DELAY);
	    xSemaphoreGive(senderSemaphore[1]);
	    task2_count++;
	    task2_time_summation+=Random_period_sender2;
}

static void Sender3TimerCallback( TimerHandle_t xTimer ){

	    int Random_period_sender3;
			Random_period_sender3 = SENDER_TIMER_PERIODS_LOW[iterations_counter] + (rand()%(SENDER_TIMER_PERIODS_HIGH[iterations_counter]-SENDER_TIMER_PERIODS_LOW[iterations_counter]+1));
		    xTimerChangePeriod(xTimer,pdMS_TO_TICKS(Random_period_sender3),portMAX_DELAY);
		    xSemaphoreGive(senderSemaphore[2]);
		    task3_count++;
		    task3_time_summation+=Random_period_sender3;
}

static void ReceiverTimerCallback( TimerHandle_t receiverTimerHandler ){

	if(receivedMessagesCount >= 1000){
	            	  	resetFunction();}
	xSemaphoreGive(RecieverSemaphore);
}

int main(int argc, char* argv[])
{
	int32_t i;

	 // Create the queue
	Communication_Queue = xQueueCreate(QUEUE_SIZE, sizeof(char[25]));

	 // Start the sender tasks
		    xTaskCreate(senderTask, "SenderTask1", configMINIMAL_STACK_SIZE, (void *)0, 2 , senderTimersHandlers[0]);
		    xTaskCreate(senderTask, "SenderTask2", configMINIMAL_STACK_SIZE, (void *)1, 1 , senderTimersHandlers[1]);
		    xTaskCreate(senderTask, "SenderTask3", configMINIMAL_STACK_SIZE, (void *)2, 1 , senderTimersHandlers[2]);
     // Start the receiver task
		    xTaskCreate(receiverTask, "ReceiverTask", configMINIMAL_STACK_SIZE, (void *)NULL, 3, receiverTimerHandler);

	 // Create the semaphores and timers for sender tasks
		    for (i = 0; i < SENDER_TASKS_COUNT; i++)
		    vSemaphoreCreateBinary(senderSemaphore[i]);

	    senderTimers[0] = xTimerCreate("SenderTimer 1", (pdMS_TO_TICKS(SENDER_TIMER_PERIODS_LOW[0] + rand()%(SENDER_TIMER_PERIODS_HIGH[0]-SENDER_TIMER_PERIODS_LOW[0]+1))), pdTRUE, (void *)1, Sender1TimerCallback);
	    senderTimers[1] = xTimerCreate("SenderTimer 2", (pdMS_TO_TICKS(SENDER_TIMER_PERIODS_LOW[0] + rand()%(SENDER_TIMER_PERIODS_HIGH[0]-SENDER_TIMER_PERIODS_LOW[0]+1))), pdTRUE, (void *)1, Sender2TimerCallback);
	    senderTimers[2] = xTimerCreate("SenderTimer 3", (pdMS_TO_TICKS(SENDER_TIMER_PERIODS_LOW[0] + rand()%(SENDER_TIMER_PERIODS_HIGH[0]-SENDER_TIMER_PERIODS_LOW[0]+1))), pdTRUE, (void *)1, Sender3TimerCallback);

	 // Create the semaphores and timer for the receiver task
	    vSemaphoreCreateBinary(RecieverSemaphore);

	    receiverTimer = xTimerCreate("RecieverTimer", (pdMS_TO_TICKS(RECEIVER_TIMER_PERIOD)), pdTRUE, (void *)2, ReceiverTimerCallback);
	 // Initialize counters
	    for (i = 0; i < SENDER_TASKS_COUNT; i++) {
	        sentMessagesCount[i] = 0;
	        blockedMessagesCount[i] = 0;
	    }
	    totalSentMessagesCount = 0;
	    totalBlockedMessagesCount = 0;
	    receivedMessagesCount = 0;

	    for (i = 0; i < SENDER_TASKS_COUNT; i++){
	    	    xTimerStart(senderTimers[i], 0);}

	    	    xTimerStart(receiverTimer, 0);

	    // Set the reset function to be called when the program starts and when the receiver task receives 1000 messages
	    resetFunction();
	    // Start the FreeRTOS scheduler
	    vTaskStartScheduler();
	    // Should never reach here
	    return 0;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------

void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;

	/* This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amout of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}

void vApplicationTickHook(void) {
}

StaticTask_t xIdleTaskTCB CCM_RAM;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE] CCM_RAM;

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
  /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
  state will be stored. */
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  /* Pass out the array that will be used as the Idle task's stack. */
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
  Note that, as the array is necessarily of type StackType_t,
  configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTaskTCB CCM_RAM;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH] CCM_RAM;

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

