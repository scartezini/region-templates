/*
 * TasksQueue.cpp
 *
 *  Created on: Aug 17, 2011
 *      Author: george
 */

#include "TasksQueue.h"


TasksQueue::TasksQueue() {
	pthread_mutex_init(&queueLock, NULL);
	sem_init(&tasksToBeProcessed, 0, 0);
}

TasksQueue::~TasksQueue() {
	pthread_mutex_destroy(&queueLock);
	sem_destroy(&tasksToBeProcessed);
}
bool TasksQueue::insertTask(Task *task)
{
	return true;
}

Task *TasksQueue::getTask(int procType)
{
	return NULL;
}

Task* TasksQueue::getByTaskId(int id)
{
	return NULL;
}

void TasksQueue::incrementTasksToProcess()
{
	sem_post(&tasksToBeProcessed);
}

int TasksQueue::getSize()
{
	return 0;
}

void TasksQueue::retrieveResources(int res)
{
	return;
}


bool TasksQueueFCFS::insertTask(Task *task)
{
	pthread_mutex_lock(&queueLock);

	if(task->getTaskType() ==  ExecEngineConstants::RT_CACHE_DUMP){
		tasksQueue.push_front(task);
	}else{
		tasksQueue.push_back(task);
	}

	pthread_mutex_unlock(&queueLock);
	sem_post(&tasksToBeProcessed);
	return true;
}

Task *TasksQueueFCFS::getTask(int procType)
{
	Task *retTask = NULL;
	sem_wait(&tasksToBeProcessed);
	pthread_mutex_lock(&queueLock);

	if(tasksQueue.size() > 0){
		retTask = tasksQueue.front();
//		tasksQueue.pop_front();
#ifdef	LOAD_BALANCING
		if(ExecEngineConstants::CPU == procType){
			float taskSpeedup = retTask->getSpeedup(ExecEngineConstants::GPU);
			if( this->gpuThreads* taskSpeedup > tasksQueue.size()){
				retTask = NULL;
			}
		}
#endif
		if(retTask != NULL)
			tasksQueue.pop_front();
	}
	pthread_mutex_unlock(&queueLock);
	return retTask;
}

int TasksQueueFCFS::getSize()
{
	int number_tasks = 0;
	pthread_mutex_lock(&queueLock);

	number_tasks = tasksQueue.size();

	pthread_mutex_unlock(&queueLock);

	return number_tasks;
}


Task* TasksQueueFCFS::getByTaskId(int id) {
	list<Task*>::iterator it;
	Task* retValue = NULL;
	pthread_mutex_lock(&queueLock);

	for(it = tasksQueue.begin(); it != tasksQueue.end(); it++){
		// if found, get pointer value, erase it from list, and stop searching.
		if((*it)->getId() == id){
			retValue = (*it);
			tasksQueue.erase(it);
			break;
		}
	}
	pthread_mutex_unlock(&queueLock);
	return retValue;
}

bool TasksQueuePriority::insertTask(Task *task)
{
	pthread_mutex_lock(&queueLock);

	float taskSpeedup = task->getSpeedup(ExecEngineConstants::GPU);
	tasksQueue.insert(pair<float,Task*>(taskSpeedup, task));

	pthread_mutex_unlock(&queueLock);
	sem_post(&tasksToBeProcessed);
	return true;
}
Task *TasksQueuePriority::getTask(int procType)
{
	Task *retTask = NULL;
	sem_wait(&tasksToBeProcessed);
	pthread_mutex_lock(&queueLock);

	int taskQueueSize = tasksQueue.size();
//	std::cout << "TasksQueuePriority::getTask. QueueSize="<< taskQueueSize << std::endl;

	if(taskQueueSize > 0){
		multimap<float, Task*>::iterator it;

		if(procType == ExecEngineConstants::GPU){
			it = tasksQueue.end();
			it--;
			retTask = (*it).second;
			tasksQueue.erase(it);
		}else{
			it = tasksQueue.begin();
			retTask = (*it).second;
#ifdef	LOAD_BALANCING
			float taskSpeedup = retTask->getSpeedup(ExecEngineConstants::GPU);
			printf("Balancing on\n");
			if( this->gpuThreads*taskSpeedup > taskQueueSize){
				retTask = NULL;
			}else{
#endif
				tasksQueue.erase(it);
#ifdef	LOAD_BALANCING
			}
#endif
		}
	}
	pthread_mutex_unlock(&queueLock);
	return retTask;
}

int TasksQueuePriority::getSize()
{
	int number_tasks = 0;
	pthread_mutex_lock(&queueLock);

	number_tasks = tasksQueue.size();

	pthread_mutex_unlock(&queueLock);

	return number_tasks;
}
void TasksQueuePriority::getFrontBackSpeedup(float& front, float& back) {
	front = -1.0;
	back = -1.0;

	pthread_mutex_lock(&queueLock);

	if(tasksQueue.size() > 0){
			multimap<float, Task*>::iterator it;
			it = tasksQueue.begin();
			front = (*it).second->getSpeedup(ExecEngineConstants::GPU);
			it = tasksQueue.end();
			it--;

			back = (*it).second->getSpeedup(ExecEngineConstants::GPU);
	}
	pthread_mutex_unlock(&queueLock);
	return;
}

/*Task *TasksQueuePriority::getTask(int procType)
{
	Task *retTask = NULL;
	sem_wait(&tasksToBeProcessed);
	pthread_mutex_lock(&queueLock);

	if(tasksQueue.size() > 0){
		multimap<float, Task*>::iterator it;

		if(procType == ExecEngineConstants::GPU){
			it = tasksQueue.end();
			it--;
			retTask = (*it).second;
			tasksQueue.erase(it);
		}else{
			it = tasksQueue.begin();
			retTask = (*it).second;
			tasksQueue.erase(it);
		}
	}
	pthread_mutex_unlock(&queueLock);
	return retTask;
}*/

void TasksQueue::releaseThreads(int numThreads)
{
	// Increment the number of tasks to be processed according to the
	// number of threads accessing this queue. So, all them will get
	// a NULL task, what is interpreted as an end of work.
	for(int i = 0; i < numThreads; i++){
		sem_post(&tasksToBeProcessed);
	}
}


bool TasksQueueMB::insertTask(Task *task)
{
	pthread_mutex_lock(&queueLock);

	this->memBlockQueue.push_back(task);

	pthread_mutex_unlock(&queueLock);
	sem_post(&tasksToBeProcessed);
	return true;

}


Task* TasksQueueMB::getTask(int procType)
{
	Task *retTask = NULL;
	sem_wait(&tasksToBeProcessed);
	pthread_mutex_lock(&queueLock);

	//Para cada elemento da lista de block
	//se puder se jogado para execução va
	for (list<Task*>::iterator it = memBlockQueue.begin(); it != memBlockQueue.end(); ++it ) {
		//if ((*it)->getCostlyPath() <= this->available) {
		if(true) {
			this->tasksQueue.push_back((*it));
			//cerr << "AVAILABLE: " << this->available << endl;
			this->available -= (*it)->getCost();
			//cerr << "COST: " << (*it)->getCost() << endl;
			//cerr << "AVAILABLE: " << this->available << endl;
			this->memBlockQueue.erase(it--);
		}
	}

	if(tasksQueue.size() > 0){
		retTask = tasksQueue.front();
		//		tasksQueue.pop_front();
#ifdef	LOAD_BALANCING
		if(ExecEngineConstants::CPU == procType){
			float taskSpeedup = retTask->getSpeedup(ExecEngineConstants::GPU);
			if( this->gpuThreads * taskSpeedup > tasksQueue.size()){
				retTask = NULL;
			}
		}
#endif
		if(retTask != NULL)
			tasksQueue.pop_front();
	}
	pthread_mutex_unlock(&queueLock);
	return retTask;

}

int TasksQueueMB::getSize()
{

	int number_tasks = 0;
	pthread_mutex_lock(&queueLock);

	number_tasks = tasksQueue.size() + memBlockQueue.size();

	pthread_mutex_unlock(&queueLock);

	return number_tasks;

}


void TasksQueueMB::retrieveResources(int memory)
{
	pthread_mutex_lock(&queueLock);

	this->available += memory;
	std::cerr << "Retornou: " << memory <<  "\t Available: " << this->available << '\n';

	pthread_mutex_unlock(&queueLock);

}

Task* TasksQueueMB::getByTaskId(int id){}
