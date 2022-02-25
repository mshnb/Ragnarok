//
//  onb.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/9.
//

#ifndef onb_h
#define onb_h

#include "vector3.h"

class onb
{
public:
    onb() {}
    onb(const vec3& n) { build_from_w(n); }

    inline vec3 operator[](int i) const 
    {
		switch (i)
		{
		default:
		case 0:
			return u;
		case 1:
			return w;
		case 2:
			return v;
		}
    }

    // Convert from world coordinates to local coordinates
    vec3 local(const vec3& a) const
    {
		return vec3(dot(a, u), dot(a, w), dot(a, v));
    }

    // Convert from local coordinates to world coordinates
    vec3 world(const vec3& a) const 
    {
        return u * a.x + w * a.y + v * a.z;
    }

    void build_from_w(const vec3& n);
    
public:
    vec3 u, w, v;
};

void onb::build_from_w(const vec3& n)
{
    w = n.unit_vector();
    vec3 a = fabs(w.x) > 0.9 ? vec3(0, 1, 0) : vec3(1, 0, 0);
    v = cross(w, a).unit_vector();
    u = cross(v, w);
}

#endif /* onb_h */
