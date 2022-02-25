//
//  camera.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/2.
//

#ifndef camera_h
#define camera_h

#include "common.h"

class camera
{
public:
    // vfov is vertical field-of-view in degrees
    camera(point3 lookfrom, point3 lookat, vec3 vup, fType vfov, fType aspect_ratio, fType aperture = 0.0, fType focus_dist = 1.0)
    {
        fType theta = degrees_to_radians(vfov);
        fType h = tan(theta/2);
        fType viewport_height = 2.0 * h;
        fType viewport_width = aspect_ratio * viewport_height;

        w = (lookfrom - lookat).unit_vector();
        u = cross(vup, w).unit_vector();
        v = cross(w, u);

        origin = lookfrom;

        horizontal = focus_dist * viewport_width * u;
        vertical = focus_dist * viewport_height * v;
        low_left_corner = origin - horizontal/2 - vertical/2 - focus_dist*w;

        lens_radius = aperture / 2;
    }
    
    ray get_ray(fType s, fType t) const
    {
        vec3 rd = lens_radius * random_in_unit_disk();
        vec3 offset = u * rd.x + v * rd.y;

        return ray(origin + offset, (low_left_corner + s * horizontal + t * vertical - origin - offset).unit_vector());
    }
    
public:
    point3 origin;
    point3 low_left_corner;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;
    fType lens_radius;
};

#endif /* camera_h */
