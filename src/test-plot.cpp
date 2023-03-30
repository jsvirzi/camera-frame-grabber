#include <TApplication.h>
#include <TGraph.h>
#include <TAxis.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TRootCanvas.h>
#include <TLatex.h>
#include <TText.h>
#include <TLine.h>
#include <TRandom3.h>

#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

//#include <stdlib.h>
//#include <stdio.h>
#include <unistd.h>

#include "plot.hpp"

extern "C" {
#include "udp.h"
}

/* calculates elapsed time (in milliseconds) between now and the initialization time (first call to this function) */
uint32_t elapsed_time(void)
{
    static const unsigned long long one_thousand = 1000ULL;
    static const unsigned long long one_million = 1000000ULL;
    static unsigned long long ts0_ms = 0;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    unsigned long long secs = (unsigned long long) ts.tv_sec;
    unsigned long long nano = (unsigned long long) ts.tv_nsec;
    unsigned long long ts_time = secs * one_thousand + nano / one_million;
    if (ts0_ms == 0) { ts0_ms = ts_time; }
    unsigned long long ts_diff = ts_time - ts0_ms;
    uint32_t diff_ms = ts_diff & 0xffffffff;
    return diff_ms;
}

class LooperInfo {
public:
    TText *g_text;
    int iter;
    int run;
    Plot *plot;
    LooperInfo(int n_points = 16);
    int udp_data_fd;
    int udp_data_port;
    int udp_ctrl_fd;
    int udp_ctrl_port;
    int wait_primitive;
    uint8_t udp_ip_addr[4];
    struct sockaddr_in server_data_addr;
    struct sockaddr_in server_ctrl_addr;
    protocol_packet_t outgoing_data_packet;
    protocol_packet_t incoming_data_packet;
    protocol_packet_t outgoing_ctrl_packet;
    protocol_packet_t incoming_ctrl_packet;
    uint32_t update_timeout;
    uint32_t update_period;
    float focus_measure;
    int focus_data_semaphore;
};

LooperInfo::LooperInfo(int n_points) {
    plot = new Plot(n_points);
    g_text = 0;
    iter = 0;
    run = 0;
    update_period = 250; /* 4 Hz */
    update_timeout = 0;
}

void delay_ms(unsigned int ms) {
    static const unsigned long long one_thousand = 1000ULL;
    struct timeval tv;
    tv.tv_sec = ms / one_thousand;
    tv.tv_usec = ms * one_thousand;
    select(0, NULL, NULL, NULL, &tv);
}

void *app_looper(void *arg)
{
    LooperInfo *looper_info = (LooperInfo *) arg;

    looper_info->udp_data_fd = socket(AF_INET, SOCK_DGRAM, 0);
    looper_info->udp_ctrl_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if ((looper_info->udp_data_fd < 0) || (looper_info->udp_ctrl_fd < 0)) {
        fprintf(stderr, "error opening socket\n");
        return 0;
    }

    memset(&looper_info->server_data_addr, 0, sizeof (looper_info->server_data_addr));
    looper_info->server_data_addr.sin_family = AF_INET;
    looper_info->server_data_addr.sin_port = htons(looper_info->udp_data_port);
    looper_info->server_data_addr.sin_addr.s_addr = INADDR_ANY;

    memset(&looper_info->server_ctrl_addr, 0, sizeof (looper_info->server_ctrl_addr));
    looper_info->server_ctrl_addr.sin_family = AF_INET;
    looper_info->server_ctrl_addr.sin_port = htons(looper_info->udp_ctrl_port);
    looper_info->server_ctrl_addr.sin_addr.s_addr = INADDR_ANY;

    /* we are server for control port */
    if (bind(looper_info->udp_ctrl_fd, (struct sockaddr *) &looper_info->server_ctrl_addr, sizeof(looper_info->server_ctrl_addr)) < 0) {
        fprintf(stderr, "bind: error");
        return 0;
    }

    while (looper_info->run) {

        socklen_t len;
        int status;

        uint32_t now = elapsed_time();
        if (now > looper_info->update_timeout && (looper_info->wait_primitive)) {
            printf("iter = %d\n", ++looper_info->iter);
            looper_info->g_text = new TText(0.0, 0.0, "hello");
            looper_info->g_text->Draw();
            looper_info->update_timeout = now + looper_info->update_period;
            looper_info->wait_primitive = 0;
        }

        protocol_packet_header_t *header = &looper_info->outgoing_data_packet.header;
        header->sync_word = SYNC_WORD;
        header->length = 0;
        header->type = PACKET_TYPE_REQUEST_FOCUS;
        sendto(looper_info->udp_data_fd, (const char *) &looper_info->outgoing_data_packet, sizeof (protocol_packet_header_t), 0,
            (const struct sockaddr *) &looper_info->server_data_addr, sizeof(looper_info->server_data_addr));

        len = 0;
        status = check_socket(looper_info->udp_data_fd);
        if (status) {
            ssize_t n = recvfrom(looper_info->udp_data_fd, (char *) &looper_info->incoming_data_packet, sizeof (looper_info->incoming_data_packet),0,
                (struct sockaddr *) &looper_info->server_data_addr, &len);
            if (n > sizeof (protocol_packet_header_t)) {
                header = &looper_info->incoming_data_packet.header;
                if ((header->type == PACKET_TYPE_RESPOND_FOCUS) && (header->length == sizeof (float))) {
                    float *focus = (float *) looper_info->incoming_data_packet.payload;
                    looper_info->focus_measure = *focus;
                    looper_info->plot->register_point(looper_info->focus_measure);
                    if (looper_info->focus_data_semaphore == 0) { looper_info->focus_data_semaphore = 1; }
                    printf("debug: response data(%d) = %f\n", looper_info->plot->n_data, looper_info->focus_measure);
                }
            }
        }

        len = 0;
        status = check_socket(looper_info->udp_ctrl_fd);
        if (status > 0) {
            ssize_t n = recvfrom(looper_info->udp_ctrl_fd, (char *) &looper_info->incoming_ctrl_packet, sizeof (looper_info->incoming_ctrl_packet),0,
                (struct sockaddr *) &looper_info->server_ctrl_addr, &len);
            looper_info->plot->reset();
            if (n > sizeof (protocol_packet_header_t)) {
                header = &looper_info->incoming_ctrl_packet.header;
                printf("we got'er done %zd\n", n);
                if ((header->type == PACKET_TYPE_RESPOND_FOCUS) && (header->length == sizeof (float))) {
                }
            }
        }

    }

    return 0;
}

void *img_looper(void *arg)
{
    LooperInfo *looper_info = (LooperInfo *) arg;
    TRandom3 *rndm = new TRandom3(0);
    while (looper_info->run) {
        sleep(1);
    }

    return 0;
}

int main(int argc, char **argv)
{

    int n_x = 16;
    elapsed_time(); /* initializes t = 0 */
    LooperInfo looper_info(n_x);
    looper_info.udp_data_port = DEFAULT_UDP_PORT;
    looper_info.udp_ctrl_port = looper_info.udp_data_port + 1;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-p") == 0) {
            looper_info.udp_data_port = atoi(argv[++i]);
            looper_info.udp_ctrl_port = looper_info.udp_data_port + 1;
        }
    }

    printf("using port %d for udp communications\n", looper_info.udp_data_port);

    TApplication app("app", &argc, argv);
    TCanvas* c = new TCanvas("c", "focus-analysis", 0, 0, 800, 600);

    double *graph_x = looper_info.plot->get_x();
    double *graph_y = looper_info.plot->get_y();
    double latest_x[1];
    double latest_y[1];

    pthread_t tid_app;
    pthread_t tid_img;

    looper_info.run = 1;
    pthread_create(&tid_app, NULL, app_looper, &looper_info);
    pthread_create(&tid_img, NULL, img_looper, &looper_info);

    double x_axis_min = 0.0;
    double x_axis_max = n_x + 1;

    TGraph *g_history = new TGraph(n_x); // , graph_x, graph_y);
    g_history->SetName("g_history");
    g_history->SetMarkerColor(kBlue);
    g_history->SetMarkerStyle(kOpenCircle);
    g_history->GetXaxis()->SetLimits(x_axis_min, x_axis_max);

    latest_x[0] = n_x;
    latest_y[0] = 0.0;
    TGraph *g_newest = new TGraph(1, latest_x, latest_y);
    g_newest->SetMarkerStyle(kFullCircle);

    int i = 1;
    while (looper_info.run)
    {
        int color = ((i % 2) == 0) ? kBlue : kRed;
        g_history->SetMarkerColor(color);
        g_newest->SetMarkerColor(color);

        latest_y[0] = looper_info.plot->y_new;
        int n_pts = g_newest->GetN();
        g_newest->SetPoint(n_pts - 1, n_x, latest_y[0]);

        double *y = looper_info.plot->get_y();
        for (int i = 0; i < n_x; ++i) {
            g_history->SetPoint(i, (double) i + 1, y[i]);
        }

        looper_info.plot->render(); /* analyze data */

        double y_line_min = looper_info.plot->y_line_target_min;
        double y_line_max = looper_info.plot->y_line_target_max;

        double y_axis_min = looper_info.plot->y_min_plot;
        double y_axis_max = looper_info.plot->y_max_plot;

        g_history->SetMinimum(0.0);
        g_history->SetMaximum(y_axis_max);

        printf("debug: x axis = (%f, %f). lines at (%f, %f)\n", x_axis_min, x_axis_max, y_line_min, y_line_max);

        g_history->GetXaxis()->SetLimits(x_axis_min, x_axis_max);

        g_history->Draw("ap");
        g_newest->Draw("p");

        TLine line_ymin(x_axis_min, y_line_min, x_axis_max, y_line_min);
        TLine line_ymax(x_axis_min, y_line_max, x_axis_max, y_line_max);
        line_ymin.SetLineColor(kRed);
        line_ymin.SetLineStyle(kDotted);
        line_ymin.SetLineWidth(3.0);
        line_ymax.SetLineColor(kRed);
        line_ymax.SetLineStyle(kDotted);
        line_ymax.SetLineWidth(3.0);
        line_ymin.Draw();
        line_ymax.Draw();

        c->Modified();
        c->Update();
        c->Draw();

        if (looper_info.wait_primitive == 0) { looper_info.wait_primitive = 1; }
        c->WaitPrimitive();
        ++i;
        looper_info.iter = 0;
    }

    pthread_join(tid_app, 0);
    pthread_join(tid_img, 0);

    return 0;
}
