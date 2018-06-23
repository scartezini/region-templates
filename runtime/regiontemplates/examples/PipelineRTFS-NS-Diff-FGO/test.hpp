#ifndef TEST_HPP
#define TEST_HPP
#include <list>
#include <unordered_map>
#include <vector>
#include "ReusableTask.hpp"

class TaskReorder {
  public:
    TaskReorder(std::list<ReusableTask *> &tasks);

    void print(int origin = 0, int level = 0) const;

    void mergeTest();

    int firstLeaf(int node);

    void apply();

  private:
    class node {
      public:
        int id = 0;
        int parent = 0;
        std::vector<int> children;
        int height = 0;
        int level = 0;
        ReusableTask *prt = nullptr;
    };

    std::unordered_map<int, std::vector<int>> tree2;
    std::unordered_map<int, ReusableTask *> id2addr;
    std::unordered_map<int, node> tree;
    int root = 0;
};

void reorder_stages(std::map<int, PipelineComponentBase *> &stages) {
    for (std::pair<const int, PipelineComponentBase *> s : stages) {
        if (!s.second->tasks.empty()) {
            TaskReorder tr(s.second->tasks);
            tr.print();
            tr.mergeTest();
        }
    }
}

void prepare_stages(std::map<int, PipelineComponentBase *> &stages) {
    for (std::pair<const int, PipelineComponentBase *> s : stages) {
        if (!s.second->tasks.empty()) {
            TaskReorder tr(s.second->tasks);
        }
    }
}
#endif
