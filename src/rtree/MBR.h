#ifndef MBR_H
#define MBR_H
#include <limits>
#include <vector>

#include "../geometry/Point3D.h"
#include "../geometry/Triangle3D.h"

struct Triangle3D;

struct MBR {
    Point3D min;
    Point3D max;

    MBR()
        : min({ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() }),
          max({ std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() }) {}

    MBR(const Triangle3D& triangle);

    MBR(const std::vector<Triangle3D>& triangles);

    float volume() const;

    static MBR combine(const MBR& a, const MBR& b);

    MBR* expandToInclude(const MBR& other);

    MBR* expandToInclude(const Triangle3D &obj);

    MBR* expandToInclude(const Point3D &point);

    bool contains(const Point3D& p) const;
    bool contains(const MBR& other) const;

    bool intersects(const MBR& other) const;
};

#endif //MBR_H
