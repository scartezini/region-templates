#include "death_breadth.hpp"

void dbs(map<int,PipelineComponentBase*> merged_stages, int index){

	cout << "Id: " << merged_stages[index]->getId() << "\n"; 
	for(int i = 0; i < merged_stages[index]->getNumberDependents(); i++){
	
		dbs(merged_stages,merged_stages[merged_stages[index]->getDependent(i)]->getId());
	}		
}	

