#ifndef RTREELEAF_H
#define RTREELEAF_H
#include "RTreeNode.h"
#include "../geometry/Triangle3D.h"

class RTreeLeaf : public RTreeNode {
    MBR mbr;
    std::vector<Triangle3D> triangles;
public:
    void addTriangle(const Triangle3D& triangle) {
        triangles.push_back(triangle);
        mbr.expandToInclude(computeTriangleMBR(triangle));
    }

    bool isLeaf() const override {
        return true;
    }

    const MBR& getMBR() const override {
        return mbr;
    }

    void recalculateMBR() override {
        if (triangles.empty()) {
            mbr = MBR();
            return;
        }

        mbr = MBR(triangles[0]);
        for (size_t i = 1; i < triangles.size(); ++i) {
            mbr = MBR::combine(mbr, MBR(triangles[i]));
        }
    }

    const std::vector<Triangle3D>& getTriangles() const {
        return triangles;
    }

    void clearTriangles() {
        triangles.clear();
    }

    void remove(const Triangle3D& triangle) {
        triangles.erase(std::remove(triangles.begin(), triangles.end(), triangle), triangles.end());
        recalculateMBR();
    }

private:
    MBR computeTriangleMBR(const Triangle3D& t) const {
        MBR mbr;
        mbr.expandToInclude(t.a);
        mbr.expandToInclude(t.b);
        mbr.expandToInclude(t.c);
        return mbr;
    }
};

#endif //RTREELEAF_H
