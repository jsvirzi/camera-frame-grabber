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
#include <TH1I.h>

#include <pthread.h>

extern "C" {

    /* TODO this has multiple definitions */
    static int delay_us(unsigned int microseconds)
    {
        struct timeval tv;
        tv.tv_sec = microseconds / 1000000ULL;
        tv.tv_usec = microseconds - 1000000ULL * tv.tv_sec;
        select(1, NULL, NULL, NULL, &tv);
        return 0;
    }

    #include "graph-focus.h"

    typedef struct {
        TH1D *focus_hist;
        TH1D *focus_hist_max;
        double *focus;
        double *focus_max;
        int n_focus;
        TApplication *app;
        TCanvas *canvas;
        pthread_t thread_id;
        int run;
        pthread_t thread_id_app;
        int new_data_semaphore;
        unsigned int graph_looper_pace;
    } FocusGraphInfo;

    static FocusGraphInfo focus_graph_info;

    void *graph_focus_app_looper(void *arg)
    {
        FocusGraphInfo *info = (FocusGraphInfo *) arg;
        info->app->Run();
    }

    void *graph_focus_looper(void *arg)
    {
        FocusGraphInfo *info = (FocusGraphInfo *) arg;
        while (info->run) {

            delay_us(info->graph_looper_pace);
            if (info->new_data_semaphore == 0) { continue; }

            TCanvas *c = info->canvas;

            c->cd();

            info->focus_hist_max->Draw();
            info->focus_hist_max->Draw("same p");
            info->focus_hist->Draw("same p");

            c->Modified();
            c->Update();
            c->Draw();

            info->new_data_semaphore = 0;
        }
    }

    int initialize_focus_graph(int n_focus)
    {
        FocusGraphInfo *info = &focus_graph_info;
        memset(&focus_graph_info, 0, sizeof (focus_graph_info));

        info->graph_looper_pace = 100; /* 100us looper pace */

        info->app = new TApplication("app", NULL, NULL);

        info->canvas = new TCanvas("c", "focus-analysis", 0, 0, 800, 600);

        info->focus_hist = new TH1D("focus-hist", "FOCUS", n_focus, -0.5, n_focus + 0.5);
        info->focus_hist_max = new TH1D("focus-hist-max", "FOCUS", n_focus, -0.5, n_focus + 0.5);

        TH1D *h = info->focus_hist_max;

        h->SetMarkerColor(kBlue);
        h->SetMarkerStyle(kOpenCircle);
        h->SetFillColor(kYellow);
        // h->SetFillStyle(kFillSolid);
        // h->SetMaximum(100.0); /* token maximum. reset later with real data */
        // h->FillRandom("gaus", 20000);

        h = info->focus_hist;

        h->SetMarkerColor(kRed);
        h->SetMarkerStyle(kFullCircle);
        // h->FillRandom("gaus", 10000);

        info->focus = new double [ n_focus ];
        info->focus_max = new double [ n_focus ];
        info->n_focus = n_focus;
        for (int i = 0; i < n_focus; ++i) {
            info->focus_max[i] = 0.0;
            info->focus[i] = 0.0;
        }

        TCanvas *c = info->canvas;

//        c->cd();

        info->focus_hist_max->Draw();
        info->focus_hist_max->Draw("same p");
        info->focus_hist->Draw("same p");

        c->Modified();
        c->Update();
        c->Draw();

        info->run = 1;
        pthread_create(&info->thread_id_app, NULL, graph_focus_app_looper, info);
        pthread_create(&info->thread_id, NULL, graph_focus_looper, info);

    }

    int display_focus_graph(double *focus)
    {
        FocusGraphInfo *info = &focus_graph_info;

        printf("display_focus_graph(semaphore=%d, n=%d)\n", info->new_data_semaphore, info->n_focus);
        if (info->new_data_semaphore == 1) {
            return 1;
        }

        int i;
        for (i = 0; i < info->n_focus; ++i) {
            double f = focus[i];
            info->focus[i] = f;
            if (f > info->focus_max[i]) {
                info->focus_max[i] = f;
            }
            printf("display() : %d %f %f\n", i, info->focus[i], info->focus_max[i]);
        }
#if 0
        info->focus[i++] = focus_img;
        info->focus[i++] = focus_roi;
#endif

        int bin = 1;
        for (i = 0; i < info->n_focus; ++i, ++bin) {
            info->focus_hist->SetBinContent(bin, info->focus[i]);
            info->focus_hist_max->SetBinContent(bin, info->focus_max[i]);
        }

#if 0
        info->focus_hist->SetBinContent(bin, info->focus[i]);
        info->focus_hist_max->SetBinContent(bin, info->focus_max[i]);
        ++i;
        ++bin;

        info->focus_hist->SetBinContent(bin, info->focus[i]);
        info->focus_hist_max->SetBinContent(bin, info->focus_max[i]);
        ++i;
        ++bin;
#endif

        info->new_data_semaphore = 1;
        return 0;
    }

}

