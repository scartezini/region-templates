#include "test.hpp"
#include <algorithm>

TaskReorder::TaskReorder(std::list<ReusableTask *> &tasks) {
    for (ReusableTask *rt : tasks) {
        node n = {};
        n.id = rt->getId();
        n.parent = rt->parentTask;
        n.prt = rt;

        if (n.parent != -1)
            tree[rt->parentTask].children.push_back(n.id);
        else
            root = rt->getId();

        tree[n.id] = n;
    }
}

void TaskReorder::print(int origin, int level) const {
    if (origin < 0) return;
    if (!origin) origin = root;

    for (int i = 0; i < level; ++i) std::cout << '\t';

    std::cout << origin << std::endl;

    for (int rt : tree.at(origin)) print(rt, level + 1);
}

void TaskReorder::mergeTest() {
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
            id2addr[node]->parentTasks.push_back(newParent);
            newParent = 0;
        }
    }
}

int TaskReorder::height(node n) {
    int maxHeight = 0;
    for (auto child : tree.at(n)) {
        int h = height(child) + 1;

        maxHeight = std::max(h, maxHeight);
    }
    return maxHeight;
}

void TaskReorder::thinning(int maxWidth) {
    for (int i = 0; auto nodes = filterLevel(i); i++) {
        if (nodes.empty()) return;

        while (nodes.size() > maxWidth) {
            int child = child2remove(nodes);
            {
                auto remove = filterRemovable(nodes);
                std::sort(
                    remove.begin(), remove.end(),
                    [this](int a, int b) -> bool { return shorter(a, b) });
                return remove.front();
            }

            int newParent = shortestBrotherLeaf(remove.front());
            changeParent(child, newParent);
            {
                tree.at(remove.front()).fakeParent = newParent;
                update(newParent, remove.front());
            }
            nodes.erase(
                std::remove_if(
                    nodes.begin(),
                    nodes.end(), [child](int x) { return x == child; }),
                nodes.end());
        }
    }
}

int TaskReorder::firstLeaf(int node) {
    while (tree[node].size() != 0) {
        node = tree[node].back();
    }
    return node;
}

// void reorder_stage(std::list<ReusableTask *> &tasks) {
//    for (ReusableTask *t : tasks) {
//        if (t->getTaskName().compare("TaskSegmentation6") == 0) {
//            segment[t->parentTask->parentTask->parentTask->parentTask
//                        ->getId()] = t->getId();
//        }

//        int i;
//        for (auto s : segment) {
//            i++;
//        }
//    }
//}
