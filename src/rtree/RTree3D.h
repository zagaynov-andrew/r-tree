#ifndef RTREE3D_H
#define RTREE3D_H
#include <iostream>

#include "RTreeInnerNode.h"
#include "RTreeLeaf.h"
#include "RTreeNode.h"


class RTree3D {
    std::shared_ptr<RTreeNode> root;
    size_t maxChildren;
    size_t minChildren;

public:
    RTree3D(size_t minChildren = 1, size_t maxChildren = 3);

    void insert(const Triangle3D& obj);

    void remove(const Triangle3D& target);

    std::vector<Triangle3D> find(const MBR& searchMBR) const;

    void buildTree(const std::vector<Triangle3D>& triangles);

    void exportToSVG(const std::string& filename, float scale = 10.0f) const;

private:

    std::shared_ptr<RTreeNode> insertRecursive(std::shared_ptr<RTreeNode> node, const Triangle3D& obj);

    std::shared_ptr<RTreeNode> splitLeaf(std::shared_ptr<RTreeLeaf> leaf, const Triangle3D& newTriangle);

    std::shared_ptr<RTreeInnerNode> splitInternal(std::shared_ptr<RTreeInnerNode> node, std::shared_ptr<RTreeNode> newChild);

    std::pair<Triangle3D, Triangle3D> pickSeedsTriangles(std::vector<Triangle3D>& triangles);

    Triangle3D pickNextTriangle(std::shared_ptr<RTreeNode> group1, std::shared_ptr<RTreeNode> group2, std::vector<Triangle3D>& triangles);

    std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> pickSeedsNodes(std::vector<std::shared_ptr<RTreeNode>>& nodes);

    std::shared_ptr<RTreeNode> pickNextNode(std::shared_ptr<RTreeInnerNode> group1, std::shared_ptr<RTreeInnerNode> group2, std::vector<std::shared_ptr<RTreeNode>>& nodes);

    void fixAfterRemove(std::shared_ptr<RTreeNode>& current, std::shared_ptr<RTreeLeaf>& targetLeaf, std::vector<Triangle3D>& reinserts);

    std::shared_ptr<RTreeLeaf> find(std::shared_ptr<RTreeNode> node, const Triangle3D& searchTriangle) const;

    void find(const std::shared_ptr<RTreeNode>& node, const MBR& searchMBR, std::vector<Triangle3D>& result) const;

    bool removeRecursive(std::shared_ptr<RTreeNode> node, const Triangle3D& target, std::vector<Triangle3D>& reinserts);

    std::vector<Triangle3D> getAllTriangles(const std::shared_ptr<RTreeNode>& node) const;

    void collectAllTriangles(const std::shared_ptr<RTreeNode>& node, std::vector<Triangle3D>& result) const;

    void buildNode(std::shared_ptr<RTreeNode> node, size_t level, const std::vector<Triangle3D>& triangles);

    std::vector<std::vector<Triangle3D>> splitIntoGroups(const std::vector<Triangle3D>& triangles, size_t groupCount, int level);

    void drawNode(const std::shared_ptr<RTreeNode>& node, std::ofstream& file, float scale) const;

    friend std::ostream& operator<<(std::ostream& os, const RTree3D& tree);
};

inline std::ostream& operator<<(std::ostream& os, const RTree3D& tree) {
    std::function<void(const std::shared_ptr<RTreeNode>&, const std::string&, bool)> recur;
    recur = [&](const std::shared_ptr<RTreeNode>& node, const std::string& prefix, bool isLast) {
        os << prefix
           << (isLast ? "└── " : "├── ")
           << (node->isLeaf() ? "Leaf" : "Node");

        const auto& box = node->getMBR();
        os << " [(" << box.min.x << "," << box.min.y << "," << box.min.z << ") - ("
           << box.max.x << "," << box.max.y << "," << box.max.z << ")]\n";

        if (node->isLeaf()) {
            auto leaf = std::dynamic_pointer_cast<RTreeLeaf>(node);
            const auto& tris = leaf->getTriangles();
            std::string childIndent = prefix + (isLast ? "    " : "│   ");

            for (size_t i = 0; i < tris.size(); ++i) {
                const auto& t = tris[i];
                os << childIndent << (i == tris.size() - 1 ? "└── " : "├── ")
                   << "Triangle " << i << ":\n";
                os << childIndent << "    A: (" << t.a.x << ", " << t.a.y << ", " << t.a.z << ")\n";
                os << childIndent << "    B: (" << t.b.x << ", " << t.b.y << ", " << t.b.z << ")\n";
                os << childIndent << "    C: (" << t.c.x << ", " << t.c.y << ", " << t.c.z << ")\n";
            }
        } else {
            auto inner = std::dynamic_pointer_cast<RTreeInnerNode>(node);
            const auto& children = inner->getChildren();
            for (size_t i = 0; i < children.size(); ++i) {
                recur(children[i], prefix + (isLast ? "    " : "│   "), i == children.size() - 1);
            }
        }
    };

    recur(tree.root, "", true);
    return os;
}

#endif //RTREE3D_H
