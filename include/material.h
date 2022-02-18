//
//  material.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/2.
//

#ifndef material_h
#define material_h

#include "common.h"
#include "texture.h"
#include "onb.h"
#include "pdf.h"

struct hit_record;

struct scatter_record
{
    ray specular_ray;
    bool is_specular;
    color attenuation;
    shared_ptr<pdf> pdf_ptr;
};

class material
{
public:
    virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const
    {
        return false;
    }
    
    virtual fType scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const
    {
        return 0;
    }
    
    virtual color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const
    {
        return color(0,0,0);
    }
};

class lambertian : public material
{
public:
    lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {}
    lambertian(shared_ptr<texture> a) : albedo(a) {}
    
    virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
    {
        srec.is_specular = false;
        srec.attenuation = albedo->value(rec.u, rec.v, rec.p);
        
        srec.pdf_ptr = make_shared<cosine_pdf>(rec.normal);
        //srec.pdf_ptr = new cosine_pdf(rec.normal);
        return true;
    }
    
    virtual fType scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override
    {
        auto cosine = dot(rec.normal, scattered.direction);
        return cosine < 0 ? 0 : cosine/pi;
    }
    
public:
    shared_ptr<texture> albedo;
};

class metal : public material
{
    public:
        metal(const color& a, fType f) : albedo(a), fuzz(f<1?f:1) {}
		
        virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
        {
            vec3 reflected = reflect(r_in.direction.unit_vector(), rec.normal);
            srec.specular_ray = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
            srec.attenuation = albedo;
            srec.is_specular = true;
            srec.pdf_ptr = nullptr;
            return true;
        }

    public:
        color albedo;
        fType fuzz;
};

class dielectric : public material
{
    public:
        dielectric(fType index_of_refraction) : ir(index_of_refraction) {}

        virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
        {
            srec.is_specular = true;
            srec.pdf_ptr = nullptr;
            srec.attenuation = color(1.0, 1.0, 1.0);
            
            fType refraction_ratio = rec.front_face ? (1.0/ir) : ir;

            vec3 unit_direction = r_in.direction.unit_vector();
            fType cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
            fType sin_theta = sqrt(1.0 - cos_theta*cos_theta);

            bool cannot_refract = refraction_ratio * sin_theta > 1.0;
            vec3 direction;

            if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_value())
                direction = reflect(unit_direction, rec.normal);
            else
                direction = refract(unit_direction, rec.normal, refraction_ratio);

            srec.specular_ray = ray(rec.p, direction);
            return true;
        }
    
    private:
        static fType reflectance(fType cosine, fType ref_idx)
        {
            // Use Schlick's approximation for reflectance.
            fType r0 = (1-ref_idx) / (1+ref_idx);
            r0 = r0*r0;
            return r0 + (1-r0)*pow((1 - cosine),5);
        }
    
    public:
        fType ir; // Index of Refraction
};

class diffuse_light : public material
{
    public:
        diffuse_light(shared_ptr<texture> a) : emit(a) {}
        diffuse_light(color c) : emit(make_shared<solid_color>(c)) {}

        virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
        {
            return false;
        }

        virtual color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const override
        {
            if (rec.front_face)
                return emit->value(u, v, p);
            else
                return color(0,0,0);
        }

    public:
        shared_ptr<texture> emit;
};

class phong : public material
{
public:
    phong(shared_ptr<texture> Kd, shared_ptr<texture> Ks, fType Ns, fType Ni) : diffuse(Kd), specular(Ks), shiness(Ns), ior(Ni) {}

	virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
	{
        //TODO
		srec.is_specular = false;
		srec.attenuation = diffuse->value(rec.u, rec.v, rec.p);

		srec.pdf_ptr = make_shared<cosine_pdf>(rec.normal);
		return true;
	}

	virtual fType scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override
	{
		//TODO
		auto cosine = dot(rec.normal, scattered.direction);
		return cosine < 0 ? 0 : cosine / pi;
	}

public:
	shared_ptr<texture> diffuse;
	shared_ptr<texture> specular;
    fType shiness;
    fType ior;//Index of Refraction

};

#endif /* material_h */
