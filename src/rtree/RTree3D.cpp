#include "RTree3D.h"

#include <fstream>

RTree3D::RTree3D(size_t minChildren, size_t maxChildren) : minChildren(minChildren), maxChildren(maxChildren) {
    root = std::make_shared<RTreeLeaf>();
}

void RTree3D::insert(const Triangle3D& obj) {
    auto newChild = insertRecursive(root, obj);

    if (newChild) {
        auto newRoot = std::make_shared<RTreeInnerNode>();
        newRoot->insert(root);
        newRoot->insert(newChild);
        root = newRoot;
    }
}

void RTree3D::remove(const Triangle3D& target) {
    if (!root) return;

    std::vector<Triangle3D> reinserts;
    bool found = removeRecursive(root, target, reinserts);

    if (!found) {
        // Объект не найден, ничего не делаем
        return;
    }

    if (!root->isLeaf()) {
        auto internal = std::dynamic_pointer_cast<RTreeInnerNode>(root);
        if (internal->getChildren().size() == 1) {
            root = internal->getChildren()[0];
        }
    }

    // Перевставка треугольников
    for (auto& tri : reinserts) {
        insert(tri);
    }
}

std::vector<Triangle3D> RTree3D::find(const MBR& searchMBR) const {
    std::vector<Triangle3D> result;
    find(root, searchMBR, result);
    return result;
}

void RTree3D::buildTree(const std::vector<Triangle3D>& triangles) {
    if (triangles.size() <= maxChildren) {
        auto leaf = std::make_shared<RTreeLeaf>();
        for (const auto& tri : triangles) {
            leaf->addTriangle(tri);
        }
        root = leaf;
    } else {
        if (triangles.empty()) return;
        size_t levels = std::ceil(log(triangles.size()) / log(maxChildren)) - 1;

        root = std::make_shared<RTreeInnerNode>();
        buildNode(root, levels - 1, triangles);
    }
}

void RTree3D::buildNode(std::shared_ptr<RTreeNode> node, size_t level, const std::vector<Triangle3D>& triangles) {
    if (level == 0) {
        size_t childrenCount = std::ceil(std::pow(triangles.size(), 1.0f / (level + 2)));
        auto groups = splitIntoGroups(triangles, childrenCount, level);

        for (const auto& group : groups) {
            auto newLeaf = std::make_shared<RTreeLeaf>();
            for (const auto& tri : group) {
                newLeaf->addTriangle(tri);
            }
            auto inner = std::dynamic_pointer_cast<RTreeInnerNode>(node);
            inner->insert(newLeaf);
        }
        return;
    }

    size_t childrenCount = std::ceil(std::pow(triangles.size(), 1.0f / (level + 2)));
    auto groups = splitIntoGroups(triangles, childrenCount, level);

    auto internal = std::dynamic_pointer_cast<RTreeInnerNode>(node);
    for (const auto& group : groups) {
        auto child = std::make_shared<RTreeInnerNode>();
        internal->insert(child);
        buildNode(child, level - 1, group);
        internal->recalculateMBR();
    }
    internal->recalculateMBR();
}

std::vector<std::vector<Triangle3D>> RTree3D::splitIntoGroups(const std::vector<Triangle3D>& triangles, size_t groupCount, int level) {
    struct Group {
        std::vector<Triangle3D> triangles;
    };

    if (triangles.empty() || groupCount == 0) {
        return {};
    }

    // Вычисляем глобальные min/max
    MBR subtreeMBR = MBR(triangles);

    // Разбиение по одной координате
    std::vector<Triangle3D> sorted = triangles;

    float dx = subtreeMBR.max.x - subtreeMBR.min.x;
    float dy = subtreeMBR.max.y - subtreeMBR.min.y;
    float dz = subtreeMBR.max.z - subtreeMBR.min.z;

    if (dx >= dy && dx >= dz) {
        std::sort(sorted.begin(), sorted.end(), [](const Triangle3D& t1, const Triangle3D& t2) {
            float ca = (t1.a.x + t1.b.x + t1.c.x) / 3.0f;
            float cb = (t2.a.x + t2.b.x + t2.c.x) / 3.0f;
            return ca < cb;
        });
    } else if (dy >= dz) {
        std::sort(sorted.begin(), sorted.end(), [](const Triangle3D& t1, const Triangle3D& t2) {
            float ca = (t1.a.y + t1.b.y + t1.c.y) / 3.0f;
            float cb = (t2.a.y + t2.b.y + t2.c.y) / 3.0f;
            return ca < cb;
        });
    } else {
        std::sort(sorted.begin(), sorted.end(), [](const Triangle3D& t1, const Triangle3D& t2) {
            float ca = (t1.a.z + t1.b.z + t1.c.z) / 3.0f;
            float cb = (t2.a.z + t2.b.z + t2.c.z) / 3.0f;
            return ca < cb;
        });
    }

    // Нарезаем на группы
    const int maxTrianglesPerGroup = (level != 0)
        ? pow(maxChildren, level + 1)
        : maxChildren;

    std::vector<Group> groups(groupCount);

    int idx = 0;
    for (int i = 0; i < groupCount; ++i) {
        for (int j = 0; j < maxTrianglesPerGroup && idx < sorted.size(); ++j) {
            groups[i].triangles.push_back(sorted[idx++]);
        }
    }

    // Приведение к нужному типу
    std::vector<std::vector<Triangle3D>> result;
    result.reserve(groups.size());
    for (auto& group : groups) {
        if (group.triangles.size() != 0) {
            result.push_back(std::move(group.triangles));
        }
    }

    return result;
}

void RTree3D::exportToSVG(const std::string& filename, float scale) const {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    file << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
    drawNode(root, file, scale);
    file << "</svg>\n";
}

void RTree3D::drawNode(const std::shared_ptr<RTreeNode>& node, std::ofstream& file, float scale) const {
    const float offset = 500;
    if (!node) return;

    if (!node->isLeaf()) {
        auto internal = std::dynamic_pointer_cast<RTreeInnerNode>(node);
        // Нарисовать MBR
        const auto& mbr = internal->getMBR();
        float x = mbr.min.x * scale;
        float y = -mbr.max.y * scale; // SVG: ось Y направлена вниз
        float width = (mbr.max.x - mbr.min.x) * scale;
        float height = (mbr.max.y - mbr.min.y) * scale;

        file << "<rect x=\"" << x + offset << "\" y=\"" << y + offset << "\" width=\"" << width << "\" height=\"" << height
             << "\" fill=\"none\" stroke=\"black\" stroke-dasharray=\"13,9\" stroke-width=\"2\"/>\n";

        for (const auto& child : internal->getChildren()) {
            drawNode(child, file, scale);
        }
    } else {
        auto leaf = std::dynamic_pointer_cast<RTreeLeaf>(node);
        // Нарисовать MBR
        const auto& mbr = leaf->getMBR();
        float x = mbr.min.x * scale;
        float y = -mbr.max.y * scale;
        float width = (mbr.max.x - mbr.min.x) * scale;
        float height = (mbr.max.y - mbr.min.y) * scale;

        file << "<rect x=\"" << x + offset << "\" y=\"" << y + offset << "\" width=\"" << width << "\" height=\"" << height
             << "\" fill=\"none\" stroke=\"black\" stroke-dasharray=\"13,9\" stroke-width=\"2\"/>\n";

        // Нарисовать треугольники
        for (const auto& tri : leaf->getTriangles()) {
            file << "<polygon points=\""
                 << tri.a.x * scale + offset << "," << -tri.a.y * scale + offset << " "
                 << tri.b.x * scale + offset << "," << -tri.b.y * scale + offset << " "
                 << tri.c.x * scale + offset << "," << -tri.c.y * scale + offset
                 << "\" fill=\"white\" stroke=\"black\" stroke-width=\"3\" />\n";
        }
    }
};


// PRIVATE ----------------------------------------------------------------------------------------------------------------------

std::shared_ptr<RTreeNode> RTree3D::insertRecursive(std::shared_ptr<RTreeNode> node, const Triangle3D& obj) {
    if (node->isLeaf()) {
        auto leaf = std::dynamic_pointer_cast<RTreeLeaf>(node);
        if (leaf->getTriangles().size() < maxChildren) {
            leaf->addTriangle(obj);
            return nullptr;
        }
        return splitLeaf(leaf, obj);
    }

    // Найдём лучший дочерний узел для вставки
    auto innerNode = std::dynamic_pointer_cast<RTreeInnerNode>(node);
    std::shared_ptr<RTreeNode> bestChild;
    float minExpansion = std::numeric_limits<float>::infinity();

    for (auto& child : innerNode->getChildren()) {
        auto updatedMbr =  new MBR(child->getMBR());
        float expansion = updatedMbr->expandToInclude(obj)->volume() - child->getMBR().volume();
        if (expansion < minExpansion) {
            minExpansion = expansion;
            bestChild = child;
        }
    }

    std::shared_ptr<RTreeNode> newGrandChild = insertRecursive(bestChild, obj);

    // Обработка нового потомка
    if (newGrandChild) {
        if (innerNode->getChildren().size() < maxChildren) {
            innerNode->insert(newGrandChild);
        } else {
            return std::dynamic_pointer_cast<RTreeNode>(splitInternal(innerNode, newGrandChild)); // дальше обрабатывается выше
        }
    }

    node->recalculateMBR();
    return nullptr;
}

std::shared_ptr<RTreeNode> RTree3D::splitLeaf(std::shared_ptr<RTreeLeaf> leaf, const Triangle3D& newTriangle) {
    // Собираем все объекты
    std::vector<Triangle3D> allTriangles = leaf->getTriangles();
    allTriangles.push_back(newTriangle);

    // Разделяем лист
    leaf->clearTriangles();

    auto newLeaf = std::make_shared<RTreeLeaf>();

    // Выбираем первую пару
    auto [first, second] = pickSeedsTriangles(allTriangles);

    // Добавляем первую пару в разные листья
    leaf->addTriangle(first);
    newLeaf->addTriangle(second);

    // Распределяем оставшиеся треугольники
    while (!allTriangles.empty()) {
        // Если осталось мало треугольников, сразу кидаем их в подходящий узел
        if (leaf->getTriangles().size() + allTriangles.size() <= minChildren) {
            for (auto& tri : allTriangles)
                leaf->addTriangle(tri);
            break;
        }
        if (newLeaf->getTriangles().size() + allTriangles.size() <= minChildren) {
            for (auto& tri : allTriangles)
                newLeaf->addTriangle(tri);
            break;
        }

        // Выбираем следующий треугольник
        auto next = pickNextTriangle(leaf, newLeaf, allTriangles);

        auto updatedLeafMbr = new MBR(leaf->getMBR());
        auto updatedNewLeafMbr = new MBR(newLeaf->getMBR());
        // Вычисляем увеличение площади
        float d1 = updatedLeafMbr->expandToInclude(next)->volume() - leaf->getMBR().volume();
        float d2 = updatedNewLeafMbr->expandToInclude(next)->volume() - newLeaf->getMBR().volume();

        if (d1 < d2 || (d1 == d2 && leaf->getTriangles().size() < newLeaf->getTriangles().size()))
            leaf->addTriangle(next);
        else
            newLeaf->addTriangle(next);
    }

    return newLeaf;
}

std::shared_ptr<RTreeInnerNode> RTree3D::splitInternal(std::shared_ptr<RTreeInnerNode> node, std::shared_ptr<RTreeNode> newChild) {
    std::vector<std::shared_ptr<RTreeNode>> allChildren = node->getChildren();
    allChildren.push_back(newChild);

    // Разделяем узел
    node->clearChildren();
    auto newNode = std::make_shared<RTreeInnerNode>();

    // Выбираем первую пару
    auto [first, second] = pickSeedsNodes(allChildren);

    // Добавляем первую пару
    node->insert(first);
    newNode->insert(second);

    // Распределяем оставшиеся узлы
    while (!allChildren.empty()) {
        if (node->getChildren().size() + allChildren.size() <= minChildren) {
            for (auto& child : allChildren)
                node->insert(child);
            break;
        }
        if (newNode->getChildren().size() + allChildren.size() <= minChildren) {
            for (auto& child : allChildren)
                newNode->insert(child);
            break;
        }

        auto next = pickNextNode(node, newNode, allChildren);

        auto updatedNodeMbr = new MBR(node->getMBR());
        auto updatedNewNodeMbr = new MBR(newNode->getMBR());
        float d1 = updatedNodeMbr->expandToInclude(next->getMBR())->volume() - node->getMBR().volume();
        float d2 = updatedNewNodeMbr->expandToInclude(next->getMBR())->volume() - newNode->getMBR().volume();

        if (d1 < d2 || (d1 == d2 && node->getChildren().size() < newNode->getChildren().size()))
            node->insert(next);
        else
            newNode->insert(next);
    }

    return newNode;
}

std::pair<Triangle3D, Triangle3D> RTree3D::pickSeedsTriangles(std::vector<Triangle3D>& triangles) {
    float maxWaste = -1.0f;
    size_t index1 = 0, index2 = 1;

    for (size_t i = 0; i < triangles.size(); ++i) {
        for (size_t j = i + 1; j < triangles.size(); ++j) {
            MBR mbr1(triangles[i]);
            MBR mbr2(triangles[j]);
            MBR combined = MBR::combine(mbr1, mbr2);
            float waste = combined.volume() - mbr1.volume() - mbr2.volume();

            if (waste > maxWaste) {
                maxWaste = waste;
                index1 = i;
                index2 = j;
            }
        }
    }

    Triangle3D t1 = triangles[index1];
    Triangle3D t2 = triangles[index2];

    // Удаляем выбранные треугольники из списка
    if (index1 > index2) std::swap(index1, index2);
    triangles.erase(triangles.begin() + index2);
    triangles.erase(triangles.begin() + index1);

    return {t1, t2};
}

Triangle3D RTree3D::pickNextTriangle(std::shared_ptr<RTreeNode> group1, std::shared_ptr<RTreeNode> group2, std::vector<Triangle3D>& triangles) {
    float maxDiff = -1.0f;
    size_t bestIndex = 0;

    for (size_t i = 0; i < triangles.size(); ++i) {
        auto box1 = new MBR(group1->getMBR());
        auto box2 = new MBR(group2->getMBR());
        box1->expandToInclude(triangles[i]);
        box2->expandToInclude(triangles[i]);
        float d1 = box1->volume() - group1->getMBR().volume();
        float d2 = box2->volume() - group2->getMBR().volume();
        float diff = std::abs(d1 - d2);

        if (diff > maxDiff) {
            maxDiff = diff;
            bestIndex = i;
        }
    }

    Triangle3D chosen = triangles[bestIndex];
    triangles.erase(triangles.begin() + bestIndex);
    return chosen;
}

std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTree3D::pickSeedsNodes(std::vector<std::shared_ptr<RTreeNode>>& nodes) {
    float maxWaste = -1.0f;
    size_t index1 = 0, index2 = 1;

    for (size_t i = 0; i < nodes.size(); ++i) {
        for (size_t j = i + 1; j < nodes.size(); ++j) {
            MBR combined = MBR::combine(nodes[i]->getMBR(), nodes[j]->getMBR());
            float waste = combined.volume() - nodes[i]->getMBR().volume() - nodes[j]->getMBR().volume();

            if (waste > maxWaste) {
                maxWaste = waste;
                index1 = i;
                index2 = j;
            }
        }
    }

    auto n1 = nodes[index1];
    auto n2 = nodes[index2];

    if (index1 > index2) std::swap(index1, index2);
    nodes.erase(nodes.begin() + index2);
    nodes.erase(nodes.begin() + index1);

    return {n1, n2};
}

std::shared_ptr<RTreeNode> RTree3D::pickNextNode(std::shared_ptr<RTreeInnerNode> group1, std::shared_ptr<RTreeInnerNode> group2, std::vector<std::shared_ptr<RTreeNode>>& nodes) {
    float maxDiff = -1.0f;
    size_t bestIndex = 0;

    for (size_t i = 0; i < nodes.size(); ++i) {
        auto box1 = new MBR(group1->getMBR());
        auto box2 = new MBR(group2->getMBR());
        box1->expandToInclude(nodes[i]->getMBR());
        box2->expandToInclude(nodes[i]->getMBR());
        float d1 = box1->volume() - group1->getMBR().volume();
        float d2 = box2->volume() - group2->getMBR().volume();
        float diff = std::abs(d1 - d2);

        if (diff > maxDiff) {
            maxDiff = diff;
            bestIndex = i;
        }
    }

    auto chosen = nodes[bestIndex];
    nodes.erase(nodes.begin() + bestIndex);
    return chosen;
}

std::shared_ptr<RTreeLeaf> RTree3D::find(std::shared_ptr<RTreeNode> node, const Triangle3D& searchTriangle) const {
    if (!node) return nullptr;

    if (node->isLeaf()) {
        auto leaf = std::dynamic_pointer_cast<RTreeLeaf>(node);
        for (const auto& t : leaf->getTriangles()) {
            if (t == searchTriangle) {
                return leaf;
            }
        }
    } else {
        auto internal = std::dynamic_pointer_cast<RTreeInnerNode>(node);
        for (auto child : internal->getChildren()) {
            if (child->getMBR().contains(searchTriangle)) {
                auto found= find(child, searchTriangle);
                if (found) return found;
            }
        }
    }

    return nullptr;
}

void RTree3D::find(const std::shared_ptr<RTreeNode>& node, const MBR& searchMBR, std::vector<Triangle3D>& result) const {
    if (!node->isLeaf()) {
        auto inner = std::dynamic_pointer_cast<RTreeInnerNode>(node);
        for (const auto& child : inner->getChildren()) {
            if (child->getMBR().intersects(searchMBR)) {
                find(child, searchMBR, result);
            }
        }
    } else {
        auto leaf = std::dynamic_pointer_cast<RTreeLeaf>(node);
        for (const auto& triangle : leaf->getTriangles()) {
            MBR triMBR(triangle);
            if (searchMBR.intersects(triMBR)) {
                result.push_back(triangle);
            }
        }
    }
}

bool RTree3D::removeRecursive(std::shared_ptr<RTreeNode> node, const Triangle3D& target, std::vector<Triangle3D>& reinserts) {
    if (node->isLeaf()) {
            auto leaf = std::dynamic_pointer_cast<RTreeLeaf>(node);
        auto it = std::find(leaf->getTriangles().begin(), leaf->getTriangles().end(), target);
        if (it != leaf->getTriangles().end()) {
            leaf->remove(*it);;
            leaf->recalculateMBR();
            return true;
        }
        return false;
    }

    auto internal = std::dynamic_pointer_cast<RTreeInnerNode>(node);
    for (auto it : internal->getChildren()) {
        if (it->getMBR().contains(target)) {  // идем только в те поддеревья, чьи MBR пересекаются с треугольником
            if (removeRecursive(it, target, reinserts)) {
                // После удаления нужно проверить размер потомка
                if (it->isLeaf()) {
                    auto leaf = std::dynamic_pointer_cast<RTreeLeaf>(it);
                    if (leaf->getTriangles().size() < minChildren) {  // minFill — минимально допустимое количество элементов
                        // Элементов слишком мало — реинсертим
                        reinserts.insert(reinserts.end(), leaf->getTriangles().begin(), leaf->getTriangles().end());
                        internal->remove(it); // Удаляем узел
                    } else {
                        it->recalculateMBR();
                    }
                } else {
                    auto childInternal = std::dynamic_pointer_cast<RTreeInnerNode>(it);
                    if (childInternal->getChildren().size() < minChildren) {
                        // Элементов слишком мало — реинсертим
                        for (auto& grandChild : childInternal->getChildren()) {
                            auto allTriangles = getAllTriangles(grandChild);
                            reinserts.insert(reinserts.end(), allTriangles.begin(), allTriangles.end());
                        }
                        internal->remove(it); // Удаляем узел
                    } else {
                        it->recalculateMBR();
                    }
                }
                // После обработки дочернего элемента пересчитываем MBR текущего узла
                internal->recalculateMBR();
                return true;
            }
        }
    }
    return false;
}

std::vector<Triangle3D> RTree3D::getAllTriangles(const std::shared_ptr<RTreeNode>& node) const {
    std::vector<Triangle3D> result;
    collectAllTriangles(node, result);
    return result;
}

void RTree3D::collectAllTriangles(const std::shared_ptr<RTreeNode>& node, std::vector<Triangle3D>& result) const {
    if (!node) return;

    if (node->isLeaf()) {
        auto leaf = std::dynamic_pointer_cast<RTreeLeaf>(node);
        result.insert(result.end(), leaf->getTriangles().begin(), leaf->getTriangles().end());
    } else {
    auto internal = std::dynamic_pointer_cast<RTreeInnerNode>(node);
        for (const auto& child : internal->getChildren()) {
            collectAllTriangles(child, result);
        }
    }
}
