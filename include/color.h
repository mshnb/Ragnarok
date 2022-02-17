//
//  color.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/1.
//

#ifndef color_h
#define color_h

#include "vector3.h"

#include <iostream>

void write_color(float* out, color& pixel_color, int index, int samples_per_pixel)
{
	fType r = pixel_color.x();
	fType g = pixel_color.y();
	fType b = pixel_color.z();

	// Replace NaN components with zero. See explanation in Ray Tracing: The Rest of Your Life.
// 	if (r != r) r = 0.0;
// 	if (g != g) g = 0.0;
// 	if (b != b) b = 0.0;

	fType scale = 1.0 / samples_per_pixel;

	// Divide the color by the number of samples and gamma-correct for gamma=2.0
// 	r = sqrt(scale * r);
// 	g = sqrt(scale * g);
// 	b = sqrt(scale * b);

	r = scale * r;
	g = scale * g;
	b = scale * b;

    out[index + 0] = static_cast<float>(r);
    out[index + 1] = static_cast<float>(g);
    out[index + 2] = static_cast<float>(b);
}

void write_color(std::ostream& out, color& pixel_color, int samples_per_pixel)
{
    fType r = pixel_color.x();
    fType g = pixel_color.y();
    fType b = pixel_color.z();
    
    // Replace NaN components with zero. See explanation in Ray Tracing: The Rest of Your Life.
    if (r != r) r = 0.0;
    if (g != g) g = 0.0;
    if (b != b) b = 0.0;
    
    // Divide the color by the number of samples and gamma-correct for gamma=2.0
    fType scale = 1.0 / samples_per_pixel;
    r = sqrt(scale * r);
    g = sqrt(scale * g);
    b = sqrt(scale * b);
    
    //write the translated [0, 255] value of each color component
    out << static_cast<int>(256 * clamp(r, 0.0, 0.9999)) << ' '
        << static_cast<int>(256 * clamp(g, 0.0, 0.9999)) << ' '
        << static_cast<int>(256 * clamp(b, 0.0, 0.9999)) << '\n';
}

#endif /* color_h */
