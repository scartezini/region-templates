/*
 * ExecutionEngine.cpp
 *
 *  Created on: Aug 17, 2011
 *      Author: george
 */

#include "ExecutionEngine.h"
//pthread_mutex_t ExecutionEngine::dependencyMapLock = PTHREAD_MUTEX_INITIALIZER;

ExecutionEngine::ExecutionEngine(int cpuThreads, int gpuThreads, int queueType, bool dataLocalityAware, bool prefetching) {
	schedType = queueType;

//	if(queueType ==ExecEngineConstants::FCFS_QUEUE){
//		tasksQueue = new TasksQueueFCFS(cpuThreads, gpuThreads);
//	}else if(queueType == ExecEngineConstants::MEM_BLOCK_QUEUE){
//		tasksQueue = new TasksQueueMB(cpuThreads, gpuThreads);
//	}else{
//		tasksQueue = new TasksQueuePriority(cpuThreads, gpuThreads);
//	}

	tasksQueue = new TasksQueueMB(cpuThreads, gpuThreads, 4);

	threadPool = new ThreadPool(tasksQueue, this);
	threadPool->createThreadPool(cpuThreads, NULL, gpuThreads, NULL, dataLocalityAware, prefetching);

	this->trackDependencies = new TrackDependencies();
}

ExecutionEngine::~ExecutionEngine() {
	delete threadPool;
	delete tasksQueue;
	delete trackDependencies;
}

//void *ExecutionEngine::getGPUTempData(int tid){
//	return threadPool->getGPUTempData(tid);
//}

bool ExecutionEngine::insertTask(Task *task)
{
#if DEBUG
	cout << "Insert: " << task->getId() << " : " << task->getCost() << endl;
#endif
	std::ofstream os;
	os.open("running", std::ios_base::app);
	os << "Insert: " << task->getId() << " : " << task->getCost() << endl;
	os.close();

	this->taskReferences[task->getId()] = TaskReferences(task);
	this->taskDependecies[task->getId()] = vector<int> (task->dependencies);

	task->curExecEngine = this;

	// Resolve task dependencies and queue it for execution, or left the task pending waiting
	this->trackDependencies->checkDependencies(task, this->tasksQueue);

	return true;
}


Task *ExecutionEngine::getTask(int procType)
{
	return tasksQueue->getTask(procType);
}

void ExecutionEngine::startupExecution()
{
	threadPool->initExecution();
}

void ExecutionEngine::endExecution()
{
	// this protection is used just in case the user calls this function multiple times.
	// It will avoid a segmentation fault
	if(threadPool != NULL){
		tasksQueue->releaseThreads(threadPool->getGPUThreads() + threadPool->getCPUThreads());
		delete threadPool;
	}
	threadPool = NULL;
}

void ExecutionEngine::resolveDependencies(Task *task){
	// forward message to track dependencies class
	this->trackDependencies->resolveDependencies(task, this->tasksQueue);
}

Task* ExecutionEngine::tryPreassignment(Task *task){
	// forward message to track dependencies class
	return this->trackDependencies->tryPreAssignement(task, this->tasksQueue, this->schedType);
}

int ExecutionEngine::getCountTasksPending() const
{
    return this->trackDependencies->getCountTasksPending();
}


void ExecutionEngine::waitUntilMinQueuedTask(int numberQueuedTasks)
{
	if(numberQueuedTasks < 0) numberQueuedTasks = 0;

	// Loop waiting the number of tasks queued decrease
	while(numberQueuedTasks < tasksQueue->getSize()){
		usleep(100000);
	}

}

void ExecutionEngine::startTransaction(CallBackTaskBase *transactionTask)
{
	this->trackDependencies->startTransaction(transactionTask);
//	if(this->transactionTask != NULL){
//		std::cout << "Error: calling startTranscation before ending previous transaction (endTransaction)" <<std::endl;
//	}
//	this->transactionTask = transactionTask;
}



void ExecutionEngine::endTransaction()
{
	this->trackDependencies->endTransaction();
}


void ExecutionEngine::retrieveResources(int id)
{

#if DEBUG
	cout <<  "Taks retrieve resources: " << id << endl;
#endif
	this->tasksQueue->retrieveResources(taskReferences[id].cost, id);
	if(taskReferences[id].nDep > 0)
		this->tasksQueue->giveResources(taskReferences[id].dataCost);

	vector<int>::iterator it;
	for(it = this->taskDependecies[id].begin(); it !=
		this->taskDependecies[id].end(); it++) {
#if DEBUG
		cout << "Dependecies: " << *it << endl;
#endif
		this->retrieveOutData(*it);
	}


}

void ExecutionEngine::retrieveOutData(int id)
{


	taskReferences[id].nDepFinished += 1;

#if DEFINE
	cout << "Task: " << id << endl;
	cout << "dependentsFinalized: " << taskReferences[id].nDepFinished <<
		"/" << taskReferences[id].nDep << endl;
#endif
	if(taskReferences[id].nDepFinished == taskReferences[id].nDep) {
		this->tasksQueue->retrieveResources(taskReferences[id].dataCost, id);
	}


}

TaskReferences::TaskReferences(Task* task)
{
	nDep = task->getNumberDependents();
	nDepFinished = task->getNumberDependentsFinished();
	dataCost = task->getDataCost();
	cost = task->getCost();
}
