#pragma once

#include "common.h"
#include "ray.h"
#include "hittable.h"
#include "bvh.h"
#include "model.h"
#include "triangle.h"
#include "cdf.h"

#include <vector>

class Scene
{
public:
	Scene() : inited(false) {}
	bool intersect(const ray& r, hit_record& rec, fType t_min = Epsilon, fType t_max = infinity) const;

	void add_hittable(shared_ptr<hittable> obj)
	{
		shapes.push_back(obj);
		triangleFlag.push_back(false);
		temp_hittable_list.push_back(obj);
	}

	void add_model(shared_ptr<model> obj)
	{
		for (int i = 0; i < obj->objects.size(); i++)
		{
			shared_ptr<mesh> mesh_ptr = std::dynamic_pointer_cast<mesh>(obj->objects[i]);

			uint32_t shapeIndex = shapes.size();
			shapes.push_back(mesh_ptr);
			triangleFlag.push_back(true);

			const std::vector<point3>& vertices = mesh_ptr->vertices;
			for (int j = 0; j < mesh_ptr->objects.size(); j++)
			{
				shared_ptr<triangle> tri_ptr = std::dynamic_pointer_cast<triangle>(mesh_ptr->objects[j]);
				tri_ptr->preCompute(shapeIndex, j, vertices);

				temp_hittable_list.push_back(tri_ptr);
			}
		}
	}

	void init()
	{
		//buid bvh
		clock_t start_time = clock();
		printf("building bvh...\n");
		bvh_root = make_shared<bvh_node>(temp_hittable_list);
		temp_hittable_list.swap(std::vector<shared_ptr<hittable>> ());

		clock_t finish_time = clock();
		double seconds = (finish_time - start_time) / CLOCKS_PER_SEC;
		printf("building bvh end in %ds.\n", static_cast<int>(seconds));

		//build lights cdf
		int lightCount = lights->objects.size();
		if (lightCount > 0)
		{
			/* Calculate a discrete PDF to importance sample emitters */
			m_cdf = make_shared<cdf>(lightCount);
			for (auto obj : lights->objects)
			{
				auto light = std::dynamic_pointer_cast<mesh>(obj);
				light->prepareSamplingTable();
				m_cdf->append(light->getSamplingWeight());
			}

			m_cdf->normalize();
		}

		inited = true;
	}

	color sampleLights(const point3& ori_p, const vec3& ori_normal, vec3& light_dir, fType& sample_pdf)
	{
		// Randomly pick an emitter
		fType light_pdf;
		size_t index = m_cdf->sample(random_value(), light_pdf);
		//TODO support other type light
		auto light_ptr = std::dynamic_pointer_cast<mesh>(lights->objects[index]);

		// Sample on a random point on selected light's surface
		hit_record light_rec;
		color value = light_ptr->sampleDirect(ori_p, ori_normal, light_rec, light_dir, sample_pdf);

		if (sample_pdf > 1e-6) 
		{
			ray r(ori_p, light_dir);
			fType dist = light_rec.t * (1.0 - Epsilon);
			//TODO use intersect_fast
			hit_record temp;
			if (intersect(r, temp, Epsilon, dist))
				return color(0.0);

			sample_pdf *= light_pdf;
			value /= light_pdf;

			return value;
		}

		return color(0.0);
	}

	fType pdfLightDirect(const hit_record& rec, const vec3& dir)
	{
		//TODO support other type light
		auto light = std::dynamic_pointer_cast<mesh>(rec.shape_ptr);

		fType pdfDirect = light->pdfPosition() * (rec.t * rec.t) / std::abs(dot(dir, rec.normal));
		return pdfDirect * light->getSamplingWeight() * m_cdf->getNormalization();
	}

private:
	bool inited;

	//used for build bvh
	std::vector<shared_ptr<hittable>> temp_hittable_list;

	//stores all obj's pointer
	std::vector<shared_ptr<hittable>> shapes;
	std::vector<bool> triangleFlag;

	shared_ptr<bvh_node> bvh_root;

	//used for sampling lights
	shared_ptr<cdf> m_cdf;

public:
	color background;
	shared_ptr<hittable_list> lights;
};

bool Scene::intersect(const ray& r, hit_record& rec, fType t_min, fType t_max) const
{
	hit_cache cache;
	if (bvh_root->hit(r, t_min, t_max, cache))
	{
		//fill hit record
		shared_ptr<hittable> shape = shapes[cache.shapeIndex];
		if (triangleFlag[cache.shapeIndex])
		{
			//triangle
			shared_ptr<mesh> mesh_ptr = std::dynamic_pointer_cast<mesh>(shape);
			shared_ptr<triangle> tri_ptr = std::dynamic_pointer_cast<triangle>(mesh_ptr->objects[cache.primIndex]);

			tri_ptr->fill_hit_record(r, cache, rec, mesh_ptr->vertices, mesh_ptr->normals, mesh_ptr->texcoords);
			rec.shape_ptr = shape;
			rec.mat_ptr = mesh_ptr->mat_ptr;
			rec.hit_light = mesh_ptr->is_light;
		}
		else
		{
			//other shape
			//TODO
			exit(1);
		}

		return true;
	}


	return false;
}