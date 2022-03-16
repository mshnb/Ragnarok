//
//  vector3.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/1.
//

#ifndef vector3_h
#define vector3_h

#include "common.h"
#include <iostream>

class vec3
{
public:
    vec3() : x(0), y(0), z(0) {}
    vec3(fType e0) : x(e0), y(e0), z(e0) {}
    vec3(fType e0, fType e1, fType e2) : x(e0), y(e1), z(e2) {}
    
	void assign(fType e0)
	{
		x = e0;
		y = e0;
		z = e0;
	}

	void assign(const vec3& v)
	{
		x = v[0];
		y = v[1];
		z = v[2];
	}

    void assign(fType* values)
    {
        x = values[0];
		y = values[1];
		z = values[2];
    }

	void assign(fType e0, fType e1, fType e2)
	{
		x = e0;
		y = e1;
		z = e2;
	}

    vec3 operator-() const { return vec3(-x, -y, -z); }
    fType operator[] (int i) const 
    {
		switch (i)
		{
		default:
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
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
		case 2:
			return z;
		}
	}
    
    vec3& operator=(const vec3& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        
        return *this;
    }
    
    vec3& operator+=(const vec3& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        
        return *this;
    }
    
    vec3& operator*=(const fType t)
    {
        x *= t;
        y *= t;
        z *= t;
        return *this;
    }
    
	vec3& operator*=(const vec3& v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

    vec3& operator/=(const fType t)
    {
        return *this *= 1/t;
    }
    
    fType length() const
    {
        return sqrt(length_squared());
    }
    
    fType length_squared() const
    {
        return x * x + y * y + z * z;
    }

    bool near_zero() const
    {
        // Return true if the vector is close to zero in all dimensions.
        const fType s = 1e-8;
        return (fabs(x) < s) && (fabs(y) < s) && (fabs(z) < s);
    }
    
    void normalize()
    {
        fType t = 1 / this->length();
        x *= t;
        y *= t;
        z *= t;
    }

    inline vec3 unit_vector() const
    {
        fType t = 1 /this->length();
        return vec3(t*x, t*y, t*z);
    }
    
    inline static vec3 random()
    {
        return vec3(random_value(), random_value(), random_value());
    }
    
    inline static vec3 random(fType min, fType max)
    {
        return vec3(random_value(min, max), random_value(min, max), random_value(min, max));
    }
    
    inline fType max() const 
    {
        return std::max(x, std::max(y, z));
    }

    //for color
	inline fType getLuminance() const 
    {
		return r * 0.212671 + g * 0.715160 + b * 0.072169;
	}

public:
	union { fType x, r; };
	union { fType y, g; };
	union { fType z, b; };
};

//type aliases for vec3
using point3 = vec3;
using color = vec3;

//vec3 utility functions
inline std::ostream& operator<<(std::ostream& out, const vec3& v)
{
    return out << v[0] << ' ' << v[1] << ' ' << v[2];
}

inline vec3 operator+(const vec3 &u, const vec3 &v)
{
    return vec3(u[0] + v[0], u[1] + v[1], u[2] + v[2]);
}

inline vec3 operator-(const vec3 &u, const vec3 &v)
{
    return vec3(u[0] - v[0], u[1] - v[1], u[2] - v[2]);
}

inline vec3 operator*(const vec3 &u, const vec3 &v)
{
    return vec3(u[0] * v[0], u[1] * v[1], u[2] * v[2]);
}

inline vec3 operator*(fType t, const vec3 &v)
{
    return vec3(t*v[0], t*v[1], t*v[2]);
}

inline vec3 operator*(const vec3 &v, fType t)
{
    return t * v;
}

inline vec3 operator/(vec3 v, fType t)
{
    return (1/t) * v;
}

inline fType dot(const vec3& u, const vec3& v)
{
    return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
}

inline vec3 cross(const vec3& u, const vec3& v)
{
    return vec3(u[1] * v[2] - u[2] * v[1],
                u[2] * v[0] - u[0] * v[2],
                u[0] * v[1] - u[1] * v[0]);
}

vec3 random_in_unit_sphere()
{
    while(1)
    {
        vec3 p = vec3::random(-1, 1);
        if(p.length_squared() >= 1.0)
            continue;
        return p;
    }
}

//TODO not real random on sphere face
vec3 random_unit_vector()
{
    return random_in_unit_sphere().unit_vector();
}

vec3 random_in_hemisphere(const vec3& normal)
{
    vec3 in_unit_sphere = random_in_unit_sphere();
    if (dot(in_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
        return in_unit_sphere;
    else
        return -in_unit_sphere;
}

vec3 random_in_unit_disk()
{
    while (1) {
        vec3 p = vec3(random_value(-1,1), random_value(-1,1), 0);
        if (p.length_squared() >= 1)
            continue;
        return p;
    }
}

inline vec3 random_cosine_direction()
{
    auto r1 = random_value();
    auto r2 = random_value();
    auto z = sqrt(1-r2);

    auto phi = 2 * PI * r1;
    auto x = cos(phi)*sqrt(r2);
    auto y = sin(phi)*sqrt(r2);

    return vec3(x, y, z);
}

inline vec3 random_to_sphere(double radius, double distance_squared)
{
    auto r1 = random_value();
    auto r2 = random_value();
    auto z = 1 + r2*(sqrt(1-radius*radius/distance_squared) - 1);

    auto phi = 2 * PI * r1;
    auto x = cos(phi)*sqrt(1-z*z);
    auto y = sin(phi)*sqrt(1-z*z);

    return vec3(x, y, z);
}

vec3 reflect(const vec3& v, const vec3& n)
{
    return v - 2*dot(v,n)*n;
}

vec3 refract(const vec3& uv, const vec3& n, fType etai_over_etat)
{
    fType cos_theta = fmin(dot(-uv, n), 1.0);
    vec3 r_out_perp =  etai_over_etat * (uv + cos_theta*n);
    vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

#endif /* vector3_h */
