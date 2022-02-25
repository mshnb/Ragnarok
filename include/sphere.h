//
//  sphere.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/1.
//

#ifndef sphere_h
#define sphere_h

// #include "vector2.h"
// #include "vector3.h"
// #include "hittable.h"
// #include "onb.h"
// 
// class sphere : public hittable
// {
// public:
//     sphere() {}
//     sphere(point3 cen, fType r, shared_ptr<material> m) : center(cen), radius(r), mat_ptr(m) 
//     {
//         vec3 offset(abs(radius));
// 		aabb_ptr = make_shared<aabb>(center - offset, center + offset);
//     }
// 
//     virtual bool hit(const ray& r, fType t_min, fType t_max, hit_record& record) const override;
//     
//     virtual fType pdf_value(const point3& origin, const vec3& v) const override;
//     virtual vec3 random(const vec3& origin) const override;
//     
// private:
//     static void get_sphere_uv(const point3& p, point2& uv)
//     {
//         // p: a given point on the sphere of radius one, centered at the origin.
//         // u: returned value [0,1] of angle around the Y axis from X=-1.
//         // v: returned value [0,1] of angle from Y=-1 to Y=+1.
//         //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
//         //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
//         //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>
// 
//         fType theta = acos(-p.y);
//         fType phi = atan2(-p.z, p.x) + pi;
// 
//         uv[0] = phi / (2*pi);
//         uv[1] = theta / pi;
//     }
//     
// public:
//     point3 center;
//     fType radius;
//     shared_ptr<material> mat_ptr;
// };
// 
// bool sphere::hit(const ray& r, fType t_min, fType t_max, hit_record& record) const
// {
//     vec3 oc = r.origin - center;
//     fType a = r.direction.length_squared();
//     fType half_b = dot(oc, r.direction);
//     fType c = oc.length_squared() - radius*radius;
// 
//     fType discriminant = half_b * half_b - a * c;
//     if (discriminant < 0.0)
//         return false;
//     
//     // Find the nearest root that lies in the acceptable range.
//     fType sqrtd = sqrt(discriminant);
//     fType root = (-half_b - sqrtd) / a;
//     if (root < t_min || t_max < root)
//     {
//         root = (-half_b + sqrtd) / a;
//         if (root < t_min || t_max < root)
//             return false;
//     }
// 
//     record.t = root;
//     record.p = r.at(record.t);
//     
//     vec3 outward_normal = (record.p - center) / radius;
//     record.set_face_normal(r, outward_normal);
//     get_sphere_uv(outward_normal, record.uv);
//     record.mat_ptr = mat_ptr;
//     
//     return true;
// }
// 
// fType sphere::pdf_value(const point3& origin, const vec3& v) const
// {
//     hit_record rec;
//     if (!this->hit(ray(origin, v), 0.001, infinity, rec))
//         return 0;
// 
//     auto cos_theta_max = sqrt(1 - radius*radius/(center-origin).length_squared());
//     auto solid_angle = 2*pi*(1-cos_theta_max);
// 
//     return  1 / solid_angle;
// }
// 
// vec3 sphere::random(const point3& origin) const
// {
//     vec3 direction = center - origin;
//     auto distance_squared = direction.length_squared();
//     onb uvw;
//     uvw.build_from_w(direction);
//     return uvw.world(random_to_sphere(radius, distance_squared));
// }

#endif /* sphere_h */
