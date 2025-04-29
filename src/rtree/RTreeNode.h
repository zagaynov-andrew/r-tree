#ifndef RTREENODE_H
#define RTREENODE_H

class RTreeNode {
public:
    virtual ~RTreeNode() = default;
    virtual bool isLeaf() const = 0;
    virtual const MBR& getMBR() const = 0;
    virtual void recalculateMBR() = 0;
};

#endif //RTREENODE_H
