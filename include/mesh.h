//
//  mesh.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/12.
//

#ifndef mesh_h
#define mesh_h

#include "vector3.h"
#include "hittable.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

struct triangle
{
	uint32_t idx[3];
};

class mesh : public hittable
{
public:
    mesh() {}
    mesh(std::string& n) : name(n)
    {

    }

    virtual bool hit(const ray& r, fType t_min, fType t_max, hit_record& record) const override;
    
    virtual fType pdf_value(const point3& origin, const vec3& v) const override;
    virtual vec3 random(const vec3& origin) const override;
    
public:
    std::string name;
    shared_ptr<material> mat_ptr;
};

bool mesh::hit(const ray& r, fType t_min, fType t_max, hit_record& record) const
{
    //TODO
    return true;
}

#endif /* mesh_h */
