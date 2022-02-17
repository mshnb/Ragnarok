//
//  aabb.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/3.
//

#ifndef aabb_h
#define aabb_h

#include "common.h"

class aabb
{
public:
    aabb() {}
	aabb(const shared_ptr<aabb> box): minimum(box->min()), maximum(box->max()) {}
    aabb(const point3& a, const point3& b): minimum(a), maximum(b) {}
    
    inline point3 min() const { return minimum; }
    inline point3 max() const { return maximum; }
    
    void reset(const point3& a, const point3& b)
    {
        minimum = a;
        maximum = b;
    }

    bool hit_slow(const ray& r, fType t_min, fType t_max) const
    {
        for (int i = 0; i < 3; i++)
        {
            fType t0 = fmin((minimum[i] - r.origin[i]) / r.direction[i],
                           (maximum[i] - r.origin[i]) / r.direction[i]);
            fType t1 = fmax((minimum[i] - r.origin[i]) / r.direction[i],
                           (maximum[i] - r.origin[i]) / r.direction[i]);
            
            t_min = fmax(t0, t_min);
            t_max = fmin(t1, t_max);
            
            if (t_max <= t_min)
                return false;
        }
        return true;
    }
    
    //fast verision by Andrew Kensler at Pixar
    inline bool hit(const ray& r, fType t_min, fType t_max) const
    {
        for (int i = 0; i < 3; i++) {
            fType invD = 1.0 / r.direction[i];
            fType t0 = (minimum[i] - r.origin[i]) * invD;
            fType t1 = (maximum[i] - r.origin[i]) * invD;
            if (invD < 0.0)
                std::swap(t0, t1);
            t_min = t0 > t_min ? t0 : t_min;
            t_max = t1 < t_max ? t1 : t_max;
            if (t_max <= t_min)
                return false;
        }
        return true;
    }
    
	void surrounding_box(shared_ptr<aabb> box0, shared_ptr<aabb> box1)
	{
		point3 small(fmin(box0->min().x(), box1->min().x()),
			fmin(box0->min().y(), box1->min().y()),
			fmin(box0->min().z(), box1->min().z()));

		point3 big(fmax(box0->max().x(), box1->max().x()),
			fmax(box0->max().y(), box1->max().y()),
			fmax(box0->max().z(), box1->max().z()));

		this->reset(small, big);
	}

public:
    point3 minimum;
    point3 maximum;
};

#endif /* aabb_h */
