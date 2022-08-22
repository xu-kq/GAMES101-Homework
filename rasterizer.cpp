// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


static bool insideTriangle(float x, float y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    Eigen::Vector3f p(x, y, 1);
    Vector3f vert[3];
    for (int i = 0; i < 3; ++i) {
        vert[i].x() = _v[i].x();
        vert[i].y() = _v[i].y();
        vert[i].z() = 1;
    }
    Eigen::Vector3f f01, f12, f20;
    f01 = vert[0].cross(vert[1]);
    f12 = vert[1].cross(vert[2]);
    f20 = vert[2].cross(vert[0]);

    if(p.dot(f01) * vert[2].dot(f01) > 0 && p.dot(f12) * vert[0].dot(f12) > 0 && p.dot(f20) * vert[1].dot(f20) > 0) {
        return true;
    }
    return false;
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();

    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle
    float xmin = v[0].x(),
        xmax = v[0].x(),
        ymin = v[0].y(),
        ymax = v[0].y();
    for (int i = 1; i < 3; ++i) {
        xmin = std::min(xmin, v[i].x());
        xmax = std::max(xmax, v[i].x());
        ymin = std::min(ymin, v[i].y());
        ymax = std::max(ymax, v[i].y());
    }

    // If so, use the following code to get the interpolated z value.
    //auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
    //float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
    //float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
    //z_interpolated *= w_reciprocal;

    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.

    float xincr, yincr;
    xincr = yincr = 1.f / N;
    float half_xincr, half_yincr;
    half_xincr = half_yincr = xincr / 2;
    int xlow = std::max(0, int(std::round(xmin))),
        ylow = std::max(0, int(std::round(ymin))),
        xupp = std::min(width, int(std::round(xmax))),
        yupp = std::min(height, int(std::round(ymax)));
    float x, y;
    for (int j = ylow; j < yupp; ++j) {
        for (int i = xmin; i < xupp; ++i) {
            Eigen::Vector3f color(0, 0, 0);
            for(int ni = 0; ni < N; ++ni) {
                for (int nj = 0; nj < N; ++nj) {
                    x = i + half_xincr + xincr * ni;
                    y = j + half_yincr + yincr * nj;
                    if (insideTriangle(x, y, t.v)) {
                        auto [alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
                        float w_reciprocal = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                        float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                        z_interpolated *= w_reciprocal;
                        if (z_interpolated > ss_depth_buf[get_ss_index(i, j, ni, nj)]) {
                            ss_depth_buf[get_ss_index(i, j, ni, nj)] = z_interpolated;
                            ss_frame_buf[get_ss_index(i, j, ni, nj)] = t.getColor();
                        }
                    }
                    color += ss_frame_buf[get_ss_index(i, j, ni, nj)];
                }
            }
            set_pixel({ float(i), float(j), 0 }, color /= N * N);
            //float percent = 1.f * cnt / N / N;
            //set_pixel({ float(i), float(j), 0 }, t.getColor() * percent + frame_buf[get_index(i, j)] * (1-percent));
        }
    }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
        std::fill(ss_frame_buf.begin(), ss_frame_buf.end(), Eigen::Vector3f{ 0, 0, 0 });
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        //std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::lowest());
        std::fill(ss_depth_buf.begin(), ss_depth_buf.end(), std::numeric_limits<float>::lowest());
    }
}

rst::rasterizer::rasterizer(int w, int h, int n = 1) : width(w), height(h), N(n)
{
    frame_buf.resize(w * h);
    ss_frame_buf.resize(w * h * N * N);

    //depth_buf.resize(w * h);
    ss_depth_buf.resize(w * h * N * N);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

int rst::rasterizer::get_ss_index(int x, int y, int ni, int nj)
{
    return ((height - 1 - y) * width + x) * N * N + ni * N + nj;

}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

// clang-format on