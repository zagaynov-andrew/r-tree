#include <iostream>

#include "src/geometry/Triangle3D.h"
#include "src/rtree/RTree3D.h"

void printTriangle(const Triangle3D& t) {
    std::cout << "Triangle: "
              << "(" << t.a.x << ", " << t.a.y << ", " << t.a.z << ") - "
              << "(" << t.b.x << ", " << t.b.y << ", " << t.b.z << ") - "
              << "(" << t.c.x << ", " << t.c.y << ", " << t.c.z << ")\n";
}

int main() {
    RTree3D tree;

    std::vector<Triangle3D> triangles;

    // Начальное построение дерева
    triangles.push_back({{-5.6, 6.8, 0}, {-4, 8, 0}, {-5.6, 8, 0}});
    triangles.push_back({{0, 6, 0}, {2, 0, 0}, {0, 1, 0}});
    triangles.push_back({{3, 2, 0}, {4, 3, 0}, {4, 2, 0}});
    triangles.push_back({{-4, 6, 0}, {-3.2, 6.8, 0}, {-3.6, 5.2, 0}});
    triangles.push_back({{5, 2, 0}, {5, 3, 0}, {6, 2, 0}});
    triangles.push_back({{-4, 4, 0}, {-6, 4, 0}, {-6, 6, 0}});
    triangles.push_back({{6, -1, 0}, {6, -2, 0}, {5, -2, 0}});
    triangles.push_back({{4.6, -2, 0}, {4, -2.5, 0}, {4.8, -3, 0}});
    triangles.push_back({{3, 3, 0}, {4.5, 3, 0}, {3, 5, 0}});
    triangles.push_back({{7, 1, 0}, {7, -3, 0}, {6, -3, 0}});
    triangles.push_back({{-6, -4, 0}, {-6, -2, 0}, {-4, -4, 0}});
    triangles.push_back({{-0.8, 0.4, 0}, {-0.8, -2, 0}, {-4, -2, 0}});
    triangles.push_back({{6, 7, 0}, {6.4, 4.4, 0}, {7, 6, 0}});

    tree.buildTree(triangles);

    // Визуализация
    tree.exportToSVG("~/CLionProjects/rtree/test.svg", 60);

    std::cout << tree << std::endl;

    // Вставка элемента
    tree.insert({{-3, 1, 0}, {-4, 1, 0}, {-3, 0, 0}});

    std::cout << tree << std::endl;

    // Удаление элемента
    tree.remove({{-3, 1, 0}, {-4, 1, 0}, {-3, 0, 0}});

    std::cout << tree << std::endl;

    // Диапазон поиска
    MBR range;
    range.min = {0, 0, 0};
    range.max = {3, 3, 3};

    // Поиск треугольников в диапазоне
    auto found = tree.find(range);

    std::cout << "\nНайденные треугольники в диапазоне:\n";
    for (const auto& tri : found) {
        printTriangle(tri);
    }

    return 0;
}
