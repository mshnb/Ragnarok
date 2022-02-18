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
#include "tiny_obj_loader.h"

#include <vector>
#include <string>
#include <map>

class model : public hittable_list
{
public:
	model() {}
	model(std::string& file_path)
	{
		//TODO get lighting radiance from xml
		loadObj(file_path);
	}

	virtual bool hit(const ray& r, fType t_min, fType t_max, hit_record& record) const override;

	virtual fType pdf_value(const point3& origin, const vec3& v) const override;
	virtual vec3 random(const vec3& origin) const override;

	void loadLight(shared_ptr<hittable_list> lights)
	{
		//lights->add();
	}

private:
	bool loadObj(std::string& file_path)
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

#ifdef _WIN32
		file_dir += "\\";
#else
		file_dir += "/";
#endif

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
		INFO("# of materials = %d", (int)materials.size());
		INFO("# of shapes    = %d", (int)inshapes.size());

		loadMaterials(inmaterials);

		bool regen_all_normals = inattrib.normals.size() == 0;
		if (regen_all_normals)
			WARN("need to regenerate all normals");//TODO

		tinyobj::attrib_t outattrib;
		std::vector<tinyobj::shape_t> outshapes;

		tinyobj::attrib_t& attrib = regen_all_normals ? outattrib : inattrib;
		std::vector<tinyobj::shape_t>& shapes = regen_all_normals ? outshapes : inshapes;

		aabb_ptr = make_shared<aabb>(std::numeric_limits<fType>::max(), -std::numeric_limits<fType>::max());

		loadMeshes(attrib, shapes);

		INFO("aabb min = [%.4f, %.4f, %.4f]", aabb_ptr->min().x(), aabb_ptr->min().y(), aabb_ptr->min().z());
		INFO("aabb max = [%.4f, %.4f, %.4f]", aabb_ptr->max().x(), aabb_ptr->max().y(), aabb_ptr->max().z());

		return true;
	}

	//load diffuse textures and set up materials
	bool loadMaterials(std::vector<tinyobj::material_t>& inmaterials)
	{
		// Append `default` material
		inmaterials.push_back(tinyobj::material_t());

		for (size_t i = 0; i < inmaterials.size(); i++)
			INFO("material[%d].diffuse_texname = %s", int(i), inmaterials[i].diffuse_texname.c_str());

		// Load diffuse textures
		for (size_t m = 0; m < inmaterials.size(); m++)
		{
			tinyobj::material_t* mp = &inmaterials[m];

			shared_ptr<texture> tex_diffuse;
			shared_ptr<texture> tex_specular;

			//diffuse
			if (mp->diffuse_texname.length() > 0)
			{
				// Only load the texture if it is not already loaded
				if (textures.find(mp->diffuse_texname) != textures.end())
					continue;

				std::string texture_filename = file_dir + mp->diffuse_texname;
				if (!file_exists(texture_filename.c_str()))
				{
					WARN("Unable to find diffuse image file %s", texture_filename.c_str());
					exit(1);
				}

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
				if (!file_exists(texture_filename.c_str()))
				{
					WARN("Unable to find specular image file %s", texture_filename.c_str());
					exit(1);
				}

				auto tex = make_shared<image_texture>(texture_filename.c_str());
				INFO("Loaded specular texture %s with w = %d, h = %d", texture_filename.c_str(), tex->width, tex->height);

				tex_specular = static_cast<shared_ptr<texture>>(tex);
			}
			else
				tex_specular = make_shared<solid_color>(mp->specular);

			textures.insert(std::make_pair(mp->specular_texname, tex_specular));

			//TODO light
			auto mat = make_shared<phong>(tex_diffuse, tex_specular, mp->shininess, mp->ior);
			//materialname2index.insert(std::make_pair(mp->name, materials.size()));
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
			if (v1.p.x() < v2.p.x()) return true;
			else if (v1.p.x() > v2.p.x()) return false;
			if (v1.p.y() < v2.p.y()) return true;
			else if (v1.p.y() > v2.p.y()) return false;
			if (v1.p.z() < v2.p.z()) return true;
			else if (v1.p.z() > v2.p.z()) return false;
			if (v1.n.x() < v2.n.x()) return true;
			else if (v1.n.x() > v2.n.x()) return false;
			if (v1.n.y() < v2.n.y()) return true;
			else if (v1.n.y() > v2.n.y()) return false;
			if (v1.n.z() < v2.n.z()) return true;
			else if (v1.n.z() > v2.n.z()) return false;
			if (v1.uv.x() < v2.uv.x()) return true;
			else if (v1.uv.x() > v2.uv.x()) return false;
			if (v1.uv.y() < v2.uv.y()) return true;
			else if (v1.uv.y() > v2.uv.y()) return false;
			return false;
		}
	};

	//set up meshes and get bounding box's size
	bool loadMeshes(tinyobj::attrib_t& attrib, std::vector<tinyobj::shape_t>& shapes)
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

			int vertex_count = inmesh.indices.size();
			int triangle_count = vertex_count / 3;
			if (triangle_count == 0)
				continue;

			vertexBuffer.reserve(vertex_count);
			vertexBuffer.clear();

			// use the material ID of the first face.
			int current_material_id = inmesh.material_ids[0];
			if (current_material_id < 0 || current_material_id >= static_cast<int>(materials.size()))
			{
				// Invaid material ID. Use default material.
				// Default material is added to the last item in `materials`.
				current_material_id = materials.size() - 1;
			}

			shared_ptr<mesh> mesh_ptr = make_shared<mesh>(inshape.name, getMaterial(current_material_id));
			auto mesh_aabb = mesh_ptr->bounding_box();

			mesh_ptr->objects.reserve(triangle_count);
			for (size_t f = 0; f < triangle_count; f++)
			{
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

					//TODO need normalize?
					vertex.n.assign(&attrib.normals[3 * normalId]);

					vertex.uv.assign(&attrib.texcoords[2 * normalId]);
					if (flip_uv_y)
						vertex.uv[1] = 1 - vertex.uv[1];

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

				mesh_aabb->surrounding_box(mesh_aabb, tri_aabb);
				mesh_ptr->objects.push_back(tri);
			}

			mesh_ptr->vertices.reserve(vertex_count);
			mesh_ptr->normals.reserve(vertex_count);
			mesh_ptr->texcoords.reserve(vertex_count);

			for (size_t i = 0; i < vertex_count; i++)
			{
				mesh_ptr->vertices.push_back(vertexBuffer[i].p);
				mesh_ptr->normals.push_back(vertexBuffer[i].n);
				mesh_ptr->texcoords.push_back(vertexBuffer[i].uv);
			}

			aabb_ptr->surrounding_box(aabb_ptr, mesh_aabb);
		}
	}

	shared_ptr<material> getMaterial(int idx)
	{
		return materials[idx];
	}

// 	shared_ptr<material> getMaterial(std::string& name)
// 	{
// 		auto iter = materialname2index.find(name);
// 		return iter != materialname2index.end() ? materials[iter->second] : nullptr;
// 	}

// 	void genNormal(fType N[3], fType v0[3], fType v1[3], fType v2[3])
// 	{
// 		fType v10[3];
// 		v10[0] = v1[0] - v0[0];
// 		v10[1] = v1[1] - v0[1];
// 		v10[2] = v1[2] - v0[2];
// 
// 		fType v20[3];
// 		v20[0] = v2[0] - v0[0];
// 		v20[1] = v2[1] - v0[1];
// 		v20[2] = v2[2] - v0[2];
// 
// 		N[0] = v10[1] * v20[2] - v10[2] * v20[1];
// 		N[1] = v10[2] * v20[0] - v10[0] * v20[2];
// 		N[2] = v10[0] * v20[1] - v10[1] * v20[0];
// 
// 		fType len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
// 		if (len2 > 0.0) 
// 		{
// 			fType len = sqrt(len2);
// 
// 			N[0] /= len;
// 			N[1] /= len;
// 			N[2] /= len;
// 		}
// 	}

public:
	std::string file_name;
	std::string file_dir;

	std::vector<shared_ptr<material>> materials;
	std::map<std::string, shared_ptr<texture>> textures;

	//std::map<std::string, uint32_t> materialname2index;
};

bool model::hit(const ray& r, fType t_min, fType t_max, hit_record& record) const
{
	//TODO
	return true;
}

#endif /* model_h */