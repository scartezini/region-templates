#include "fake_dependency.hpp"
	
	

void dbs::make_fake_dependency(map<int,PipelineComponentBase*> merged_stages, int index, int max_parallel_run){
	PipelineComponentBase * parent = merged_stages[index];

	for(int i = 0; i < parent->getNumberDependents(); i++){
		if(i >= max_parallel_run){
		//	cerr << "addFakeDependency " << parent->getDependent(i) << " -> " << parent->getDependent(i%max_parallel_run) << endl; 
			merged_stages[parent->getDependent(i)]->addFakeDependency(parent->getDependent(i%max_parallel_run));
		}
	}	

}



void dbs::dbs(map<int,PipelineComponentBase*> merged_stages, int index, int max_parallel_run){

	for(int i = 0; i < merged_stages[index]->getNumberDependents(); i++){
		if(merged_stages[merged_stages[index]->getDependent(i)]->getNumberDependents() > 0){
			if(merged_stages[merged_stages[index]->getDependent(i)]->reused == NULL)
				dbs(merged_stages,merged_stages[merged_stages[index]->getDependent(i)]->getId(),max_parallel_run);
		}else{
		make_fake_dependency(merged_stages, index, max_parallel_run);
			return;	
		}
	}		
}	

void res::print_merged_stages(map<int,PipelineComponentBase*> merged_stages){
		for(pair<int,PipelineComponentBase*> p: merged_stages){
			if(p.second->reused == NULL){
				cout << "Id: " << p.second->getId() << "\n";
				cout << "Name: " << p.second->getName() << "\n";
				cout << "ComponentName: " << p.second->getComponentName() << "\n";


				cout << "Dependents: \n";
				for (int i=0; i<p.second->getNumberDependents(); i++) {
				//	if(merged_stages[p.second->getDependent(i)]->reused == NULL)
						cout << "\t" << p.second->getDependent(i)  << ": "
							<< merged_stages[p.second->getDependent(i)]->getName() << "\n";
				}	

				cout << "Dependency: \n";
				for (int i=0; i<p.second->getNumberDependencies(); i++) {
			//		if(merged_stages[p.second->getDependent(i)]->reused == NULL)
						cout << "\t" << p.second->getDependency(i)  << ": "
							<< merged_stages[p.second->getDependency(i)]->getName() << "\n";
				}	

				cout << "Fake: \n";
				for (int i=0; i<p.second->getNumberFakeDependencies(); i++) {
			//		if(merged_stages[p.second->getDependent(i)]->reused == NULL)
						cout << "\t" << p.second->getFakeDependency(i)  << ": "
							<< merged_stages[p.second->getFakeDependency(i)]->getName() << "\n";
				}	
				
				cout << "\n";
			}
		}
}
