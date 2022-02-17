//
//  main.cpp
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/1.
//

#include "common.h"
#include "hittable_list.h"
#include "color.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "aarect.h"
#include "box.h"
#include "pdf.h"
#include "bvh.h"

#ifdef OUTPUT_EXR
#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"
#endif

#include <iostream>
#include <fstream>
#include <time.h>

color ray_color(const ray& r, const color& background, const hittable& world, shared_ptr<hittable_list>& lights, int depth)
{
    hit_record rec;
    
    if(depth <= 0)
        return color(0, 0, 0);
    
    //this 1e-3 is important for reducing artifacts
    if (!world.hit(r, 1e-3, infinity, rec))
        return background;
    
    scatter_record srec;
    color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
    if (!rec.mat_ptr->scatter(r, rec, srec))
        return emitted;

    if (srec.is_specular)
        return srec.attenuation
             * ray_color(srec.specular_ray, background, world, lights, depth-1);
    
    auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
    mixture_pdf p(light_ptr, srec.pdf_ptr);

    ray scattered = ray(rec.p, p.generate().unit_vector());
    fType pdf_val = p.value(scattered.direction);
    while (pdf_val < 1e-6)
    {
        scattered.direction = p.generate().unit_vector();
        pdf_val = p.value(scattered.direction);
    }

    return emitted
        + srec.attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered)
                           * ray_color(scattered, background, world, lights, depth-1) / pdf_val;
}

hittable_list random_scene()
{
    hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            fType choose_mat = random_value();
            point3 center(a + 0.9*random_value(), 0.2, b + 0.9*random_value());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_value(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    return world;
}

hittable_list cornell_box()
{
    hittable_list objects;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    //objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<flip_face>(make_shared<xz_rect>(213, 343, 227, 332, 554, light)));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

//    shared_ptr<material> aluminum = make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
//    shared_ptr<hittable> box1 = make_shared<box>(point3(0,0,0), point3(165,330,165), aluminum);
    shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));
    objects.add(box1);

//    shared_ptr<hittable> box2 = make_shared<box>(point3(0,0,0), point3(165,165,165), white);
//    box2 = make_shared<rotate_y>(box2, -18);
//    box2 = make_shared<translate>(box2, vec3(130,0,65));
//    objects.add(box2);
    
    auto glass = make_shared<dielectric>(1.5);
    objects.add(make_shared<sphere>(point3(190,90,190), 90 , glass));
    
    return objects;
}

int main(int argc, const char * argv[])
{
    //image
    fType aspect_ratio = 16.0 / 9.0;
    int image_width = 400;
    int image_height = static_cast<int>(image_width / aspect_ratio);
    int samples_per_pixel = 100;
    int max_bounce = 50;
    
    //world
    hittable_list world;
    color background(0,0,0);
    
    //lights
    auto lights = make_shared<hittable_list>();
    lights->add(make_shared<xz_rect>(213, 343, 227, 332, 554, nullptr));
    //lights->add(make_shared<sphere>(point3(190, 90, 190), 90, nullptr));
    
    point3 lookfrom;
    point3 lookat;
    fType vfov = 40.0;
    fType aperture = 0.0;
    fType dist_to_focus = 1.0;
    
    switch (4)
    {
        case 1:
        {
            //auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
            auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
            auto material_ground = make_shared<lambertian>(checker);
            auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
            auto material_left   = make_shared<dielectric>(1.5);
            auto material_right  = make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);

            world.add(make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
            world.add(make_shared<sphere>(point3( 0.0,    0.0, -1.0),   0.5, material_center));
            world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.5, material_left));
            //the geometry is unaffected, but the surface normal points inward
            world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),  -0.45, material_left));
            world.add(make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.5, material_right));
            
            background = color(0.70, 0.80, 1.00);
            
            lookfrom = point3(-2,2,1);
            lookat = point3(0,0,-1);
            vfov = 20.0;
            break;
        }

        case 2:
        {
            world = random_scene();
            background = color(0.70, 0.80, 1.00);
            lookfrom = point3(13,2,3);
            lookat = point3(0,0,0);
            vfov = 20.0;
            aperture = 0.1;
            dist_to_focus = 10.0;
            break;
        }

        case 3:
        {
            auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
            auto material_ground = make_shared<lambertian>(checker);
            world.add(make_shared<sphere>(point3( 0.0, -100, -1.0), 100.0, material_ground));
            
            auto earth_texture = make_shared<image_texture>("earthmap.jpg");
            auto earth_surface = make_shared<lambertian>(earth_texture);
            auto globe = make_shared<sphere>(point3(0,2,0), 2, earth_surface);
            world.add(globe);
            
            auto difflight = make_shared<diffuse_light>(color(4,4,4));
            world.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));
            
            lookfrom = point3(26,3,6);
            lookat = point3(0,2,0);
            vfov = 20.0;
            
            samples_per_pixel = 400;
            
            break;
        }
        
        case 4:
        {
            world = cornell_box();
            aspect_ratio = 1.0;
            image_width = 256;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 1000;
            background = color(0,0,0);
            lookfrom = point3(278, 278, -800);
            lookat = point3(278, 278, 0);
            vfov = 40.0;
            break;
        }
            
        default:
            break;
    }
    
    //bvh_node world_wrapper(world);
    
    //camera
    vec3 vup(0,1,0);
    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus);
    
    //output image
#ifdef OUTPUT_EXR
    const char* output_path = "image.exr";
    int output_size = image_width * image_height * 3;
    float* output = new float[output_size];
#else
    std::ofstream output("image.ppm");
    if(!output.is_open())
    {
        std::cerr << "write output file failed." << std::endl;
        return -1;
    }
    output << "P3\n" << image_width << ' ' << image_height << "\n255\n";
#endif
    clock_t start_time = clock();
    
    //render
    for(int y = image_height - 1; y >= 0; y--)
    {
        std::cerr << "\rScanlines remaining: " << y << ' ' << std::flush;
        for(int x = 0; x < image_width; x++)
        {
//             if (x == 60 && y == 173)
//                 printf("\n");
            color pixel_color(0, 0, 0);
            for(int s = 0; s < samples_per_pixel; s++)
            {
                fType u = (x + random_value()) / (image_width - 1);
                fType v = (y + random_value()) / (image_height - 1);
                
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, background, world, lights, max_bounce);
            }
#ifdef OUTPUT_EXR
            int flip_y = image_height - 1 - y;
            int index = 3 * (flip_y * image_width + x);
            write_color(output, pixel_color, index, samples_per_pixel);
#else
            write_color(output, pixel_color, samples_per_pixel);
#endif
        }
    }

#ifdef OUTPUT_EXR
    const char* err;
    int ret = SaveEXR(output, image_width, image_height, 3, false, output_path, &err);
    std::cerr << "\nwriting to exr, ret = " << ret;
    if(ret < 0)
        std::cerr << ", error info: " << err;

    delete[] output;
#else
    output.close();
#endif

    clock_t finish_time = clock();
    double seconds = (finish_time - start_time) / CLOCKS_PER_SEC;
    
    std::cerr << "\nDone with " << static_cast<int>(seconds) << "s.\n";
    
    return 0;
}
