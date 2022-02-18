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
#include "hittable_list.h"

#define TINYOBJLOADER_IMPLEMENTATION
#ifndef USE_FP32
#define TINYOBJLOADER_USE_DOUBLE
#endif
#include "tiny_obj_loader.h"

class mesh : public hittable_list
{
public:
    mesh() {}
    mesh(std::string& n, shared_ptr<material> mat) : name(n), mat_ptr(mat)
    {
		aabb_ptr = make_shared<aabb>(std::numeric_limits<fType>::max(), -std::numeric_limits<fType>::max());

    }

    virtual bool hit(const ray& r, fType t_min, fType t_max, hit_record& record) const override;

    virtual fType pdf_value(const point3& origin, const vec3& v) const override;
    virtual vec3 random(const vec3& origin) const override;
    
public:
    std::string name;
    shared_ptr<material> mat_ptr;

    std::vector<point3> vertices;
	std::vector<point3> normals;
	std::vector<point2> texcoords;
};

bool mesh::hit(const ray& r, fType t_min, fType t_max, hit_record& record) const
{
    //TODO
    return true;
}

#endif /* mesh_h */
