//
//  main.cpp
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/1.
//

#include "common.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "box.h"
#include "bvh.h"
#include "model.h"
#include "scene.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

#include "tinyxml2.h"

#include <iostream>
#include <fstream>
#include <time.h>
#include <omp.h>

std::string model_name = "cornell-box";
std::string resource_dir = "../resource";

int samples_per_pixel = 128;

int max_depth = 10;
int rr_depth = 3;
bool gamma_correct = false;

//TODO use while instead of recursion
color ray_color(const ray& r, Scene& scene, int depth, fType bsdf_pdf = 0.0)
{
	if (depth >= max_depth)
		return color(0.0);

    hit_record rec;
    if (!scene.intersect(r, rec))
        return scene.background;
    
    auto bsdf = rec.mat_ptr;
    if (rec.hit_light)
    {
        if(depth == 0)
            return bsdf->emitted(rec);

        if (!rec.front_face)
            return color(0.0);

        /* Compute the prob. of generating that direction using the
           implemented direct illumination sampling technique */
		fType light_dir_pdf = scene.pdfLightDirect(rec, r.direction);
        fType weight = mix_weight(bsdf_pdf, light_dir_pdf);
        return bsdf->emitted(rec) * weight;
    }   

    color ret(0.0);

	//sample direct illumination
    vec3 light_dir;
	fType light_pdf;
	color direct = scene.sampleLights(rec.p, rec.normal, light_dir, light_pdf);
    if (!direct.near_zero())
    {
		onb shadingFrame(rec.normal);
		vec3 wi = -shadingFrame.local(r.direction);
        vec3 wo = shadingFrame.local(light_dir);

        color bsdfVal = bsdf->eval(rec, wi, wo);
        if (!bsdfVal.near_zero())
        {
			// Calculate prob. of having generated that direction using BSDF sampling
			fType bsdfPdf = bsdf->pdf(rec, wi, wo);

			/* Weight using the power heuristic */
			fType weight = mix_weight(light_pdf, bsdfPdf);
            ret += direct * bsdfVal * weight;
        }
    }

	//sample indirect illumination
    scatter_record srec;
    if (!bsdf->scatter(r.direction, rec, srec) || srec.pdf_value < 1e-6)
        return ret;

    //TODO Prevent light leaks due to the use of shading normals

    // Russian roulette
    fType rr_weight = 1.0;
    if (depth >= rr_depth)
    {
        //TODO rr = std::min(throughput.max() * eta * eta, 0.95);
        fType rr = 0.618;
        if (random_value() > rr)
            return ret;

        rr_weight = 1.0 / rr;
    }

    color indirect = rr_weight * srec.attenuation * ray_color(srec.scatter_ray, scene, depth + 1, srec.pdf_value) / srec.pdf_value;
    return ret + indirect;
}

int main(int argc, const char * argv[])
{
    //image
    fType aspect_ratio;
    int image_width, image_height;

    point3 lookfrom;
    point3 lookat;
	vec3 vup(0, 1, 0);

    fType vfov = 40.0;
    fType aperture = 0.0;
    fType dist_to_focus = 1.0;
    
	//scene
	Scene scene;
    std::string model_path = resource_dir + "/" + model_name + "/" + model_name + ".obj";
	std::string xml_path = resource_dir + "/" + model_name + "/" + model_name + ".xml";

	tinyxml2::XMLDocument doc;
	doc.LoadFile(xml_path.c_str());
    if (doc.Error())
    {
        WARN("error occur when parsing %s, info: [line %d]%s", xml_path.c_str(), doc.ErrorLineNum(), doc.ErrorStr());
        exit(1);
    }

    tinyxml2::XMLElement* camera_node = doc.FirstChildElement("camera");
    if (PARSE_VECTOR3(camera_node, "eye", lookfrom) != 3
        || PARSE_VECTOR3(camera_node, "lookat", lookat) != 3
        || PARSE_VECTOR3(camera_node, "up", vup) != 3
        || PARSE_VECTOR3(camera_node, "lookat", lookat) != 3
        || PARSE_FLOAT(camera_node, "fovy", vfov) != 1
        || PARSE_INT(camera_node, "width", image_width) != 1
        || PARSE_INT(camera_node, "height", image_height) != 1)
    {
		WARN("error occur when parsing camera params");
		exit(1);
    }

    std::map<std::string, vec3> matname2radiance;
    tinyxml2::XMLElement* light_node = doc.FirstChildElement("light");
    while (light_node)
    {
        vec3 radiance;
        std::string mat_name(light_node->Attribute("mtlname"));
        if(mat_name.empty() || PARSE_RADIANCE(light_node, radiance) != 3)
		{
			WARN("error occur when parsing light params");
			exit(1);
		}

        matname2radiance.insert(std::make_pair(mat_name, radiance));
        light_node = light_node->NextSiblingElement("light");
    }

	doc.Clear();

    aspect_ratio = static_cast<fType>(image_width) / static_cast<fType>(image_height);

	//lights
	scene.lights = make_shared<hittable_list>();
	shared_ptr<model> model_ptr = make_shared<model>();
    if (!model_ptr->loadObj(model_path, scene.lights, matname2radiance))
        exit(1);

    scene.add_model(model_ptr);
    scene.init();

    //camera
    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus);
    
    //output exr
    std::string output_file = model_name + "_" + std::to_string(samples_per_pixel) + ".exr";
    int output_size = image_width * image_height * 3;
    float* output = new float[output_size];

    //render
    fType sample_scale = 1.0 / samples_per_pixel;
    int total_samples = image_width * image_height * samples_per_pixel;
    int current_samples = 0;

    clock_t start_time = clock();

#pragma omp parallel for
    for(int y = image_height - 1; y >= 0; y--)
    {
        for(int x = 0; x < image_width; x++)
        {
            color pixel_color(0, 0, 0);
            for(int s = 0; s < samples_per_pixel; s++)
            {
                fType u = (x + random_value()) / (image_width - 1);
                fType v = (y + random_value()) / (image_height - 1);

                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, scene, 0);
                current_samples++;
            }
            int flip_y = image_height - 1 - y;
            int index = 3 * (flip_y * image_width + x);

            //write color
			fType r = sample_scale * pixel_color.r;
			fType g = sample_scale * pixel_color.g;
			fType b = sample_scale * pixel_color.b;

			// Divide the color by the number of samples and gamma-correct for gamma = 2.0
            if (gamma_correct)
            {
				fType gamma = 1.0 / 2.2;
				r = std::pow(r, gamma);
				g = std::pow(g, gamma);
				b = std::pow(b, gamma);
            }

            output[index + 0] = static_cast<float>(r);
            output[index + 1] = static_cast<float>(g);
            output[index + 2] = static_cast<float>(b);
        }
        printf("\rrendering %.2f%%...", 100.0f * (float)current_samples / total_samples);
    }

    printf("\rrendering %.2f%%...", 100.0f);

    const char* err;
    int ret = SaveEXR(output, image_width, image_height, 3, false, output_file.c_str(), &err);
    std::cerr << "\nwriting to " << output_file << ", ret = " << ret;
    if(ret < 0)
        std::cerr << ", error info: " << err;

    delete[] output;

    clock_t finish_time = clock();
    double seconds = (finish_time - start_time) / CLOCKS_PER_SEC;
    
    std::cerr << "\nDone with " << static_cast<int>(seconds) << "s.\n";
    
    return 0;
}
