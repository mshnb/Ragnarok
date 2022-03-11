//
//  common.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/1.
//

#ifndef common_h
#define common_h

#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>

#define USE_FP32

#ifdef _MSC_VER
#define FN_NAME __FUNCTION__
#else
#define FN_NAME __func__
#endif

#define PRINT(print_type, ...) {printf("[%c] line %d in function %s: ", print_type, __LINE__, FN_NAME);printf(__VA_ARGS__);printf("\n");}

#define INFO(...) PRINT('I', __VA_ARGS__)
#define WARN(...) PRINT('W', __VA_ARGS__)

#define CHECK(status) if(status!=0){ERROR("check failed with err .\n", status);}

//parse xml
#define PARSE_VECTOR3(element, att_name, output) sscanf_s(element->FirstChildElement(att_name)->Attribute("value"), "%f,%f,%f", &output[0], &output[1], &output[2])

#ifdef USE_FP32
#define PARSE_FLOAT(element, att_name, output) sscanf_s(element->FirstChildElement(att_name)->Attribute("value"), "%f", &output)
#define PARSE_RADIANCE(element, output) sscanf_s(element->Attribute("radiance"), "%f,%f,%f", &output[0], &output[1], &output[2])
#else
#define PARSE_FLOAT(element, att_name, output) sscanf_s(element->FirstChildElement(att_name)->Attribute("value"), "%lf", &output)
#define PARSE_RADIANCE(element, output) sscanf_s(element->Attribute("radiance"), "%lf,%lf,%lf", &output[0], &output[1], &output[2])
#endif

#define PARSE_INT(element, att_name, output) sscanf_s(element->FirstChildElement(att_name)->Attribute("value"), "%d", &output)

// Usings
using std::shared_ptr;
using std::make_shared;
using std::sqrt;

#ifdef USE_FP32
using fType = float;
#else
using fType = double;
#endif

// Constants
const fType infinity = std::numeric_limits<fType>::infinity();
const fType PI = 3.1415926535897932385;
const fType INV_PI = 1 / PI;
const fType INV_TWOPI = INV_PI / 2;

//TODO set more epsilon for different use
const fType Epsilon = 1e-4;

const bool ensureEnergyConservation = true;

// Utility Functions
inline fType degrees_to_radians(fType degrees)
{
    return degrees * PI / 180.0;
}

inline fType clamp(fType x, fType min, fType max)
{
    if (x < min)
        return min;
    if (x > max)
        return max;
    return x;
}

inline int clamp(int x, int min, int max)
{
    if (x < min)
        return min;
    if (x > max)
        return max;
    return x;
}

//return a random real in [0,1)
inline fType random_value()
{
    return static_cast<fType>(rand() / (RAND_MAX + 1.0));
}

//return a random real in [min,max)
inline fType random_value(fType min, fType max)
{
    return min + (max-min) * random_value();
}

// Returns a random integer in [min,max]
inline int random_int(int min, int max)
{
    int ret = static_cast<int>(random_value(min, max+1));
#ifdef FP32
    return clamp(ret, min, max);
#else
    return ret;
#endif
}

// using the power heuristic
inline fType mix_weight(fType pdfA, fType pdfB) 
{
	pdfA *= pdfA;
	pdfB *= pdfB;
	return pdfA / (pdfA + pdfB);
}

#endif /* common_h */
