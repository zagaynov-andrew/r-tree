#include "MBR.h"

MBR::MBR(const Triangle3D& triangle) {
    // Инициализируем минимальные и максимальные значения для каждой оси
    min.x = std::min({triangle.a.x, triangle.b.x, triangle.c.x});
    min.y = std::min({triangle.a.y, triangle.b.y, triangle.c.y});
    min.z = std::min({triangle.a.z, triangle.b.z, triangle.c.z});

    max.x = std::max({triangle.a.x, triangle.b.x, triangle.c.x});
    max.y = std::max({triangle.a.y, triangle.b.y, triangle.c.y});
    max.z = std::max({triangle.a.z, triangle.b.z, triangle.c.z});
}

MBR::MBR(const std::vector<Triangle3D>& triangles) {
    std::vector<float> xs, ys, zs;

    for (const auto& tri : triangles) {
        xs.push_back(tri.a.x);
        xs.push_back(tri.b.x);
        xs.push_back(tri.c.x);

        ys.push_back(tri.a.y);
        ys.push_back(tri.b.y);
        ys.push_back(tri.c.y);

        zs.push_back(tri.a.z);
        zs.push_back(tri.b.z);
        zs.push_back(tri.c.z);
    }

    min.x = *std::min_element(xs.begin(), xs.end());
    max.x = *std::max_element(xs.begin(), xs.end());

    min.y = *std::min_element(ys.begin(), ys.end());
    max.y = *std::max_element(ys.begin(), ys.end());

    min.z = *std::min_element(zs.begin(), zs.end());
    max.z = *std::max_element(zs.begin(), zs.end());
}

float MBR::volume() const {
    return (max.x - min.x) * (max.y - min.y) * (max.z - min.z);
}

MBR MBR::combine(const MBR& a, const MBR& b) {
    MBR result;

    result.min.x = std::min(a.min.x, b.min.x);
    result.min.y = std::min(a.min.y, b.min.y);
    result.min.z = std::min(a.min.z, b.min.z);

    result.max.x = std::max(a.max.x, b.max.x);
    result.max.y = std::max(a.max.y, b.max.y);
    result.max.z = std::max(a.max.z, b.max.z);

    return result;
}

MBR* MBR::expandToInclude(const MBR& other) {
    min.x = std::min(min.x, other.min.x);
    min.y = std::min(min.y, other.min.y);
    min.z = std::min(min.z, other.min.z);

    max.x = std::max(max.x, other.max.x);
    max.y = std::max(max.y, other.max.y);
    max.z = std::max(max.z, other.max.z);
    return this;
}

MBR* MBR::expandToInclude(const Triangle3D &obj) {
    min.x = std::min({ min.x, obj.a.x, obj.b.x, obj.c.x });
    min.y = std::min({ min.y, obj.a.y, obj.b.y, obj.c.y });
    min.z = std::min({ min.z, obj.a.z, obj.b.z, obj.c.z });

    max.x = std::max({ max.x, obj.a.x, obj.b.x, obj.c.x });
    max.y = std::max({ max.y, obj.a.y, obj.b.y, obj.c.y });
    max.z = std::max({ max.z, obj.a.z, obj.b.z, obj.c.z });
    return this;
}

MBR* MBR::expandToInclude(const Point3D &point) {
    min.x = std::min(min.x, point.x);
    min.y = std::min(min.y, point.y);
    min.z = std::min(min.z, point.z);

    max.x = std::max(max.x, point.x);
    max.y = std::max(max.y, point.y);
    max.z = std::max(max.z, point.z);
    return this;
}

bool MBR::contains(const Point3D& p) const {
    return p.x >= min.x && p.x <= max.x &&
           p.y >= min.y && p.y <= max.y &&
           p.z >= min.z && p.z <= max.z;
}

bool MBR::contains(const MBR& other) const {
    return (min.x <= other.min.x && other.max.x <= max.x) &&
           (min.y <= other.min.y && other.max.y <= max.y) &&
           (min.z <= other.min.z && other.max.z <= max.z);
}

bool MBR::intersects(const MBR& other) const {
    return (min.x <= other.max.x && max.x >= other.min.x) &&
           (min.y <= other.max.y && max.y >= other.min.y) &&
           (min.z <= other.max.z && max.z >= other.min.z);
}
