#ifndef RTREEINNERNODE_H
#define RTREEINNERNODE_H
#include <vector>

#include "MBR.h"
#include "RTreeNode.h"

class RTreeInnerNode : public RTreeNode {
    MBR mbr;
    std::vector<std::shared_ptr<RTreeNode>> children;

    void updateBoundingBox() {
        mbr = MBR();
        for (const auto& child : children) {
            mbr.expandToInclude(child->getMBR());
        }
    }

public:
    RTreeInnerNode() = default;

    bool isLeaf() const override { return false; }

    const MBR& getMBR() const override {
        return mbr;
    }

    void recalculateMBR() override {
        if (children.empty()) {
            mbr = MBR();
            return;
        }

        mbr = children[0]->getMBR();
        for (size_t i = 1; i < children.size(); ++i) {
            mbr = MBR::combine(mbr, children[i]->getMBR());
        }
    }

    void insert(const std::shared_ptr<RTreeNode>& node) {
        children.push_back(node);
        mbr.expandToInclude(node->getMBR());
    }

    const std::vector<std::shared_ptr<RTreeNode>>& getChildren() const {
        return children;
    }

    void remove(std::shared_ptr<RTreeNode> node) {
        children.erase(std::remove(children.begin(), children.end(), node), children.end());
    }

    void clearChildren() {
        children.clear();
    }
};

#endif //RTREEINNERNODE_H
