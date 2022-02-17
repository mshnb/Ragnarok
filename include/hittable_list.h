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
    hittable_list() {}
    hittable_list(shared_ptr<hittable> object) { add(object); }
    
    void clear()
    {
        objects.clear();
    }
    
    void add(shared_ptr<hittable> object)
    {
        objects.push_back(object);
    }
    
    virtual bool hit(const ray& r, fType t_min, fType t_max, hit_record& rec) const override;
    virtual bool bounding_box(aabb& output_box) const override;
    
    virtual fType pdf_value(const point3& origin, const vec3& v) const override;
    virtual vec3 random(const vec3& origin) const override;
    
public:
    std::vector<shared_ptr<hittable>> objects;
};

bool hittable_list::hit(const ray& r, fType t_min, fType t_max, hit_record& rec) const {
    hit_record temp_rec;
    bool hit_anything = false;
    fType closest_so_far = t_max;

    for (const auto& object : objects)
    {
        if (object->hit(r, t_min, closest_so_far, temp_rec))
        {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    return hit_anything;
}

bool hittable_list::bounding_box(aabb& output_box) const
{
    if (objects.empty())
        return false;

    aabb temp_box;
    bool first_box = true;

    for (const auto& object : objects) {
        if (!object->bounding_box(temp_box))
            return false;
        output_box = first_box ? temp_box : surrounding_box(output_box, temp_box);
        first_box = false;
    }

    return true;
}


fType hittable_list::pdf_value(const point3& o, const vec3& v) const
{
    fType weight = 1.0/objects.size();
    fType sum = 0.0;

    for (const auto& object : objects)
        sum += weight * object->pdf_value(o, v);

    return sum;
}

vec3 hittable_list::random(const vec3& o) const
{
    auto int_size = static_cast<int>(objects.size());
    int idx = random_int(0, int_size-1);
    return objects[idx]->random(o);
    
    //return objects[random_int(0, int_size-1)]->random(o);
}

#endif /* hittable_list_h */
