#include "plot.hpp"

Plot::Plot() {
    x_min =  999999999.0;
    x_max = -999999999.0;
    y_min =  999999999.0;
    y_max = -999999999.0;
    n_points = 0;
    n_x = 100;
    n_y = 100;
    n_axes = 0;
    x_margin = 0.1;
    y_margin = 0.1;
    x_target_window = 0.5;
    y_target_window = 0.5;
}

void Plot::register_point(double x, double y)
{
    n_axes = 2;
    if (x < x_min) { x_min = x; }
    if (x > x_max) { x_max = x; }
    if (y < y_min) { y_min = y; }
    if (y > y_max) { y_max = y; }
}

void Plot::register_point(double x)
{
    n_axes = 1;
    if (x < x_min) { x_min = x; }
    if (x > x_max) { x_max = x; }
}

void Plot::render()
{
    if (n_axes == 1) {
        double x_interval = (x_max - x_min);
        double x_delta = x_interval * x_margin;
        x_min_plot = x_min - x_delta;
        x_max_plot = x_max + x_delta;
        x_delta = x_interval * x_target_window;
        x_line_target_min = x_min - x_delta;
        x_line_target_max = x_max + x_delta;
    }
}
