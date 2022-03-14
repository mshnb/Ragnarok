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
#include "vector2.h"
#include "onb.h"

class material;
class hittable;

// Temporarily holds some intersection information
struct hit_cache
{
	uint32_t shapeIndex;
	uint32_t primIndex;
	fType u, v, t;
};

struct hit_record
{
    point3 p;
    vec3 normal;
    shared_ptr<hittable> shape_ptr;
    shared_ptr<material> mat_ptr;

    fType t;
    vec2 uv;

    bool front_face;
    bool hit_light;

    inline void set_face_normal(const ray& r, const vec3& outward_normal)
    {
        front_face = dot(r.direction, outward_normal) < 0;
        normal = outward_normal; // front_face ? outward_normal : -outward_normal;
    }
};

class hittable
{
public:
    virtual bool hit(const ray& r, fType t_min, fType t_max, hit_cache& cache) const = 0;
    virtual shared_ptr<aabb> bounding_box() const
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

	virtual fType getSamplingWeight() const
	{
		//TODO use all triangle's area?
		return 1.0;
	}

    bool is_light = false;

protected:
    shared_ptr<aabb> aabb_ptr;
};

#endif /* hittable_h */
