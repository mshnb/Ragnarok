//
//  aarect.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/3.
//

#ifndef aarect_h
#define aarect_h

#include "common.h"
#include "hittable.h"

const fType rect_pad = 1e-4;

class xy_rect : public hittable
{
public:
    xy_rect() {}

    xy_rect(fType _x0, fType _x1, fType _y0, fType _y1, fType _k, shared_ptr<material> mat)
        : x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mp(mat) 
    {
        aabb_ptr = make_shared<aabb>(point3(x0, y0, k - rect_pad), point3(x1, y1, k + rect_pad));
    }

    virtual bool hit(const ray& r, fType t_min, fType t_max, hit_record& rec) const override;
    
public:
    shared_ptr<material> mp;
    fType x0, x1, y0, y1, k;
};

bool xy_rect::hit(const ray& r, fType t_min, fType t_max, hit_record& rec) const
{
    fType t = (k-r.origin.z()) / r.direction.z();
    if (t < t_min || t > t_max)
        return false;
    fType x = r.origin.x() + t*r.direction.x();
    fType y = r.origin.y() + t*r.direction.y();
    if (x < x0 || x > x1 || y < y0 || y > y1)
        return false;
    rec.u = (x-x0)/(x1-x0);
    rec.v = (y-y0)/(y1-y0);
    rec.t = t;
    auto outward_normal = vec3(0, 0, 1);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mp;
    rec.p = r.at(t);
    return true;
}

class xz_rect : public hittable
{
    public:
        xz_rect() {}

        xz_rect(fType _x0, fType _x1, fType _z0, fType _z1, fType _k, shared_ptr<material> mat)
            : x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mp(mat) 
        {
            aabb_ptr = make_shared<aabb>(point3(x0, k - rect_pad, z0), point3(x1, k + rect_pad, z1));
        }

        virtual bool hit(const ray& r, fType t_min, fType t_max, hit_record& rec) const override;

        virtual fType pdf_value(const point3& origin, const vec3& v) const override
        {
            //TODO use hit_fast
            hit_record rec;
            if (!this->hit(ray(origin, v), 1e-3, infinity, rec))
                return 0;

			auto cosine = fabs(dot(v, rec.normal) / v.length());
            if (cosine < 1e-6)
                return 0;

            auto area = (x1-x0)*(z1-z0);
            auto distance_squared = rec.t * rec.t * v.length_squared();

            return distance_squared / (cosine * area);
        }

        virtual vec3 random(const point3& origin) const override
        {
            auto random_point = point3(random_value(x0,x1), k, random_value(z0,z1));
            return random_point - origin;
        }
    
    public:
        shared_ptr<material> mp;
        fType x0, x1, z0, z1, k;
};

bool xz_rect::hit(const ray& r, fType t_min, fType t_max, hit_record& rec) const
{
    fType t = (k-r.origin.y()) / r.direction.y();
    if (t < t_min || t > t_max)
        return false;
    fType x = r.origin.x() + t*r.direction.x();
    fType z = r.origin.z() + t*r.direction.z();
    if (x < x0 || x > x1 || z < z0 || z > z1)
        return false;
    rec.u = (x-x0)/(x1-x0);
    rec.v = (z-z0)/(z1-z0);
    rec.t = t;
    auto outward_normal = vec3(0, 1, 0);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mp;
    rec.p = r.at(t);
    return true;
}

class yz_rect : public hittable
{
    public:
        yz_rect() {}

        yz_rect(fType _y0, fType _y1, fType _z0, fType _z1, fType _k, shared_ptr<material> mat)
            : y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mp(mat) 
        {
            aabb_ptr = make_shared<aabb>(point3(k - rect_pad, y0, z0), point3(k + rect_pad, y1,  z1));
        }

        virtual bool hit(const ray& r, fType t_min, fType t_max, hit_record& rec) const override;

    public:
        shared_ptr<material> mp;
        fType y0, y1, z0, z1, k;
};

bool yz_rect::hit(const ray& r, fType t_min, fType t_max, hit_record& rec) const
{
    fType t = (k-r.origin.x()) / r.direction.x();
    if (t < t_min || t > t_max)
        return false;
    fType y = r.origin.y() + t*r.direction.y();
    fType z = r.origin.z() + t*r.direction.z();
    if (y < y0 || y > y1 || z < z0 || z > z1)
        return false;
    rec.u = (y-y0)/(y1-y0);
    rec.v = (z-z0)/(z1-z0);
    rec.t = t;
    auto outward_normal = vec3(1, 0, 0);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mp;
    rec.p = r.at(t);
    return true;
}

#endif /* aarect_h */
