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

#include <string>

class mesh : public hittable
{
public:
    mesh() {}
    mesh(std::string& path) : file_path(path)
    {
        //TODO load mesh and build aabb

		//INFO("%s loaded with %d vertex and %d faces.", path.c_str(), numVerticesTotal, numFacesTotal);
    }

    virtual bool hit(const ray& r, fType t_min, fType t_max, hit_record& record) const override;
    
    virtual fType pdf_value(const point3& origin, const vec3& v) const override;
    virtual vec3 random(const vec3& origin) const override;
    
public:
    std::string file_path;
    shared_ptr<material> mat_ptr;
};

bool mesh::hit(const ray& r, fType t_min, fType t_max, hit_record& record) const
{

    return true;
}

#endif /* mesh_h */
