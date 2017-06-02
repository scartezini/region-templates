#include<iostream>
#include <map>
#include <list>
#include <iostream>
#include <fstream>

#include "PipelineComponentBase.h"
#include "RTPipelineComponentBase.h"
using namespace std;


namespace dbs{
	void dbs(map<int,PipelineComponentBase*> merged_stages, int index, int max_parallel_run);
	void make_fake_dependecy(map<int,PipelineComponentBase*> merged_stages, int index, int max_parallel_run);
};

namespace res{
	void print_merged_stages(map<int,PipelineComponentBase*> merged_stages);
};
