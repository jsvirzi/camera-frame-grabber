#ifndef PLOT_HPP
#define PLOT_HPP

class Plot {
public:
    double x_min, x_max, y_min, y_max;
    double x_min_plot, x_max_plot, y_min_plot, y_max_plot;
    double x_margin, y_margin;
    double x_target_window, y_target_window;
    double x_line_target_min, x_line_target_max;
    double y_line_target_min, y_line_target_max;
    int n_points;
    int n_axes, n_x, n_y;
    Plot();
    void register_point(double x, double y);
    void register_point(double x);
    void render();
};

#endif

