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
    if (p_x) { delete [] p_x; }
    if (p_y) { delete [] p_y; }
    p_x = new double [ n_points ];
    p_y = new double [ n_points ];
    for (int i = 0; i < n_points; ++i) {
        p_x[i] = 0.0;
        p_y[i] = 0.0;
    }
}

void Plot::reset() {
    setup(n_points);
}

Plot::Plot() {
    p_x = 0;
    p_y = 0;
    setup(16);
}

Plot::Plot(int n) {
    p_x = 0;
    p_y = 0;
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

void Plot::register_point(double y)
{
    n_axes = 1;
    y_new = y;
    int idx = n_data % n_points;
    p_y[idx] = y;
    ++n_data;
    if (y < y_min) { y_min = y; }
    if (y > y_max) { y_max = y; }
}

void Plot::render()
{
    if (n_axes == 1) {
        y_min = 0.0;
        double y_interval = (y_max - y_min);
        double y_delta = y_interval * y_margin;
        y_min_plot = y_min - y_delta;
        y_max_plot = y_max + y_delta;
        y_delta = y_interval * x_target_window;
        y_line_target_min = y_max - y_delta;
        y_line_target_max = y_max + y_delta;
    }
}
