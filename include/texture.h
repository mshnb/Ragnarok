//
//  texture.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/3.
//

#ifndef texture_h
#define texture_h

#include <iostream>

#include "vector2.h"
#include "common.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class texture
{
    public:
        virtual color value(const vec2& uv, const point3& p) const = 0;
        virtual color getMinimum() const { return color(0.0); }
		virtual color getMaximum() const { return color(0.0); }
		virtual color getAverage() const { return color(0.0); }
		virtual void applyScale(fType scale) {}
};

class solid_color : public texture
{
    public:
        solid_color() {}
        solid_color(color c) : color_value(c) {}
		solid_color(fType rgb[]) : solid_color(color(rgb[0], rgb[1], rgb[2])) {}
        solid_color(fType red, fType green, fType blue) : solid_color(color(red,green,blue)) {}

        virtual color value(const vec2& uv, const vec3& p) const override
        {
            return color_value;
        }

        virtual color getMinimum() const override
        {
            return color_value;
        }

		virtual color getMaximum() const override
		{
			return color_value;
		}

		virtual color getAverage() const override
		{
			return color_value;
		}

        virtual void applyScale(fType scale) override
        {
            color_value *= scale;
        }

    private:
        color color_value;
};

//TODO add mipmap
class image_texture : public texture
{
    public:
        const static int bytes_per_pixel = 3;

        image_texture() : data(nullptr), width(0), height(0), bytes_per_scanline(0) {}

        image_texture(const char* filename)
        {
            auto components_per_pixel = bytes_per_pixel;

            unsigned char* raw_data = stbi_load(filename, &width, &height, &components_per_pixel, components_per_pixel);

            if (!raw_data)
            {
                WARN("Could not load texture image file %s", filename);
                exit(1);
            }

            bytes_per_scanline = bytes_per_pixel * width;

            color minimum(std::numeric_limits<fType>::max());
            color maximum(-std::numeric_limits<fType>::max());
            color average(0.0);

			int size = width * height;
            data = new fType[size * bytes_per_pixel];

			const fType color_scale = 1.0 / 255.0;
            for (int i = 0; i < size; i++)
            {
                for (int d = 0; d < bytes_per_pixel; d++)
                {
                    int index = i * bytes_per_pixel + d;
					fType value = static_cast<fType>(raw_data[index]) * color_scale;

                    //invert gamma correction
                    value = std::pow(value, 2.2);

					minimum[d] = std::min(minimum[d], value);
                    maximum[d] = std::max(maximum[d], value);
                    average[d] += value;
                    data[index] = value;
                }
            }

            m_minimum = minimum;
            m_maximum = maximum;
            m_average = average / size;

            stbi_image_free(raw_data);
        }

        ~image_texture()
        {
            if (data)
                delete[] data;
        }

        virtual color value(const vec2& uv, const vec3& p) const override
        {
            // If we have no texture data, then return solid cyan as a debugging aid.
            if (data == nullptr)
                return color(0,1,1);

            bool flip_v = true;

            // Clamp input texture coordinates to [0,1] x [1,0]
            fType u = fmod(uv.u, 1.0f);
            fType v = fmod(uv.v, 1.0f);

            if (u < 0)
                u += 1.0;

			if (v < 0)
				v += 1.0;

            if (flip_v)
                v = 1.0 - v;

            auto i = static_cast<int>(u * width);
            auto j = static_cast<int>(v * height);

            // Clamp integer mapping, since actual coordinates should be less than 1.0
            if (i >= width)  i = width-1;
            if (j >= height) j = height-1;

            auto pixel = data + j * bytes_per_scanline + i * bytes_per_pixel;
            return color(pixel[0], pixel[1], pixel[2]);
        }

		virtual color getMinimum() const override
		{
			return m_minimum;
		}

		virtual color getMaximum() const override
		{
			return m_maximum;
		}

		virtual color getAverage() const override
		{
			return m_average;
		}

		virtual void applyScale(fType scale) override
		{
			int size = width * height;
			for (int i = 0; i < size; i++)
			{
				for (int d = 0; d < bytes_per_pixel; d++)
				{
					int index = i * bytes_per_pixel + d;
					data[index] *= scale;
				}
			}

            m_minimum *= scale;
            m_maximum *= scale;
            m_average *= scale;
		}

    private:
        color m_minimum;
        color m_maximum;
        color m_average;

    public:
        fType *data;

        int width, height;
        int bytes_per_scanline;
};

#endif /* texture_h */
