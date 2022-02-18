//
//  triangle.h
//  Ragnarok
//
//  Created by ÄªÉÙ»ª on 2022/2/18.
//

#ifndef triangle_h
#define triangle_h

#include "vector3.h"
#include "hittable.h"

class triangle : public hittable
{
public:
	triangle()
	{
		aabb_ptr = make_shared<aabb>(std::numeric_limits<fType>::max(), -std::numeric_limits<fType>::max());

	}
	virtual bool hit(const ray& r, fType t_min, fType t_max, hit_record& record) const override;

	virtual fType pdf_value(const point3& origin, const vec3& v) const override;
	virtual vec3 random(const vec3& origin) const override;

public:
	uint32_t idx[3];
};

bool triangle::hit(const ray& r, fType t_min, fType t_max, hit_record& record) const
{
	//TODO
	return true;
}

#endif /* triangle_h */
