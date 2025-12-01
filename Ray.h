#ifndef RAY_H
#define RAY_H


#include "Vector3.h"


struct Ray {
    Vector3 origin;
    Vector3 dir; // should be normalized
    Ray() {}
    Ray(const Vector3 &o, const Vector3 &d): origin(o), dir(d) {}
};


#endif // RAY_H