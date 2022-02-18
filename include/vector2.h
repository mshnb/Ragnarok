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
	vec2() : e{ 0,0 } {}
	vec2(fType e0) : e{ e0, e0 } {}
	vec2(fType e0, fType e1) : e{ e0, e1 } {}

	inline fType x() const { return e[0]; }
	inline fType y() const { return e[1]; }

	inline fType u() const { return e[0]; }
	inline fType v() const { return e[1]; }

	void assign(fType* values)
	{
		e[0] = values[0];
		e[1] = values[1];
	}

	fType operator[] (int i) const { return e[i]; }
	fType& operator[] (int i) { return e[i]; }

public:
	fType e[2];
};

//type aliases for vec2
using point2 = vec2;

#endif /* vector2_h */
