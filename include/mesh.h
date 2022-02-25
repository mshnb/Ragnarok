//
//  mesh.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/12.
//

#ifndef mesh_h
#define mesh_h

#include "vector2.h"
#include "vector3.h"
#include "triangle.h"
#include "hittable_list.h"
#include "cdf.h"

class mesh : public hittable_list
{
public:
    mesh(std::string& n, shared_ptr<material> mat) : name(n), mat_ptr(mat), hittable_list() 
    {
        m_surfaceArea = m_invSurfaceArea = -1;

    }
    
    void prepareSamplingTable()
    {
        uint32_t triangleCount = objects.size();
        if (triangleCount == 0)
        {
            WARN("sampling a empty triangle mesh.");
            exit(1);
        }

		if (m_surfaceArea < 0) 
        {
            /* Generate a PDF for sampling wrt. area */
            m_cdf = make_shared<cdf>(triangleCount);
			for (size_t i = 0; i < triangleCount; i++)
				m_cdf->append(std::dynamic_pointer_cast<triangle>(objects[i])->surfaceArea(vertices));

			m_surfaceArea = m_cdf->normalize();
			m_invSurfaceArea = 1.0 / m_surfaceArea;
		}
    }

    color sampleDirect(const point3& ori_p, const vec3& ori_normal, hit_record& rec, vec3& light_dir, fType& sample_pdf)
    {
		size_t index = m_cdf->sample(random_value());
        auto tri_ptr = std::dynamic_pointer_cast<triangle>(objects[index]);
        tri_ptr->sample(vertices, normals, texcoords, rec);

        light_dir = rec.p - ori_p;
        rec.front_face = dot(light_dir, rec.normal) < 0;
        if (rec.front_face)
        {
			fType distSquared = light_dir.length_squared();
			rec.t = std::sqrt(distSquared);
			light_dir /= rec.t;

            fType dp = std::abs(dot(light_dir, rec.normal));
			sample_pdf = m_invSurfaceArea * (distSquared / dp);

			// Check that the emitter and reference position are oriented correctly with respect to each other
			if (sample_pdf > 1e-6 && dot(light_dir, ori_normal) >= 0)
				return mat_ptr->emitted(rec) / sample_pdf;
        }

        sample_pdf = 0.0;
        return color(0.0);
    }

	// Query the probability density of samplePosition() for a particular point on the surface.
	fType pdfPosition()
    {
        return m_invSurfaceArea;
    }

private:
    fType m_surfaceArea;
    fType m_invSurfaceArea;
    shared_ptr<cdf> m_cdf;

public:
    std::string name;
    shared_ptr<material> mat_ptr;

    std::vector<point3> vertices;
	std::vector<point3> normals;
	std::vector<point2> texcoords;
};

#endif /* mesh_h */
