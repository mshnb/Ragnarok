//
//  box.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/3.
//

#ifndef box_h
#define box_h

#include "common.h"
#include "aarect.h"
#include "hittable_list.h"

class box : public hittable
{
    public:
        box() {}
        box(const point3& p0, const point3& p1, shared_ptr<material> ptr);

        virtual bool hit(const ray& r, fType t_min, fType t_max, hit_record& rec) const override;

    public:
        point3 box_min;
        point3 box_max;
        hittable_list sides;
};

box::box(const point3& p0, const point3& p1, shared_ptr<material> ptr)
{
    box_min = p0;
    box_max = p1;

    sides.add(make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), ptr));
    sides.add(make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), ptr));

    sides.add(make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), ptr));
    sides.add(make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), ptr));

    sides.add(make_shared<yz_rect>(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), ptr));
    sides.add(make_shared<yz_rect>(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), ptr));

    aabb_ptr = make_shared<aabb>(box_min, box_max);
}

bool box::hit(const ray& r, fType t_min, fType t_max, hit_record& rec) const
{
    return sides.hit(r, t_min, t_max, rec);
}

#endif /* box_h */
