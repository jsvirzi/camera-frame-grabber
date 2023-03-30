#include "plot.hpp"

void Plot::setup(int n) {
    n_data = 0;
    n_points = n;
    x_min =  999999999.0;
    x_max = -999999999.0;
    y_min =  999999999.0;
    y_max = -999999999.0;
    n_axes = 0;
    x_margin = 0.2;
    y_margin = 0.2;
    x_target_window = 0.1;
    y_target_window = 0.1;
    p_x = new double [ n_points ];
    p_y = new double [ n_points ];
    for (int i = 0; i < n_points; ++i) {
        p_x[i] = 0.0;
        p_y[i] = 0.0;
    }
}

Plot::Plot() {
    setup(16);
}

Plot::Plot(int n) {
    setup(n);
}

void Plot::register_point(double x, double y)
{
    n_axes = 2;
    x_new = x;
    y_new = y;
    p_x[(n_data % n_points)] = x;
    p_y[(n_data % n_points)] = y;
    ++n_data;
    if (x < x_min) { x_min = x; }
    if (x > x_max) { x_max = x; }
    if (y < y_min) { y_min = y; }
    if (y > y_max) { y_max = y; }
}

void Plot::register_point(double x)
{
    n_axes = 1;
    x_new = x;
    int idx = n_data % n_points;
    p_x[idx] = x;
    ++n_data;
    if (x < x_min) { x_min = x; }
    if (x > x_max) { x_max = x; }
}

void Plot::render()
{
    if (n_axes == 1) {
        x_min = 0.0;
        double x_interval = (x_max - x_min);
        double x_delta = x_interval * x_margin;
        x_min_plot = x_min - x_delta;
        x_max_plot = x_max + x_delta;
        x_delta = x_interval * x_target_window;
        x_line_target_min = x_max - x_delta;
        x_line_target_max = x_max + x_delta;
    }
}
