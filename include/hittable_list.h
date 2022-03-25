//
//  hittable_list.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/1.
//

#ifndef hittable_list_h
#define hittable_list_h

#include "hittable.h"
#include "aabb.h"

#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;

class hittable_list : public hittable
{
public:
    hittable_list() 
    {
        aabb_ptr = make_shared<aabb>();
    }

    hittable_list(shared_ptr<hittable> object) { add(object); }
    
    void clear()
    {
        objects.clear();
    }
    
    void add(shared_ptr<hittable> object)
    {
        objects.push_back(object);
        auto obj_aabb = object->bounding_box();
        if (obj_aabb)
            aabb_ptr->surrounding_box(aabb_ptr, obj_aabb);
    }
    
    virtual bool hit_fast(const ray& r, fType t_min, fType t_max) const override;
    virtual bool hit(const ray& r, fType t_min, fType t_max, hit_cache& cache) const override;
    
public:
    std::vector<shared_ptr<hittable>> objects;
};

bool hittable_list::hit_fast(const ray& r, fType t_min, fType t_max) const
{
	for (const auto& object : objects)
	{
        if (object->hit_fast(r, t_min, t_max))
            return true;
	}

	return false;
}

bool hittable_list::hit(const ray& r, fType t_min, fType t_max, hit_cache& cache) const
{
    hit_cache temp;
    bool hit_anything = false;
    fType closest_so_far = t_max;

    for (const auto& object : objects)
    {
        if (object->hit(r, t_min, closest_so_far, temp))
        {
            hit_anything = true;
            closest_so_far = temp.t;
            cache = temp;
        }
    }

    return hit_anything;
}

#endif /* hittable_list_h */
