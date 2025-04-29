#ifndef TRIANGLE3D_H
#define TRIANGLE3D_H
#include "Point3D.h"

struct Triangle3D {
    Point3D a, b, c;

    bool operator==(const Triangle3D& other) const {
        return (a == other.a && b == other.b && c == other.c) ||
               (a == other.b && b == other.c && c == other.a) ||
               (a == other.c && b == other.a && c == other.b);
    }
};

#endif //TRIANGLE3D_H
