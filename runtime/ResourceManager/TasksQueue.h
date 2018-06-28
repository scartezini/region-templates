/*
 * TasksQueue.h
 *
 *  Created on: Aug 17, 2011
 *      Author: george
 */

#ifndef TASKSQUEUE_H_
#define TASKSQUEUE_H_

#include <map>
#include <list>
#include <iostream>
#include <fstream>

#include <semaphore.h>
#include "Task.h"
#include "ExecEngineConstants.h"

class Task;

class TasksQueue {
protected:
	pthread_mutex_t queueLock;
	sem_t tasksToBeProcessed;
	int cpuThreads;
	int gpuThreads;

public:
	TasksQueue();
	virtual ~TasksQueue();

	// These methods are implemented in subclasses according to the type of queue chosen
	virtual bool insertTask(Task* task);
	// Remove task if available and returns a pointer
	virtual Task* getTask(int procType=ExecEngineConstants::CPU);
	// Remove task if found and returns a pointer
	virtual Task* getByTaskId(int id);


	virtual void retrieveResources(int res, int id);
	virtual void giveResources(int res);

	// Unlock threads that may be waiting at the getTask function
	void releaseThreads(int numThreads);

	void incrementTasksToProcess();

	// Return the number of tasks queued for execution
	virtual int getSize();

	virtual void getFrontBackSpeedup(float &front, float &back){};

};

class TasksQueueFCFS: public TasksQueue {
private:
	list<Task*> tasksQueue;

public:
	TasksQueueFCFS(int cpuThreads, int gpuThreads){
		this->cpuThreads = cpuThreads;
		this->gpuThreads = gpuThreads;
	}
	bool insertTask(Task* task);
	Task* getTask(int procType=ExecEngineConstants::CPU);
	int getSize();
	Task* getByTaskId(int id);

};

class TasksQueuePriority: public TasksQueue {
private:
	multimap<float, Task*> tasksQueue;

public:
	TasksQueuePriority(int cpuThreads, int gpuThreads){
		this->cpuThreads = cpuThreads;
		this->gpuThreads = gpuThreads;
	}
	bool insertTask(Task* task);
	Task* getTask(int procType=ExecEngineConstants::CPU);
	int getSize();
	void getFrontBackSpeedup(float &front, float &back);
};

//FIFO Memory block
class TasksQueueMB: public TasksQueue {
private:
	list<Task*> tasksQueue;
	list<Task*> memBlockQueue;

	list<int> runningId;


	int available;
	int curr = 0;

public:
	TasksQueueMB(int cpuThreads, int gpuThreads, int available){
		this->cpuThreads = cpuThreads;
		this->gpuThreads = gpuThreads;

		this->available = available;
	}

	bool insertTask(Task* task);
	Task* getTask(int procType=ExecEngineConstants::CPU);
	int getSize();
	Task* getByTaskId(int id);

	void retrieveResources(int memory, int id);
	void giveResources(int memory);

	void outRunning();

};


#endif /* TASKSQUEUE_H_ */
