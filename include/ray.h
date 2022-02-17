//
//  ray.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/1.
//

#ifndef ray_h
#define ray_h

#include "vector3.h"

class ray
{
public:
    ray() {}
    ray(const point3& ori, const vec3& dir):origin(ori), direction(dir) {}
    
    point3 at(fType t) const
    {
        return origin + t * direction;
    }
    
public:
    point3 origin;
    vec3 direction;
};

#endif /* ray_h */
