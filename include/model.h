//
//  model.h
//  Ragnarok
//
//  Created by ÄªÉÙ»ª on 2022/2/17.
//

#ifndef model_h
#define model_h

#include "vector2.h"
#include "vector3.h"
#include "texture.h"
#include "mesh.h"
#include "material.h"
#include "triangle.h"
#include "hittable_list.h"
#include "bvh.h"

#ifndef USE_FP32
#define TINYOBJLOADER_USE_DOUBLE
#endif
#include "tiny_obj_loader.h"

#include <vector>
#include <string>
#include <map>

class model : public hittable_list
{
public:
	model() : hittable_list() {}

// 	virtual bool hit(const ray& r, fType t_min, fType t_max, hit_record& record) const override;
// 	virtual fType pdf_value(const point3& origin, const vec3& v) const override;
// 	virtual vec3 random(const vec3& origin) const override;

	bool loadObj(std::string& file_path, shared_ptr<hittable_list> lights, std::map<std::string, vec3>& matname2radiance)
	{
		tinyobj::attrib_t inattrib;
		std::vector<tinyobj::shape_t> inshapes;
		std::vector<tinyobj::material_t> inmaterials;

		size_t dot_pos = file_path.find_last_of('.');
		if (dot_pos == std::string::npos)
			dot_pos = file_path.length();

		size_t slash_pos = file_path.find_last_of("/\\");
		if (slash_pos != std::string::npos)
		{
			file_name = file_path.substr(slash_pos + 1, dot_pos - slash_pos - 1);
			file_dir = file_path.substr(0, slash_pos);
		}
		else
		{
			file_name = file_path.substr(0, dot_pos);
			file_dir = ".";
		}

		file_dir += "/";

		std::string warn;
		std::string err;
		bool ret = tinyobj::LoadObj(&inattrib, &inshapes, &inmaterials, &warn, &err, file_path.c_str(), file_dir.c_str(), true);
		if (!warn.empty())
			WARN("loading obj file %s with warn info: %s", file_path.c_str(), warn.c_str());
		if (!err.empty())
			WARN("loading obj file %s with error info: %s", file_path.c_str(), err.c_str());

		if (!ret)
			return false;

		INFO("loading model from %s", file_path.c_str());

		INFO("# of vertices  = %d", (int)(inattrib.vertices.size()) / 3);
		INFO("# of normals   = %d", (int)(inattrib.normals.size()) / 3);
		INFO("# of texcoords = %d", (int)(inattrib.texcoords.size()) / 2);
		INFO("# of materials = %d", (int)inmaterials.size());
		INFO("# of shapes    = %d", (int)inshapes.size());

		std::vector<int> light_mat_index;
		loadMaterials(inmaterials, matname2radiance, light_mat_index);

		if (inattrib.normals.size() == 0)
			WARN("need to regenerate all normals");//TODO

		loadMeshes(inattrib, inshapes, light_mat_index, lights);

		INFO("aabb min = [%.4f, %.4f, %.4f]", aabb_ptr->min().x, aabb_ptr->min().y, aabb_ptr->min().z);
		INFO("aabb max = [%.4f, %.4f, %.4f]", aabb_ptr->max().x, aabb_ptr->max().y, aabb_ptr->max().z);

		return true;
	}

private:

	//load diffuse textures and set up materials
	bool loadMaterials(std::vector<tinyobj::material_t>& inmaterials, std::map<std::string, vec3>& matname2radiance, std::vector<int>& light_mat_index)
	{
		// Load diffuse textures
		for (size_t m = 0; m < inmaterials.size(); m++)
		{
			tinyobj::material_t* mp = &inmaterials[m];

			shared_ptr<material> mat;

			auto iter = matname2radiance.find(mp->name);
			if (iter != matname2radiance.end()) //light
			{
				mat = make_shared<diffuse_light>(iter->second);
				light_mat_index.push_back(materials.size());
			}
			else if (abs(mp->ior - 1) > 1e-3) //dielectric
			{
				mat = make_shared<dielectric>(mp->ior);
			}
			else
			{
				shared_ptr<texture> tex_diffuse;
				shared_ptr<texture> tex_specular;

				//diffuse
				if (mp->diffuse_texname.length() > 0)
				{
					// Only load the texture if it is not already loaded
					if (textures.find(mp->diffuse_texname) != textures.end())
						continue;

					std::string texture_filename = file_dir + mp->diffuse_texname;
					auto tex = make_shared<image_texture>(texture_filename.c_str());
					INFO("Loaded diffuse texture %s with w = %d, h = %d", texture_filename.c_str(), tex->width, tex->height);

					tex_diffuse = static_cast<shared_ptr<texture>>(tex);
				}
				else
					tex_diffuse = make_shared<solid_color>(mp->diffuse);

				textures.insert(std::make_pair(mp->diffuse_texname, tex_diffuse));

				//specular
				if (mp->specular_texname.length() > 0)
				{
					// Only load the texture if it is not already loaded
					if (textures.find(mp->specular_texname) != textures.end())
						continue;

					std::string texture_filename = file_dir + mp->specular_texname;
					auto tex = make_shared<image_texture>(texture_filename.c_str());
					INFO("Loaded specular texture %s with w = %d, h = %d", texture_filename.c_str(), tex->width, tex->height);

					tex_specular = static_cast<shared_ptr<texture>>(tex);
				}
				else
					tex_specular = make_shared<solid_color>(mp->specular);

				textures.insert(std::make_pair(mp->specular_texname, tex_specular));

				mat = make_shared<phong>(tex_diffuse, tex_specular, mp->shininess);
			}
			
			materials.push_back(mat);
		}

		return true;
	}

	struct Vertex 
	{
		point3 p;
		point3 n;
		point2 uv;
	};

	/// For using vertices as keys in an associative structure
	struct vertex_key_order : public std::binary_function<Vertex, Vertex, bool> 
	{
	public:
		bool operator()(const Vertex& v1, const Vertex& v2) const 
		{
			if (v1.p.x < v2.p.x) return true;
			else if (v1.p.x > v2.p.x) return false;
			if (v1.p.y < v2.p.y) return true;
			else if (v1.p.y > v2.p.y) return false;
			if (v1.p.z < v2.p.z) return true;
			else if (v1.p.z > v2.p.z) return false;
			if (v1.n.x < v2.n.x) return true;
			else if (v1.n.x > v2.n.x) return false;
			if (v1.n.y < v2.n.y) return true;
			else if (v1.n.y > v2.n.y) return false;
			if (v1.n.z < v2.n.z) return true;
			else if (v1.n.z > v2.n.z) return false;
			if (v1.uv.x < v2.uv.x) return true;
			else if (v1.uv.x > v2.uv.x) return false;
			if (v1.uv.y < v2.uv.y) return true;
			else if (v1.uv.y > v2.uv.y) return false;
			return false;
		}
	};

	//set up meshes and get bounding box's size
	bool loadMeshes(tinyobj::attrib_t& attrib, std::vector<tinyobj::shape_t>& shapes, std::vector<int>& light_mat_index, shared_ptr<hittable_list> lights)
	{
		bool flip_uv_y = true;

		typedef std::map<Vertex, uint32_t, vertex_key_order> VertexMapType;
		VertexMapType vertexMap;
		std::vector<Vertex> vertexBuffer;

		// Collapse the mesh into a more usable form
		for (size_t s = 0; s < shapes.size(); s++) 
		{
			tinyobj::shape_t& inshape = shapes[s];
			tinyobj::mesh_t& inmesh = inshape.mesh;
			
			int triangle_count = inmesh.indices.size() / 3;
			if (triangle_count == 0)
				continue;

			vertexBuffer.clear();

			int current_material_id;
			int prev_material_id;
			shared_ptr<mesh> mesh_ptr = nullptr;

			for (size_t f = 0; f < triangle_count; f++)
			{
				current_material_id = inmesh.material_ids[f];
				if (f == 0)
				{
					prev_material_id = current_material_id;
					mesh_ptr = make_shared<mesh>(inshape.name, materials[current_material_id]);
				}
					
				if (current_material_id != prev_material_id)
				{
					int vertex_count = vertexBuffer.size();

					mesh_ptr->vertices.reserve(vertex_count);
					mesh_ptr->normals.reserve(vertex_count);
					mesh_ptr->texcoords.reserve(vertex_count);

					for (size_t i = 0; i < vertex_count; i++)
					{
						mesh_ptr->vertices.push_back(vertexBuffer[i].p);
						mesh_ptr->normals.push_back(vertexBuffer[i].n);
						mesh_ptr->texcoords.push_back(vertexBuffer[i].uv);
					}

					this->add(mesh_ptr);

					for (size_t i = 0; i < light_mat_index.size(); i++)
						if (light_mat_index[i] == prev_material_id)
						{
							mesh_ptr->is_light = true;
							lights->add(mesh_ptr);
							break;
						}

					prev_material_id = current_material_id;
					mesh_ptr = make_shared<mesh>(inshape.name, materials[current_material_id]);
				}

				shared_ptr<triangle> tri = make_shared<triangle>();
				auto tri_aabb = tri->bounding_box();

				for (size_t i = 0; i < 3; i++)
				{
					tinyobj::index_t idx = inmesh.indices[3 * f + i];

					int vertexId = idx.vertex_index;
					int normalId = idx.normal_index;
					int uvId = idx.texcoord_index;
					uint32_t key;

					Vertex vertex;
					vertex.p.assign(&attrib.vertices[3 * vertexId]);
					tri_aabb->extand(vertex.p);

					vertex.n.assign(&attrib.normals[3 * normalId]);

					vertex.uv.assign(&attrib.texcoords[2 * uvId]);
					if (flip_uv_y)
						vertex.uv[1] = 1 - vertex.uv[1];

					//modify uv
					vertex.uv.u = vertex.uv.u - (int)vertex.uv.u;
					vertex.uv.v = vertex.uv.v - (int)vertex.uv.v;

					VertexMapType::iterator it = vertexMap.find(vertex);
					if (it != vertexMap.end()) 
						key = it->second;
					else 
					{
						key = (uint32_t)vertexBuffer.size();
						vertexMap[vertex] = key;
						vertexBuffer.push_back(vertex);
					}

					tri->idx[i] = key;
				}

				mesh_ptr->add(tri);
			}

			int vertex_count = vertexBuffer.size();

			if (!mesh_ptr)
				continue;

			//handle the last mesh
			mesh_ptr->vertices.reserve(vertex_count);
			mesh_ptr->normals.reserve(vertex_count);
			mesh_ptr->texcoords.reserve(vertex_count);

			for (size_t i = 0; i < vertex_count; i++)
			{
				mesh_ptr->vertices.push_back(vertexBuffer[i].p);
				mesh_ptr->normals.push_back(vertexBuffer[i].n);
				mesh_ptr->texcoords.push_back(vertexBuffer[i].uv);
			}

			this->add(mesh_ptr);

			for (size_t i = 0; i < light_mat_index.size(); i++)
				if (light_mat_index[i] == current_material_id)
				{
					mesh_ptr->is_light = true;
					lights->add(mesh_ptr);
					break;
				}
		}

		return true;
	}

public:
	std::string file_name;
	std::string file_dir;

	std::vector<shared_ptr<material>> materials;
	std::map<std::string, shared_ptr<texture>> textures;
};

#endif /* model_h */