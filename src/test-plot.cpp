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


class LooperInfo {
public:
    TText *g_text;
    int iter;
    int run;
    Plot *plot;
    LooperInfo(int n_points = 16);
    int udp_fd;
    int udp_port;
    uint8_t udp_ip_addr[4];
    struct sockaddr_in server_addr;
    protocol_packet_t outgoing_packet;
    protocol_packet_t incoming_packet;
};

LooperInfo::LooperInfo(int n_points) {
    plot = new Plot(n_points);
    g_text = 0;
    iter = 0;
    run = 0;
}

void *app_looper(void *arg)
{
    LooperInfo *looper_info = (LooperInfo *) arg;

    looper_info->udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (looper_info->udp_fd < 0) {
        fprintf(stderr, "error opening socket\n");
        return 0;
    }

    looper_info->server_addr.sin_family = AF_INET;
    looper_info->server_addr.sin_port = htons(looper_info->udp_port);
    looper_info->server_addr.sin_addr.s_addr = INADDR_ANY;

    while (looper_info->run) {

        sleep(1);
        printf("iter = %d\n", ++looper_info->iter);
        if (looper_info->iter >= 1) {
            looper_info->g_text = new TText(0.0, 0.0, "hello");
            looper_info->g_text->Draw();
        }

        protocol_packet_header_t *header = &looper_info->outgoing_packet.header;
        header->sync_word = SYNC_WORD;
        header->length = 0;
        header->type = PACKET_TYPE_REQUEST_FOCUS;
        sendto(looper_info->udp_fd, (const char *) &looper_info->outgoing_packet, sizeof (protocol_packet_header_t), 0,
            (const struct sockaddr *) &looper_info->server_addr, sizeof(looper_info->server_addr));

        socklen_t len = 0;
        int status = check_socket(looper_info->udp_fd);
        if (status) {
            ssize_t n = recvfrom(looper_info->udp_fd, (char *) &looper_info->incoming_packet, sizeof (looper_info->incoming_packet),0,
                (struct sockaddr *) &looper_info->server_addr, &len);
            printf("they said it would not be done %d\n", n);
            if (n > sizeof (protocol_packet_header_t)) {
                header = &looper_info->incoming_packet.header;
                if (header->type == PACKET_TYPE_RESPOND_FOCUS && header->length == sizeof (float)) {
                    float *focus = (float *) looper_info->incoming_packet.payload;
                    printf("response = %f\n", *focus);
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
        double x = rndm->Uniform(220.0, 250.0);
        looper_info->plot->register_point(x);
        printf("new data(%d) = %f\n", looper_info->plot->n_data, x);
    }

    return 0;
}

int main(int argc, char **argv)
{
    int n_x = 16;
    LooperInfo looper_info(n_x);
    looper_info.udp_port = 55153; /* TODO */
    TApplication app("app", &argc, argv);
    TCanvas* c = new TCanvas("c", "Something", 0, 0, 800, 600);

    double *graph_x = looper_info.plot->get_x();
    double *graph_y = looper_info.plot->get_y();
    double latest_x[1];
    double latest_y[1];

    pthread_t tid_app;
    pthread_t tid_img;

    looper_info.run = 1;
    pthread_create(&tid_app, NULL, app_looper, &looper_info);
    pthread_create(&tid_img, NULL, img_looper, &looper_info);

//    double x_axis_min = 200.0;
//    double x_axis_max = 260.0;
    double y_axis_min = 0.0;
    double y_axis_max = 1.0;

    TGraph *g_history = new TGraph(n_x, graph_x, graph_y);
    g_history->SetName("g_history");
    g_history->SetMarkerColor(kBlue);
    g_history->SetMarkerStyle(kOpenCircle);
    g_history->SetMinimum(y_axis_min);
    g_history->SetMaximum(y_axis_max);
//    g_history->GetXaxis()->SetLimits(x_axis_min, x_axis_max);

    latest_y[0] = 0.0;
    TGraph *g_newest = new TGraph(1, latest_x, latest_y);
    g_newest->SetMarkerStyle(kFullCircle);

    int i = 1;
    while (1)
    {

        if ((i % 2) == 0) {
            g_history->SetMarkerColor(kBlue);
            g_newest->SetMarkerColor(kBlue);
        } else {
            g_history->SetMarkerColor(kRed);
            g_newest->SetMarkerColor(kRed);
        }

        latest_x[0] = looper_info.plot->x_new;
        g_newest->SetPoint(0, latest_x[0], latest_y[0]);

        for (int i = 0; i < n_x; ++i) {
            g_history->SetPoint(i, graph_x[i], graph_y[i]);
        }

        looper_info.plot->render();

        double x_axis_min = looper_info.plot->x_min_plot;
        double x_axis_max = looper_info.plot->x_max_plot;
        g_history->GetXaxis()->SetLimits(x_axis_min, x_axis_max);

        g_history->Draw("ap");
        g_newest->Draw("p");

        double x_line_min = looper_info.plot->x_line_target_min;
        double x_line_max = looper_info.plot->x_line_target_max;
        TLine line_xmin(x_line_min, y_axis_min, x_line_min, y_axis_max);
        TLine line_xmax(x_line_max, y_axis_min, x_line_max, y_axis_max);
        line_xmin.SetLineColor(kRed);
        line_xmin.SetLineStyle(kDotted);
        line_xmin.SetLineWidth(3.0);
        line_xmax.SetLineColor(kRed);
        line_xmax.SetLineStyle(kDotted);
        line_xmax.SetLineWidth(3.0);
        line_xmin.Draw();
        line_xmax.Draw();

        c->Modified();
        c->Update();
        c->Draw();

        c->WaitPrimitive();
        ++i;
        looper_info.iter = 0;
    }

    TRootCanvas *rc = (TRootCanvas *) c->GetCanvasImp();
    rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
    app.Run();

#if 0
    TCanvas* c = new TCanvas("c", "Something", 0, 0, 800, 600);
    TF1 *f1 = new TF1("f1","sin(x)", -5, 5);
    f1->SetLineColor(kBlue+1);
    f1->SetTitle("My g_history;x; sin(x)");
    f1->Draw();
    c->Print("demo1.pdf");
#endif

    return 0;
}
