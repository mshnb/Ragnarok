//
//  triangle.h
//  Ragnarok
//
//  Created by ÄªÉÙ»ª on 2022/2/18.
//

#ifndef triangle_h
#define triangle_h

#include "vector2.h"
#include "vector3.h"
#include "hittable.h"

#include <vector>

vec2 squareToUniformTriangle(const vec2& sample) 
{
	fType a = std::sqrt(1.0 - sample.x);
	return vec2(1.0 - a, a * sample.y);
}

class triangle : public hittable
{
public:
	triangle()
	{
		aabb_ptr = make_shared<aabb>(std::numeric_limits<fType>::max(), -std::numeric_limits<fType>::max());

	}

	int preCompute(uint32_t si, uint32_t pi, const std::vector<point3>& vertices)
	{
		shapeIndex = si;
		primIndex = pi;

		static const int waldModulo[4] = { 1, 2, 0, 1 };

		const point3& A = vertices[idx[0]];
		const point3& B = vertices[idx[1]];
		const point3& C = vertices[idx[2]];

		vec3 b = C - A, c = B - A, N = cross(c, b);

		k = 0;
		/* Determine the largest projection axis */
		for (int j = 0; j < 3; j++) 
			if (std::abs(N[j]) > std::abs(N[k]))
				k = j;

		uint32_t u = waldModulo[k], v = waldModulo[k + 1];
		const fType n_k = N[k], denom = b[u] * c[v] - b[v] * c[u];

		if (denom == 0) 
		{
			k = 3;
			return 1;
		}

		/* Pre-compute intersection calculation constants */
		n_u = N[u] / n_k;
		n_v = N[v] / n_k;
		n_d = dot(A, N) / n_k;
		b_nu = b[u] / denom;
		b_nv = -b[v] / denom;
		a_u = A[u];
		a_v = A[v];
		c_nu = c[v] / denom;
		c_nv = -c[u] / denom;

		return 0;
	}

	virtual bool hit(const ray& r, fType t_min, fType t_max, hit_cache& cache) const override
	{
		float o_u, o_v, o_k, d_u, d_v, d_k;
		switch (k) 
		{
		case 0:
			o_u = r.origin[1];
			o_v = r.origin[2];
			o_k = r.origin[0];
			d_u = r.direction[1];
			d_v = r.direction[2];
			d_k = r.direction[0];
			break;
		case 1:
			o_u = r.origin[2];
			o_v = r.origin[0];
			o_k = r.origin[1];
			d_u = r.direction[2];
			d_v = r.direction[0];
			d_k = r.direction[1];
			break;
		case 2:
			o_u = r.origin[0];
			o_v = r.origin[1];
			o_k = r.origin[2];
			d_u = r.direction[0];
			d_v = r.direction[1];
			d_k = r.direction[2];
			break;
		default:
			return false;
		}

		/* Calculate the plane intersection (Typo in the thesis?) */
		float t = (n_d - o_u * n_u - o_v * n_v - o_k) / (d_u * n_u + d_v * n_v + d_k);
		if (t < static_cast<float>(t_min) || t > static_cast<float>(t_max))
			return false;

		/* Calculate the projected plane intersection point */
		const float hu = o_u + t * d_u - a_u;
		const float hv = o_v + t * d_v - a_v;

		/* In barycentric coordinates */
		float u = hv * b_nu + hu * b_nv;
		float v = hu * c_nu + hv * c_nv;

		if (u >= 0 && v >= 0 && u + v <= 1.0f)
		{
			cache.shapeIndex = shapeIndex;
			cache.primIndex = primIndex;

			cache.u = static_cast<fType>(u);
			cache.v = static_cast<fType>(v);
			cache.t = static_cast<fType>(t);

			return true;
		}

		return false;
	}

	void fill_hit_record(const ray& r, const hit_cache& cache, hit_record& record, const std::vector<point3>& vertices, const std::vector<point3>& normals, const std::vector<point2>& texcoords) const
	{
		const uint32_t idx0 = idx[0], idx1 = idx[1], idx2 = idx[2];
		const point3& p0 = vertices[idx0];
		const point3& p1 = vertices[idx1];
		const point3& p2 = vertices[idx2];

		record.t = cache.t;
		record.p = r.at(cache.t);

		fType u = cache.u, v = cache.v;

		//set normal
		//use shading normals instead of geometric
		const vec3& n0 = normals[idx0];
		const vec3& n1 = normals[idx1];
		const vec3& n2 = normals[idx2];

		vec3 outward_normal = (n0 * (1.0 - u - v) + n1 * u + n2 * v).unit_vector();
		record.set_face_normal(r, outward_normal);

		//set tc
		const vec2& t0 = texcoords[idx0];
		const vec2& t1 = texcoords[idx1];
		const vec2& t2 = texcoords[idx2];
		record.uv = (1.0 - u - v) * t0 + u * t1 + v * t2;
	}

	fType surfaceArea(const std::vector<point3>& vertices)
	{
		const point3& p0 = vertices[idx[0]];
		const point3& p1 = vertices[idx[1]];
		const point3& p2 = vertices[idx[2]];
		vec3 sideA = p1 - p0, sideB = p2 - p0;
		return 0.5 * cross(sideA, sideB).length();
	}

	void sample(const std::vector<point3>& vertices, const std::vector<point3>& normals, const std::vector<point2>& texcoords, hit_record& rec) const
	{
		vec2 sample = vec2::random();

		const uint32_t idx0 = idx[0], idx1 = idx[1], idx2 = idx[2];
		const point3& p0 = vertices[idx0];
		const point3& p1 = vertices[idx1];
		const point3& p2 = vertices[idx2];

		vec2 bary = squareToUniformTriangle(sample);
		vec3 sideA = p1 - p0, sideB = p2 - p0;
		rec.p = p0 + (sideA * bary.x) + (sideB * bary.y);

		const vec3& n0 = normals[idx0];
		const vec3& n1 = normals[idx1];
		const vec3& n2 = normals[idx2];

		rec.normal.assign(n0 * (1.0 - bary.x - bary.y) + n1 * bary.x + n2 * bary.y);
		rec.normal.normalize();

		const vec2& t0 = texcoords[idx0];
		const vec2& t1 = texcoords[idx1];
		const vec2& t2 = texcoords[idx2];

		rec.uv = t0 * (1.0 - bary.x - bary.y) + t1 * bary.x + t2 * bary.y;
	}

// 	virtual fType pdf_value(const point3& origin, const vec3& v) const override;
// 	virtual vec3 random(const vec3& origin) const override;

private:
	//Pre-computed triangle representation based on Ingo Wald's TriAccel layout.
	uint32_t k;
	float n_u;
	float n_v;
	float n_d;

	float a_u;
	float a_v;
	float b_nu;
	float b_nv;

	float c_nu;
	float c_nv;

public:
	uint32_t shapeIndex;
	uint32_t primIndex;
	uint32_t idx[3];
};

#endif /* triangle_h */
