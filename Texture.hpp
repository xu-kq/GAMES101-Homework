//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture {
private:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        if (u < 0 || u >= 1 || v < 0 || v >= 1) {
            return { 0, 0, 0 };
        }
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

    Eigen::Vector3f getColorBilinear(float u, float v) {

        float s, t;
        int x0, x1, y0, y1;
        x0 = int(u * (width - 1)), x1 = x0 + 1;
        y0 = int((1 - v) * (height - 1)), y1 = y0 + 1;
        
        if (x0 < 0 || x1 >= width || y0 < 0 || y1 >= height) {
            return { 0, 0, 0 };
        }

        s = u * (width - 1) - x0, t = (1-v) * (height - 1) - y0;

        Eigen::Vector3f u00 = to_vec3(image_data.at<cv::Vec3b>(y0, x0));
        Eigen::Vector3f u01 = to_vec3(image_data.at<cv::Vec3b>(y1, x0));
        Eigen::Vector3f u10 = to_vec3(image_data.at<cv::Vec3b>(y0, x1));
        Eigen::Vector3f u11 = to_vec3(image_data.at<cv::Vec3b>(y1, x1));

        

        auto u0 = lerp(s, u00, u10),
            u1 = lerp(s, u01, u11);
        auto f = lerp(t, u0, u1);

        return f;
    }
    Eigen::Vector3f to_vec3(cv::Vec3b image_data) {
        return Eigen::Vector3f(  image_data[0], image_data[1], image_data[2] );
    }


    template<typename T>
    T lerp(float x, T v0, T v1) {
        return v0 + x * (v1 - v0);
    }
};
#endif //RASTERIZER_TEXTURE_H
