//
//  material.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/2.
//

#ifndef material_h
#define material_h

#include "vector3.h"
#include "common.h"
#include "texture.h"
#include "onb.h"

struct hit_record;

vec2 squareToUniformDiskConcentric(const vec2& sample)
{
    fType r1 = 2.0 * sample.x - 1.0;
    fType r2 = 2.0 * sample.y - 1.0;

    /* Modified concencric map code with less branching (by Dave Cline), see
       http://psgraphics.blogspot.ch/2011/01/improved-code-for-concentric-map.html */
    fType phi, r;
    if (r1 == 0 && r2 == 0)
        r = phi = 0;
    else if (r1 * r1 > r2 * r2)
    {
        r = r1;
        phi = (PI / 4.0) * (r2 / r1);
    }
    else
    {
        r = r2;
        phi = (PI / 2.0) - (r1 / r2) * (PI / 4.0);
    }

    return vec2(r * cos(phi), r * sin(phi));
}

vec3 squareToCosineHemisphere(const vec2& sample)
{
    vec2 p = squareToUniformDiskConcentric(sample);
    fType z = std::sqrt(std::max(0.0, 1.0 - p.x * p.x - p.y * p.y));

    /* Guard against numerical imprecisions */
    if (z == 0)
        z = 1e-10;

    return vec3(p.x, z, p.y).unit_vector();
}

struct scatter_record
{
    ray scatter_ray;
    color attenuation;
    fType pdf_value;
};

class material
{
public:
    virtual bool scatter(const vec3& in_dir, const hit_record& rec, scatter_record& srec) const
    {
        return false;
    }
    
    virtual fType pdf(const hit_record& rec, const vec3& wi, const vec3& wo, int scatter_type = -1) const
    {
        return 0;
    }
    
    virtual color eval(const hit_record& rec, const vec3& wi, const vec3& wo, int scatter_type = -1) const
    {
        return color(0.0);
    }

    virtual color emitted(const hit_record& rec) const
    {
        return color(0.0);
    }
};

class dielectric : public material
{
    public:
        dielectric(fType index_of_refraction) : ior(index_of_refraction) {}

        //scatter_type -1:all 0:reflect 1:refract
		virtual color eval(const hit_record& rec, const vec3& wi, const vec3& wo, int scatter_type = -1) const override
		{
            bool sampleReflection = scatter_type == -1 || scatter_type == 0;
            bool sampleTransmission = scatter_type == -1 || scatter_type == 1;

            fType cosThetaT;
            fType fresnel = fresnelDielectric(wi.y, cosThetaT, ior);

            if (wi.y * wo.y >= 0) 
            {
                vec3 wi_reflect(-wi.x, wi.y, -wi.z);
                if (!sampleReflection || std::abs(dot(wi_reflect, wo) - 1) > 1e-3)
                    return color(0.0);

                // assume dielectric material has color white
                return color(fresnel);
            }
            else 
            {
                vec3 wi_refract = refract(wi, cosThetaT);
                if (!sampleTransmission || std::abs(dot(wi_refract, wo) - 1) > 1e-3)
                    return color(0.0);

                /* Radiance must be scaled to account for the solid angle compression
                   that occurs when crossing the interface. */
                fType factor = cosThetaT < 0 ? 1.0 / ior : ior;
                return factor * factor * (1.0 - fresnel);
            }
		}

        //scatter_type -1:all 0:reflect 1:refract
		virtual fType pdf(const hit_record& rec, const vec3& wi, const vec3& wo, int scatter_type = -1) const override
		{
			bool sampleReflection = scatter_type == -1 || scatter_type == 0;
			bool sampleTransmission = scatter_type == -1 || scatter_type == 1;

            fType cosThetaT;
            fType fresnel = fresnelDielectric(wi.y, cosThetaT, ior);

			if (wi.y * wo.y >= 0)
            {
                vec3 wi_reflect(-wi.x, wi.y, -wi.z);
				if (!sampleReflection || std::abs(dot(wi_reflect, wo) - 1) > 1e-3)
					return 0.0;

				return sampleTransmission ? fresnel : 1.0;
			}
			else 
            {
                vec3 wi_refract = refract(wi, cosThetaT);
				if (!sampleTransmission || std::abs(dot(wi_refract, wo) - 1) > 1e-3)
					return 0.0;

				return sampleReflection ? (1.0 - fresnel) : 1.0;
			}
		}

        virtual bool scatter(const vec3& in_dir, const hit_record& rec, scatter_record& srec) const override
        {
			onb shadingFrame(rec.normal);
			vec3 wi = -shadingFrame.local(in_dir);
			if (wi.y < 1e-6)
				return false;

			fType cosThetaT;
			fType fresnel = fresnelDielectric(wi.y, cosThetaT, ior);
            vec3 wo;

            if (random_value() <= fresnel) 
            {
//                     bRec.sampledComponent = 0;
//                     bRec.sampledType = EDeltaReflection;
                wo = vec3(-wi.x, wi.y, -wi.z);
                srec.attenuation.assign(1, 1, 1);
				srec.pdf_value = fresnel;
            }
            else 
            {
//                     bRec.sampledComponent = 1;
//                     bRec.sampledType = EDeltaTransmission;
                wo = refract(wi, cosThetaT);
                //bRec.eta = cosThetaT < 0 ? ior : 1.0 / ior;
                /* Radiance must be scaled to account for the solid angle compression
                    that occurs when crossing the interface. */
                fType factor = cosThetaT < 0 ? 1.0 / ior : ior;
                srec.attenuation.assign(factor * factor);
				srec.pdf_value = 1.0 - fresnel;
            }

			if (wo.y < 1e-6)
				return false;

			srec.scatter_ray = ray(rec.p, shadingFrame.world(wo));
            return true;
        }
    
    private:
		inline vec3 refract(const vec3& wi, fType cosThetaT) const 
        {
			fType scale = -(cosThetaT < 0 ? 1.0 / ior : ior);
			return vec3(scale * wi.x, cosThetaT, scale * wi.z);
		}

        fType fresnelDielectric(fType cosThetaI_, fType& cosThetaT_, fType eta) const
        {
            if (eta == 1) 
            {
                cosThetaT_ = -cosThetaI_;
                return 0.0;
            }

            /* Using Snell's law, calculate the squared sine of the
               angle between the normal and the transmitted ray */
            fType scale = (cosThetaI_ > 0) ? 1 / eta : eta,
                cosThetaTSqr = 1 - (1 - cosThetaI_ * cosThetaI_) * (scale * scale);

            /* Check for total internal reflection */
            if (cosThetaTSqr <= 0.0) 
            {
                cosThetaT_ = 0.0;
                return 1.0;
            }

            /* Find the absolute cosines of the incident/transmitted rays */
            fType cosThetaI = std::abs(cosThetaI_);
            fType cosThetaT = std::sqrt(cosThetaTSqr);

            fType Rs = (cosThetaI - eta * cosThetaT)
                / (cosThetaI + eta * cosThetaT);
            fType Rp = (eta * cosThetaI - cosThetaT)
                / (eta * cosThetaI + cosThetaT);

            cosThetaT_ = (cosThetaI_ > 0) ? -cosThetaT : cosThetaT;

            /* No polarization -- return the unpolarized reflectance */
            return 0.5 * (Rs * Rs + Rp * Rp);
        }

        static fType reflectance(fType cosine, fType ref_idx)
        {
            // Use Schlick's approximation for reflectance.
            fType r0 = (1-ref_idx) / (1+ref_idx);
            r0 = r0*r0;
            return r0 + (1-r0)*pow((1 - cosine),5);
        }
    
    public:
        fType ior; // Index of Refraction
};

class diffuse_light : public material
{
    public:
        diffuse_light(shared_ptr<texture> a) : emit(a) {}
        diffuse_light(color c) : emit(make_shared<solid_color>(c)) {}

        virtual bool scatter(const vec3& in_dir, const hit_record& rec, scatter_record& srec) const override
        {
            return false;
        }

        virtual color emitted(const hit_record& rec) const override
        {
            if (rec.front_face)
                return emit->value(rec.uv, rec.p);
            else
                return color(0,0,0);
        }

    public:
        shared_ptr<texture> emit;
};

class phong : public material
{
public:
    phong(shared_ptr<texture> Kd, shared_ptr<texture> Ks, fType Ns) : diffuse(Kd), specular(Ks), shiness(Ns) 
    {
        if (ensureEnergyConservation)
        {
			fType actualMax = (diffuse->getMaximum() + specular->getMaximum()).max();
            if (actualMax > 1.0)
            {
                fType scale = 0.99 * (1.0 / actualMax);
                diffuse->applyScale(scale);
                specular->applyScale(scale);
            }
        }

        fType dAvg = diffuse->getAverage().getLuminance();
		fType sAvg = specular->getAverage().getLuminance();

        specularSamplingWeight = sAvg / (dAvg + sAvg);
    }

    //scatter_type -1:all 0:specular 1:diffuse
	virtual color eval(const hit_record& rec, const vec3& wi, const vec3& wo, int scatter_type = -1) const override
    {
        fType diffuseSamplingWeight = 1.0 - specularSamplingWeight;

		bool hasSpecular = specularSamplingWeight > 0 && (scatter_type == -1 || scatter_type == 0);
		bool hasDiffuse = diffuseSamplingWeight > 0 && (scatter_type == -1 || scatter_type == 1);

		color result(0.0);
		if (hasSpecular) 
        {
            vec3 wi_r(-wi.x, wi.y, -wi.z);
            fType alpha = dot(wo, wi_r);
			if (alpha > 0)
				result += specular->value(rec.uv, rec.p) * ((shiness + 2) * INV_TWOPI * std::pow(alpha, shiness));
		}

		if (hasDiffuse)
			result += diffuse->value(rec.uv, rec.p) * INV_PI;

		return result * wo.y;
	}

    //scatter_type -1:all 0:specular 1:diffuse
	virtual fType pdf(const hit_record& rec, const vec3& wi, const vec3& wo, int scatter_type = -1) const override
    {
		fType diffuseSamplingWeight = 1.0 - specularSamplingWeight;

		bool hasSpecular = specularSamplingWeight > 0 && (scatter_type == -1 || scatter_type == 0);
		bool hasDiffuse = diffuseSamplingWeight > 0 && (scatter_type == -1 || scatter_type == 1);

		fType diffuseProb = 0.0, specProb = 0.0;

		if (hasSpecular) 
        {
            vec3 wi_r(-wi.x, wi.y, -wi.z);
            fType alpha = dot(wo, wi_r);
			if (alpha > 0)
				specProb = std::pow(alpha, shiness) * (shiness + 1.0) * INV_TWOPI;
		}

		if (hasDiffuse)
			diffuseProb = wo.y * INV_PI;

		if (hasDiffuse && hasSpecular)
			return specularSamplingWeight * specProb + diffuseSamplingWeight * diffuseProb;
		else if (hasDiffuse)
			return diffuseProb;
		else if (hasSpecular)
			return specProb;
		else
			return 0.0;
	}

	virtual bool scatter(const vec3& in_dir, const hit_record& rec, scatter_record& srec) const override
	{
		vec2 sample = vec2::random();

		bool choseSpecular = true;
		fType diffuseSamplingWeight = 1.0 - specularSamplingWeight;

		if (sample.x < specularSamplingWeight)
			sample.x /= specularSamplingWeight;
		else
		{
			sample.x = (sample.x - specularSamplingWeight) / diffuseSamplingWeight;
			choseSpecular = false;
		}

		onb shadingFrame(rec.normal);
		vec3 wi = -shadingFrame.local(in_dir);
		if (wi.y < 1e-6)
			return false;

		vec3 wo;
		if (choseSpecular)
		{
			/* Sample from a Phong lobe centered around (0, 1, 0) */
			fType sinAlpha = std::sqrt(1 - std::pow(sample.y, 2 / (shiness + 1)));
			fType cosAlpha = std::pow(sample.y, 1 / (shiness + 1));
			fType phi = 2.0 * PI * sample.x;
			vec3 localDir(sinAlpha * std::cos(phi), cosAlpha, sinAlpha * std::sin(phi));
            vec3 wi_r(-wi.x, wi.y, -wi.z);
			/* Rotate into the correct coordinate system */
			wo = onb(wi_r).world(localDir);
		}
		else
			wo = squareToCosineHemisphere(sample);

		if (wo.y < 1e-6)
			return false;

        int scatter_type = choseSpecular ? 0 : 1;
		srec.pdf_value = pdf(rec, wi, wo, scatter_type);
        srec.attenuation = eval(rec, wi, wo, scatter_type);
		srec.scatter_ray = ray(rec.p, shadingFrame.world(wo));

		return true;
	}

private:
    fType specularSamplingWeight;

public:
	shared_ptr<texture> diffuse;
	shared_ptr<texture> specular;
    fType shiness;
};

#endif /* material_h */
