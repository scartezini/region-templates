#ifndef DOT_H
#define DOT_H

void DEBUG_PCBLIST_TO_DOT(const std::string& filename,
                          const std::map<int, PipelineComponentBase*>& stages) {
    std::ofstream os(filename + ".dot");

    os << "digraph graphname {" << std::endl;
    os << "graph[compound=true];\nrankdir=\"LR\";" << std::endl;

    for (std::pair<const int, PipelineComponentBase*> s : stages) {
        os << "subgraph cluster" << s.second->getId() << " {" << std::endl;
        os << "label = \"Stage " << s.second->getId() << "\";" << std::endl;
        for (ReusableTask* t : s.second->tasks) {
            os << t->getId() << " [shape=box,label=\"" << t->getId() << "\\n"
               << t->getTaskName();
            os << "\\nsize: " << t->size() << "\"]; " << std::endl;

            if(t->parentTask != -1)
            {
                os << t->parentTask << " -> " << t->getId() << ";\n";
            }
        }
        os << 's' << s.second->getId() << " [shape=plaintext,label=\"\"]; "
           << std::endl;
        os << "}" << std::endl;
        if (auto reused = s.second->reused) {
            // os << 's' << s.second->getId() << " [shape=plaintext,label=\"" <<
            // s.second->getId() << "\\nReused:" << s.second->reused->getId();
            // os
            // << "\"]; " << std::endl;
            os << 's' << s.second->getId() << " -> s"
               << s.second->reused->getId();
            os << " [ltail=cluster" << s.second->getId() << " lhead=cluster"
               << s.second->reused->getId() << "];" << std::endl;
        }
    }
    os << "}" << std::endl;
}


void DEBUG_PCBLIST_TO_DOT(const std::string& filename,
                          const PipelineComponentBase* stage,
                          std::vector<int> hl) {
    static int count = 0;
    std::ofstream os(filename + std::to_string(stage->getId()) +
                     std::to_string((count++) + 1000) +".dot");

    os << "digraph graphname {" << std::endl;
    os << "graph[compound=true];\nrankdir=\"LR\";" << std::endl;

    os << "subgraph cluster" << stage->getId() << " {" << std::endl;
    os << "label = \"Stage " << stage->getId() << "\";" << std::endl;
    for (ReusableTask* t : stage->tasks) {
        os << t->getId() << " [shape=box,label=\"" << t->getId() << "\\n"
           << t->getTaskName() << "\\nsize: " << t->size() << "\"";

        if (std::find(hl.begin(), hl.end(), t->getId()) != hl.end()) {
            os << ",style=filled,color=seagreen";
        }

        os << "];" << std::endl;

        if(t->parentTask != -1)
        {
            os << t->parentTask << " -> " << t->getId() << ";\n";
        }
    }
    os << 's' << stage->getId() << " [shape=plaintext,label=\"\"]; "
       << std::endl;
    os << "}\n}" << std::endl;
}


void DEBUG_PCBLIST_TO_DOT_HIGHLIGHT(const std::string& filename,
                                    const PipelineComponentBase* stage,
                                    const std::string& traceFile) {
    std::ifstream stm(traceFile);
    std::string line;
    while(std::getline(stm, line)) {
        std::istringstream iss(line);

        int tmp;
        std::vector<int> vec;
        while(iss >> tmp) {
            vec.push_back(tmp);
        }

        DEBUG_PCBLIST_TO_DOT(filename, stage, vec);
    }

    DEBUG_PCBLIST_TO_DOT("dot/stage", stage, {});

}


#endif
