#ifndef TEST_HPP
#define TEST_HPP
#include <list>
#include <unordered_map>
#include <vector>

class TaskReorder {
  public:
    TaskReorder(std::list<ReusableTask *> &tasks) {
        for (ReusableTask *rt : tasks) {
            id2addr[rt->getId()] = rt;
            if (tree.count(rt->getId()) == 0) tree[rt->getId()] = {};

            if (rt->parentTask != -1)
                tree[rt->parentTask].push_back(rt->getId());
            else
                root = rt->getId();
        }
    }

    void print(int origin = 0, int level = 0) const {
        if (origin < 0) return;
        if (!origin) origin = root;

        for (int i = 0; i < level; ++i) std::cout << '\t';

        std::cout << origin << std::endl;

        for (int rt : tree.at(origin)) print(rt, level + 1);
    }

    void mergeTest() {
        int max = 0;
        int maxId = 0;
        for (auto node : tree) {
            if (node.second.size() > max) {
                maxId = node.first;
                max = node.second.size();
            }
        }

        int newParent = 0;
        for (auto node : tree[maxId]) {
            if (!newParent) {
                newParent = firstLeaf(node);
            } else {
                id2addr[node]->parentTask = newParent;
                newParent = 0;
            }
        }
    }

    int firstLeaf(int node) {
        while (tree[node].size() != 0) {
            node = tree[node].back();
        }
        return node;
    }

  private:
    std::unordered_map<int, std::vector<int>> tree;
    std::unordered_map<int, ReusableTask *> id2addr;
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

#endif
