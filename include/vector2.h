//
//  vector2.h
//  Ragnarok
//
//  Created by ÄªÉÙ»ª on 2022/2/18.
//

#ifndef vector2_h
#define vector2_h

#include "common.h"

class vec2
{
public:
	vec2() : x(0), y(0) {}
	vec2(fType e0) : x(e0), y(e0) {}
	vec2(fType e0, fType e1) : x(e0), y(e1) {}

	void assign(fType* values)
	{
		x = values[0];
		y = values[1];
	}

	fType operator[] (int i) const 
	{
		switch (i)
		{
		default:
		case 0:
			return x;
		case 1:
			return y;
		}
	}
	fType& operator[] (int i) 
	{
		switch (i)
		{
		default:
		case 0:
			return x;
		case 1:
			return y;
		}
	}

	vec2& operator=(const vec2& v)
	{
		x = v[0];
		y = v[1];

		return *this;
	}

	inline static vec2 random()
	{
		return vec2(random_value(), random_value());
	}

	inline static vec2 random(fType min, fType max)
	{
		return vec2(random_value(min, max), random_value(min, max));
	}

public:
	union { fType x, u; };
	union { fType y, v; };
};

//type aliases for vec2
using point2 = vec2;

inline vec2 operator+(const vec2& u, const vec2& v)
{
	return vec2(u[0] + v[0], u[1] + v[1]);
}

inline vec2 operator-(const vec2& u, const vec2& v)
{
	return vec2(u[0] - v[0], u[1] - v[1]);
}

inline vec2 operator*(fType t, const vec2& v)
{
	return vec2(t * v[0], t * v[1]);
}

inline vec2 operator*(const vec2& v, fType t)
{
	return t * v;
}

#endif /* vector2_h */
