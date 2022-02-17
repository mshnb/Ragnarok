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

#ifdef _MSC_VER
#define FN_NAME __FUNCTION__
#else
#define FN_NAME __func__
#endif

#define PRINT(print_type, ...) printf("[%c] line %d in function %s: ", print_type, __LINE__, FN_NAME);printf(__VA_ARGS__);printf("\n")

#define INFO(...) PRINT('I', __VA_ARGS__)
#define WARN(...) PRINT('W', __VA_ARGS__)

#define USE_FP32
#define OUTPUT_EXR

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
const fType pi = 3.1415926535897932385;

// Utility Functions
inline fType degrees_to_radians(fType degrees)
{
    return degrees * pi / 180.0;
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

#endif /* common_h */
