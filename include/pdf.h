//
//  pdf.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/10.
//

#ifndef pdf_h
#define pdf_h

#include "common.h"
#include "onb.h"


class pdf
{
    public:
        virtual ~pdf() {}

        virtual fType value(const vec3& direction) const = 0;
        virtual vec3 generate() const = 0;
};

class cosine_pdf : public pdf
{
    public:
        cosine_pdf(const vec3& w) { uvw.build_from_w(w); }

        virtual fType value(const vec3& direction) const override
        {
            fType cosine = dot(direction.unit_vector(), uvw.w());

            if (cosine != cosine)
                abort();

            return (cosine <= 0) ? 0 : cosine/pi;
        }

        virtual vec3 generate() const override
        {
            return uvw.local(random_cosine_direction());
        }

    public:
        onb uvw;
};

class hittable_pdf : public pdf
{
    public:
        hittable_pdf(shared_ptr<hittable> p, const point3& ori) : ptr(p), origin(ori) {}

        virtual fType value(const vec3& direction) const override
        {
            return ptr->pdf_value(origin, direction);
        }

        virtual vec3 generate() const override
        {
            return ptr->random(origin);
        }

    public:
        point3 origin;
        shared_ptr<hittable> ptr;
};

class mixture_pdf : public pdf
{
    public:
        mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1)
        {
            p[0] = p0;
            p[1] = p1;
        }

        virtual fType value(const vec3& direction) const override
        {
            return 0.5 * p[0]->value(direction) + 0.5 *p[1]->value(direction);
        }

        virtual vec3 generate() const override
        {
            if (random_value() < 0.5)
                return p[0]->generate();
            else
                return p[1]->generate();
        }

    public:
        shared_ptr<pdf> p[2];
};

#endif /* pdf_h */
