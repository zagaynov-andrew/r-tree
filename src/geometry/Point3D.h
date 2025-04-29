#ifndef POINT3D_H
#define POINT3D_H

struct Point3D {
    float x, y, z;

    bool operator==(const Point3D& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

#endif //POINT3D_H
