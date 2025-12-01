#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <limits>
#include "Vector3.h"
#include "Ray.h"

const double EPS = 1e-6;

struct Material {
    Vector3 color;
    double kd;
    double ks;
    double shininess;

    Material() : color(1,1,1), kd(0.8), ks(0.2), shininess(32) {}
};

struct HitInfo {
    bool hit;
    double t;
    Vector3 position;
    Vector3 normal;
    Material mat;
};

struct Sphere {
    Vector3 center;
    double radius;
    Material mat;

    Sphere(const Vector3 &c, double r, const Material &m)
            : center(c), radius(r), mat(m) {}

    bool intersect(const Ray &ray, double &tOut) const {
        Vector3 oc = ray.origin - center;
        double a = ray.dir.dot(ray.dir);
        double b = 2.0 * oc.dot(ray.dir);
        double c = oc.dot(oc) - radius * radius;
        double disc = b*b - 4*a*c;

        if (disc < 0) return false;

        double sqrtD = sqrt(disc);
        double t1 = (-b - sqrtD) / (2*a);
        double t2 = (-b + sqrtD) / (2*a);
        double t = (t1 > EPS) ? t1 : (t2 > EPS ? t2 : -1);

        if (t > EPS) {
            tOut = t;
            return true;
        }
        return false;
    }
};

struct Plane {
    Vector3 point;
    Vector3 normal;
    Material mat;

    Plane(const Vector3 &p, const Vector3 &n, const Material &m)
            : point(p), normal(n.normalized()), mat(m) {}

    bool intersect(const Ray &ray, double &tOut) const {
        double denom = normal.dot(ray.dir);
        if (fabs(denom) < EPS) return false;

        double t = (point - ray.origin).dot(normal) / denom;

        if (t > EPS) {
            tOut = t;
            return true;
        }
        return false;
    }
}; // <--- ESTA LLAVE FALTABA EN TU ERROR

struct PointLight {
    Vector3 position;
    Vector3 intensity;

    PointLight(const Vector3 &p, const Vector3 &i)
            : position(p), intensity(i) {}
};

struct Scene {
    std::vector<Sphere> spheres;
    std::vector<Plane> planes;
    std::vector<PointLight> lights;
    Vector3 ambient;

    Scene() : ambient(0.1,0.1,0.1) {}

    HitInfo trace(const Ray &ray, double tMin=1e-6, double tMax=1e12) const {
        HitInfo info;
        info.hit = false;
        info.t = tMax;

        for (const auto &s : spheres) {
            double t;
            if (s.intersect(ray, t) && t < info.t && t > tMin) {
                info.hit = true;
                info.t = t;
                info.position = ray.origin + ray.dir * t;
                info.normal = (info.position - s.center).normalized();
                info.mat = s.mat;
            }
        }

        for (const auto &p : planes) {
            double t;
            if (p.intersect(ray, t) && t < info.t && t > tMin) {
                info.hit = true;
                info.t = t;
                info.position = ray.origin + ray.dir * t;
                info.normal = p.normal;
                info.mat = p.mat;
            }
        }

        return info;
    }
};

#endif // SCENE_H
