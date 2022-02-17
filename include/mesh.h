//
//  mesh.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/12.
//

#ifndef mesh_h
#define mesh_h

#include "vector3.h"
#include "hittable.h"

class mesh : public hittable
{
public:
    mesh() {}
    mesh(point3 cen, fType r, shared_ptr<material> m) : center(cen), radius(r), mat_ptr(m) {}
    virtual bool hit(const ray& r, fType t_min, fType t_max, hit_record& record) const override;
    virtual bool bounding_box(aabb& output_box) const override;
    
    virtual fType pdf_value(const point3& origin, const vec3& v) const override;
    virtual vec3 random(const vec3& origin) const override;
    
public:
    point3 center;
    fType radius;
    shared_ptr<material> mat_ptr;
};

bool mesh::hit(const ray& r, fType t_min, fType t_max, hit_record& record) const
{
    vec3 oc = r.origin - center;
    fType a = r.direction.length_squared();
    fType half_b = dot(oc, r.direction);
    fType c = oc.length_squared() - radius*radius;

    fType discriminant = half_b * half_b - a * c;
    if (discriminant < 0.0)
        return false;
    
    // Find the nearest root that lies in the acceptable range.
    fType sqrtd = sqrt(discriminant);
    fType root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root)
    {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }

    record.t = root;
    record.p = r.at(record.t);
    
    vec3 outward_normal = (record.p - center) / radius;
    record.set_face_normal(r, outward_normal);
    get_sphere_uv(outward_normal, record.u, record.v);
    record.mat_ptr = mat_ptr;
    
    return true;
}

bool mesh::bounding_box(aabb& output_box) const
{
    fType r = abs(radius);
    output_box = aabb(center - vec3(r, r, r), center + vec3(r, r, r));
    return true;
}

#endif /* mesh_h */
