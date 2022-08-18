//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    
    // ray-obj intersection check 
    Intersection inter;
    inter = intersect(ray);
    if(!inter.happened) {
        return {};
    }
    if(inter.obj->hasEmit()){
        // return {};
        return inter.m->getEmission();
    }
    // direct illumination
    Intersection pos;
    float pdf;
    sampleLight(pos, pdf);

    Vector3f ori, dir;
    ori = inter.coords;
    dir = (pos.coords - ori).normalized();
    Ray dir_ray(ori, dir);
    
    Vector3f L_dir;
    float eps = 1e-3;
    if((intersect(dir_ray).coords - pos.coords).norm() < eps) {
        Vector3f L_i, f_r;
        Vector3f wi, wo, N, N_x;
    
        float cos_theta, cos_theta_x;
        L_i = pos.emit;
        N = inter.normal;
        N_x = pos.normal;
        
        wi = dir_ray.direction;
        wo = -ray.direction;
        f_r = inter.m->eval(wi, wo, N);
        
        cos_theta = std::max(0.f, dotProduct(N, wi));
        cos_theta_x = std::max(0.f, dotProduct(N_x, -wi));
        
        float dis_inv = 1.f / (pos.coords - ori).norm();

        L_dir = L_i * f_r * cos_theta * cos_theta_x * dis_inv * dis_inv / pdf;
    }

    Vector3f L_indir;

    if(get_random_float() < RussianRoulette) {
        Vector3f L_i, f_r;
        Vector3f wi, wo, N;

        float cos_theta;

        N = inter.normal;        
        wo = -ray.direction;
        wi = (inter.m->sample(wo, N)).normalized();

        f_r =  inter.m->eval(wi, wo, N);
        cos_theta = std::max(0.f, dotProduct(N, wi));

        Ray indir_ray(inter.coords, wi);
        inter = intersect(indir_ray);
        if(inter.happened && !inter.obj->hasEmit()) {
            L_indir = castRay(indir_ray, depth + 1) * f_r * cos_theta  / inter.m->pdf(wi,wo,N) / RussianRoulette;
        }
    }
    return L_dir + L_indir;
}