//
//  model.h
//  Ragnarok
//
//  Created by ÄªÉÙ»ª on 2022/2/17.
//

#ifndef model_h
#define model_h

#include "vector3.h"
#include "texture.h"
#include "mesh.h"
#include "material.h"
#include "hittable_list.h"
#include "tiny_obj_loader.h"

#include <vector>
#include <string>
#include <map>

class model : public hittable_list
{
public:
	model() {}
	model(std::string& path) : file_path(path)
	{
		//TODO get lighting radiance from xml
		loadObj();
	}

	virtual bool hit(const ray& r, fType t_min, fType t_max, hit_record& record) const override;

	virtual fType pdf_value(const point3& origin, const vec3& v) const override;
	virtual vec3 random(const vec3& origin) const override;

private:
	bool loadObj()
	{
		//TODO load mesh and build aabb
		tinyobj::attrib_t inattrib;
		std::vector<tinyobj::shape_t> inshapes;
		std::vector<tinyobj::material_t> inmaterials;

		size_t slash_pos = file_path.find_last_of("/\\");
		if (slash_pos != std::string::npos)
			file_dir = file_path.substr(0, slash_pos);
		else
			file_dir = ".";

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

			if (mp->diffuse_texname.length() > 0)
			{
				// Only load the texture if it is not already loaded
				if (textures.find(mp->diffuse_texname) != textures.end())
					continue;

				std::string texture_filename = file_dir + mp->diffuse_texname;
				if (!file_exists(texture_filename))
				{
					WARN("Unable to find image file %s", texture_filename.c_str());
					exit(1);
				}

				auto tex = make_shared<image_texture>(texture_filename.c_str());
				INFO("Loaded texture %s with w = %d, h = %d", texture_filename.c_str(), tex->width, tex->height);
				
				textures.insert(std::make_pair(mp->diffuse_texname, static_cast<shared_ptr<texture>>(tex)));
			}

			//TODO create material adding to materials, and build materialname2index

		}

		return true;
	}

	//set up meshes and get bounding box's size
	bool loadMeshes(tinyobj::attrib_t& attrib, std::vector<tinyobj::shape_t>& shapes)
	{
		for (size_t s = 0; s < shapes.size(); s++) 
		{
			//TODO

		}
	}

public:
	std::string file_path;
	std::string file_dir;

	std::vector<shared_ptr<mesh>> meshes;
	std::vector<shared_ptr<material>> materials;
	std::map<std::string, shared_ptr<texture>> textures;

	std::map<std::string, uint32_t> materialname2index;
};

bool model::hit(const ray& r, fType t_min, fType t_max, hit_record& record) const
{
	//TODO
	return true;
}

#endif /* model_h */