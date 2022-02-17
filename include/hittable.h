//
//  hittable.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/1.
//

#ifndef hittable_h
#define hittable_h

#include "common.h"
#include "ray.h"
#include "aabb.h"

class material;

struct hit_record
{
    point3 p;
    vec3 normal;
    shared_ptr<material> mat_ptr;
    fType t;
    fType u, v;
    bool front_face;
    
    inline void set_face_normal(const ray& r, const vec3& outward_normal)
    {
        front_face = dot(r.direction, outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable
{
public:
    virtual bool hit(const ray& r, fType t_min, fType t_max, hit_record& record) const = 0;
    virtual shared_ptr<aabb> bounding_box()
    {
        return aabb_ptr;
    }

    virtual fType pdf_value(const point3& origin, const vec3& v) const
    {
        return 0.0;
    }

    virtual vec3 random(const vec3& origin) const
    {
        return vec3(1, 0, 0);
    }

protected:
    shared_ptr<aabb> aabb_ptr;
};

class translate : public hittable
{
    public:
        translate(shared_ptr<hittable> p, const vec3& displacement) : ptr(p), offset(displacement) 
        {
            this->aabb_ptr = ptr->bounding_box();
            if(aabb_ptr)
                aabb_ptr->reset(aabb_ptr->min() + offset, aabb_ptr->max() + offset);
        }

        virtual bool hit(
            const ray& r, fType t_min, fType t_max, hit_record& rec) const override;

    public:
        shared_ptr<hittable> ptr;
        vec3 offset;
};

bool translate::hit(const ray& r, fType t_min, fType t_max, hit_record& rec) const
{
    ray moved_r(r.origin - offset, r.direction);
    if (!ptr->hit(moved_r, t_min, t_max, rec))
        return false;

    rec.p += offset;
    rec.set_face_normal(moved_r, rec.normal);

    return true;
}

class rotate_y : public hittable
{
    public:
        rotate_y(shared_ptr<hittable> p, fType angle);

        virtual bool hit(const ray& r, fType t_min, fType t_max, hit_record& rec) const override;

    public:
        shared_ptr<hittable> ptr;
        fType sin_theta;
        fType cos_theta;
        bool hasbox;
};

rotate_y::rotate_y(shared_ptr<hittable> p, fType angle) : ptr(p)
{
    this->aabb_ptr = ptr->bounding_box();

    fType radians = degrees_to_radians(angle);
    sin_theta = sin(radians);
    cos_theta = cos(radians);
    hasbox = aabb_ptr != nullptr;

    point3 min( infinity,  infinity,  infinity);
    point3 max(-infinity, -infinity, -infinity);

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 2; k++)
            {
                fType x = i* aabb_ptr->max().x() + (1-i)* aabb_ptr->min().x();
                fType y = j* aabb_ptr->max().y() + (1-j)* aabb_ptr->min().y();
                fType z = k* aabb_ptr->max().z() + (1-k)* aabb_ptr->min().z();

                fType newx =  cos_theta*x + sin_theta*z;
                fType newz = -sin_theta*x + cos_theta*z;

                vec3 tester(newx, y, newz);

                for (int c = 0; c < 3; c++)
                {
                    min[c] = fmin(min[c], tester[c]);
                    max[c] = fmax(max[c], tester[c]);
                }
            }
        }
    }

    //use one copy of aabb
    this->aabb_ptr->reset(min, max);
}

bool rotate_y::hit(const ray& r, fType t_min, fType t_max, hit_record& rec) const
{
    auto origin = r.origin;
    auto direction = r.direction;

    origin[0] = cos_theta*r.origin[0] - sin_theta*r.origin[2];
    origin[2] = sin_theta*r.origin[0] + cos_theta*r.origin[2];

    direction[0] = cos_theta*r.direction[0] - sin_theta*r.direction[2];
    direction[2] = sin_theta*r.direction[0] + cos_theta*r.direction[2];

    ray rotated_r(origin, direction);

    if (!ptr->hit(rotated_r, t_min, t_max, rec))
        return false;

    auto p = rec.p;
    auto normal = rec.normal;

    p[0] =  cos_theta*rec.p[0] + sin_theta*rec.p[2];
    p[2] = -sin_theta*rec.p[0] + cos_theta*rec.p[2];

    normal[0] =  cos_theta*rec.normal[0] + sin_theta*rec.normal[2];
    normal[2] = -sin_theta*rec.normal[0] + cos_theta*rec.normal[2];

    rec.p = p;
    rec.set_face_normal(rotated_r, normal);

    return true;
}

class flip_face : public hittable
{
    public:
        flip_face(shared_ptr<hittable> p) : ptr(p) {}

        virtual bool hit(const ray& r, fType t_min, fType t_max, hit_record& rec) const override
        {
            if (!ptr->hit(r, t_min, t_max, rec))
                return false;

            rec.front_face = !rec.front_face;
            return true;
        }

//         virtual bool bounding_box(aabb& output_box) const override
//         {
//             return ptr->bounding_box(output_box);
//         }

    public:
        shared_ptr<hittable> ptr;
};

#endif /* hittable_h */
