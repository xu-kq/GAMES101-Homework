#include <algorithm>
#include <cassert>
#include "BVH.hpp"

BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
    SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
    primitives(std::move(p))
{
    time_t start, stop;
    time(&start);
    if (primitives.empty())
        return;
    else if (splitMethod == SplitMethod::NAIVE)
        root = recursiveBuild(primitives);
    else
        root = recursive_SAH_Build(primitives);

    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    if (splitMethod == SplitMethod::NAIVE)
    printf(
        "\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
        hrs, mins, secs);
    else
        printf(
            "\rSAH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
            hrs, mins, secs);
}

BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
    BVHBuildNode* node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        switch (dim) {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().x <
                       f2->getBounds().Centroid().x;
            });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().y <
                       f2->getBounds().Centroid().y;
            });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().z <
                       f2->getBounds().Centroid().z;
            });
            break;
        }
        auto beginning = objects.begin();
        auto middling = objects.begin() + (objects.size() / 2);
        auto ending = objects.end();

        auto leftshapes = std::vector<Object*>(beginning, middling);
        auto rightshapes = std::vector<Object*>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
    }

    return node;
}

BVHBuildNode* BVHAccel::recursive_SAH_Build(std::vector<Object*>objects) {
    BVHBuildNode* node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursive_SAH_Build(std::vector{ objects[0] });
        node->right = recursive_SAH_Build(std::vector{ objects[1] });

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
            Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        switch (dim) {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().x <
                    f2->getBounds().Centroid().x;
                });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().y <
                    f2->getBounds().Centroid().y;
                });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().z <
                    f2->getBounds().Centroid().z;
                });
            break;
        }
        // SAH
        double S0 = 0., S_A = 0., S_B = 0., S1 = 0.;
        int N_A = 0, N_B = 0;
        Bounds3 tmp_bounds_A, tmp_bounds_B;
        std::vector<double> prefix_Area(objects.size());
        std::vector<double> postfix_Area(objects.size());

        for (int i = 0; i < objects.size(); ++i) {
            tmp_bounds_A = Union(tmp_bounds_A, objects[i]->getBounds());
            tmp_bounds_B = Union(tmp_bounds_B, objects[objects.size() - 1 - i]->getBounds());
            S0 = tmp_bounds_A.SurfaceArea();
            S1 = tmp_bounds_B.SurfaceArea();
            prefix_Area[i] = S0;
            postfix_Area[objects.size() - 1 - i] = S1;
        }
        S0 = prefix_Area.back();
        int idx = 0;
        double cost = std::numeric_limits<double>::infinity();
        for (int i = 0; i < objects.size() - 1; ++i) {
            N_A = i + 1;
            N_B = objects.size() - N_A;
            S_A = prefix_Area[i];
            S_B = postfix_Area[i+1];
            double cur_cost = N_A * S_A + N_B * S_B;
            if (cur_cost < cost) {
                cost = cur_cost;
                idx = N_A;
            }
        }

        auto beginning = objects.begin();
        auto middling = objects.begin() + idx;
        auto ending = objects.end();


        auto leftshapes = std::vector<Object*>(beginning, middling);
        auto rightshapes = std::vector<Object*>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursive_SAH_Build(leftshapes);
        node->right = recursive_SAH_Build(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
    }

    return node;
}
Intersection BVHAccel::Intersect(const Ray& ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    // TODO Traverse the BVH to find intersection
    Intersection inter;
    std::array<int, 3> dirIsNeg{ int(ray.direction.x > 0), int(ray.direction.y > 0), int(ray.direction.z > 0) };
    if (!node->bounds.IntersectP(ray, ray.direction_inv, dirIsNeg)) {
        return inter;
    }
    if (node->left == nullptr && node->right == nullptr) {
        inter = node->object->getIntersection(ray);
        return inter;
    }
    Intersection lleaf, rleaf;
    lleaf = getIntersection(node->left, ray);
    rleaf = getIntersection(node->right, ray);

    return std::min(lleaf, rleaf, [](const auto& lhs, const auto& rhs) {
        return lhs.distance < rhs.distance;
        });

    //return Intersection();
}