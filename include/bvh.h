//
//  bvh.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/3.
//

#ifndef bvh_h
#define bvh_h

#include <algorithm>

#include "common.h"
#include "hittable.h"
#include "hittable_list.h"

class bvh_node : public hittable
{
public:
	bvh_node(const std::vector<shared_ptr<hittable>>& src_objects) : bvh_node(src_objects, 0, src_objects.size()) {}
    bvh_node(const std::vector<shared_ptr<hittable>>& src_objects,size_t start, size_t end);

    virtual bool hit(const ray& r, fType t_min, fType t_max, hit_cache& cache) const override;
    
public:
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;
};

inline bool box_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis)
{
    shared_ptr<aabb> box_a = a->bounding_box();
	shared_ptr<aabb> box_b = b->bounding_box();

    if (!box_a || !box_b)
        std::cerr << "No bounding box in bvh_node constructor.\n";

    return box_a->min()[axis] < box_b->min()[axis];
}

bool box_x_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b)
{
    return box_compare(a, b, 0);
}

bool box_y_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b)
{
    return box_compare(a, b, 1);
}

bool box_z_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b)
{
    return box_compare(a, b, 2);
}

bool bvh_node::hit(const ray& r, fType t_min, fType t_max, hit_cache& cache) const
{
    if (!aabb_ptr->hit(r, t_min, t_max))
        return false;

    bool hit_left = left->hit(r, t_min, t_max, cache);
    bool hit_right = right->hit(r, t_min, hit_left ? cache.t : t_max, cache);
    return hit_left || hit_right;
}

bvh_node::bvh_node(const std::vector<shared_ptr<hittable>>& src_objects, size_t start, size_t end)
{
    auto objects = src_objects; // Create a modifiable array of the source scene objects

    int axis = random_int(0,2);
    auto comparator = (axis == 0) ? box_x_compare
                    : (axis == 1) ? box_y_compare
                                  : box_z_compare;

    size_t object_span = end - start;
    if (object_span == 1)
        left = right = objects[start];
    else if (object_span == 2)
    {
        if (comparator(objects[start], objects[start+1]))
        {
            left = objects[start];
            right = objects[start+1];
        }
        else
        {
            left = objects[start+1];
            right = objects[start];
        }
    }
    else
    {
        std::sort(objects.begin() + start, objects.begin() + end, comparator);
        auto mid = start + object_span / 2;
        left = make_shared<bvh_node>(objects, start, mid);
        right = make_shared<bvh_node>(objects, mid, end);
    }

    shared_ptr<aabb> box_left = left->bounding_box();
    shared_ptr<aabb> box_right = right->bounding_box();

    if (!box_left || !box_right)
        std::cerr << "No bounding box in bvh_node constructor.\n";

    aabb_ptr = make_shared<aabb>();
    aabb_ptr->surrounding_box(box_left, box_right);
}

#endif /* bvh_h */
