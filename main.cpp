#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <memory>
#include <cmath>
#include <omp.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


#include "Vector3.h"
#include "Ray.h"
#include "Scene.h"

using namespace std;

Vector3 clampVec(const Vector3 &v) {
    auto c = [](double x){ if (x<0) return 0.0; if (x>1) return 1.0; return x; };
    return Vector3(c(v.x), c(v.y), c(v.z));
}

// Phong shading with shadows
Vector3 shade(const Scene &scene, const HitInfo &hit) {
    Vector3 color = scene.ambient * hit.mat.color * hit.mat.kd; // ambient term

    for (const auto &light : scene.lights) {
        Vector3 L = (light.position - hit.position).normalized();
        // Shadow ray
        Ray shadowRay(hit.position + hit.normal * 1e-4, L);
        HitInfo shadowHit = scene.trace(shadowRay, 1e-4);
        bool inShadow = shadowHit.hit;
        if (!inShadow) {
            // Diffuse
            double NdotL = max(0.0, hit.normal.dot(L));
            Vector3 diffuse = hit.mat.color * hit.mat.kd * NdotL;
            // Specular
            Vector3 V = (Vector3(0,0,0) - hit.position).normalized(); // camera at origin in view space later will adapt
            // For correct V we pass proper eye location later; here we'll compute actual in caller if needed
            Vector3 R = (hit.normal * 2.0 * hit.normal.dot(L) - L).normalized();
            double RdotV = max(0.0, R.dot(V));
            Vector3 spec = light.intensity * hit.mat.ks * pow(RdotV, hit.mat.shininess);
            color += (diffuse + spec) * light.intensity;
        }
    }
    return clampVec(color);
}

// A helper that computes shading with explicit eye position
Vector3 computeColor(const Scene &scene, const Ray &ray, const HitInfo &hit, const Vector3 &eye) {
    Vector3 color = scene.ambient * hit.mat.color * hit.mat.kd;
    for (const auto &light : scene.lights) {
        Vector3 L = (light.position - hit.position).normalized();
        Ray shadowRay(hit.position + hit.normal * 1e-4, L);
        HitInfo s = scene.trace(shadowRay, 1e-4);
        if (s.hit) continue;
        double NdotL = max(0.0, hit.normal.dot(L));
        Vector3 diffuse = hit.mat.color * hit.mat.kd * NdotL;
        Vector3 V = (eye - hit.position).normalized();
        Vector3 R = (hit.normal * 2.0 * hit.normal.dot(L) - L).normalized();
        double RdotV = max(0.0, R.dot(V));
        Vector3 spec = light.intensity * hit.mat.ks * pow(RdotV, hit.mat.shininess);
        color += (diffuse + spec) * light.intensity;
    }
    return clampVec(color);
}

int main(int argc, char** argv) {
    // Parameters
    int width = 1024;
    int height = 768;
    if (argc >= 3) {
        width = atoi(argv[1]); height = atoi(argv[2]);
    }

    vector<int> threadCounts = {1,2,4,8};
    if (argc >= 4) {
        threadCounts.clear();
        for (int i=3;i<argc;i++) threadCounts.push_back(atoi(argv[i]));
    }

    // Build scene (hardcoded)
    Scene scene;
    Material red; red.color = Vector3(0.8,0.1,0.1); red.kd=0.7; red.ks=0.3; red.shininess=64;
    Material green; green.color = Vector3(0.1,0.8,0.1); green.kd=0.7; green.ks=0.3; green.shininess=32;
    Material blue; blue.color = Vector3(0.1,0.1,0.8); blue.kd=0.7; blue.ks=0.3; blue.shininess=16;
    Material gray; gray.color = Vector3(0.7,0.7,0.7); gray.kd=0.8; gray.ks=0.2; gray.shininess=8;

    scene.spheres.push_back(Sphere(Vector3(-1.2, 0.5, -6.0), 1.0, red));
    scene.spheres.push_back(Sphere(Vector3(1.2, 0.2, -5.0), 0.8, blue));
    scene.spheres.push_back(Sphere(Vector3(0.0, -0.5, -4.0), 0.6, green));
    scene.planes.push_back(Plane(Vector3(0,-1.5,0), Vector3(0,1,0), gray));

    scene.lights.push_back(PointLight(Vector3(5, 7, -3), Vector3(1.0,1.0,1.0)));
    scene.lights.push_back(PointLight(Vector3(-4, 4, -2), Vector3(0.4,0.4,0.4)));

    // Camera setup (pinhole)
    Vector3 eye(0, 0, 0);
    Vector3 lookAt(0, 0, -1);
    Vector3 up(0,1,0);
    double fov = 60.0; // degrees
    double aspect = double(width)/double(height);
    double scale = tan((fov * 0.5) * M_PI/180.0);

    // Results container (colors as Vector3 in [0..1])

    for (int th : threadCounts) {
        // Set threads
        omp_set_num_threads(th);

        vector<Vector3> framebuffer(width * height);

        auto t0 = chrono::high_resolution_clock::now();

        #pragma omp parallel for schedule(dynamic, 16)
        for (int j = 0; j < height; ++j) {
            for (int i = 0; i < width; ++i) {
                double x = (2 * ((i + 0.5) / (double)width) - 1) * aspect * scale;
                double y = (1 - 2 * ((j + 0.5) / (double)height)) * scale;
                Vector3 dir = Vector3(x, y, -1).normalized();
                Ray ray(eye, dir);
                HitInfo hit = scene.trace(ray);
                Vector3 col(0,0,0);
                if (hit.hit) {
                    col = computeColor(scene, ray, hit, eye);
                } else {
                    // background gradient
                    double t = 0.5 * (dir.y + 1.0);
                    col = Vector3(0.6,0.7,0.9)*(1.0 - t) + Vector3(0.1,0.1,0.15)*t;
                }
                framebuffer[j*width + i] = col;
            }
        }

        auto t1 = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = t1 - t0;
        cout << "Threads=" << th << " Time(s)=" << elapsed.count() << endl;

        // Save to PPM
        char filename[256];
        snprintf(filename, sizeof(filename), "img_output/output_parallel_%d.ppm", th);
        ofstream ofs(filename);
        ofs << "P3\n" << width << " " << height << "\n255\n";
        for (int k = 0; k < width*height; ++k) {
            Vector3 c = framebuffer[k];
            int r = (int)(255.0 * max(0.0, min(1.0, c.x)));
            int g = (int)(255.0 * max(0.0, min(1.0, c.y)));
            int b = (int)(255.0 * max(0.0, min(1.0, c.z)));
            ofs << r << " " << g << " " << b << ( (k%width==width-1) ? '\n' : ' ' );
        }
        ofs.close();
    }

    return 0;
}
