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
        TH1D *focus_hist_rel;
        TH1D *focus_hist_rel_max;
        double *focus;
        double *focus_max;
        int n_focus;
        TApplication *app;
        TCanvas *canvas;
        TPad *pad_data;
        TPad *pad_diff;
        pthread_t thread_id;
        int run;
        pthread_t thread_id_app;
        int new_data_semaphore;
        int display_semaphore;
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

            info->display_semaphore = 1; /* display is busy */

            info->pad_data->cd();

            info->focus_hist_max->DrawCopy();
            info->focus_hist_max->DrawCopy("same p");
            info->focus_hist->DrawCopy("same p");

            info->pad_diff->cd();

            info->focus_hist_rel_max->DrawCopy();
            info->focus_hist_rel_max->DrawCopy("same p");
            info->focus_hist_rel->DrawCopy("same p");

            TCanvas *c = info->canvas;
            c->Modified();
            c->Update();
            c->Draw();

            info->display_semaphore = 0;

            info->new_data_semaphore = 0;
        }
    }

    int initialize_focus_graph(int n_focus)
    {
        FocusGraphInfo *info = &focus_graph_info;
        memset(&focus_graph_info, 0, sizeof (focus_graph_info));

        info->graph_looper_pace = 100; /* 100us looper pace */

        info->app = new TApplication("app", NULL, NULL);

        info->canvas = new TCanvas("c", "focus-analysis", 0, 0, 800, 500);
        double x_min = 0.02;
        double x_max = 0.98;

        info->pad_data = new TPad("data","FOCUS",x_min,0.42,x_max,0.98);
        info->focus_hist = new TH1D("focus-hist", "FOCUS", n_focus, -0.5, n_focus + 0.5);
        info->focus_hist_max = new TH1D("focus-hist-max", "FOCUS", n_focus, -0.5, n_focus + 0.5);
        // info->pad_diff->SetFillColor(38);
        info->pad_data->Draw();

        info->pad_diff = new TPad("diff","GOAL",x_min,0.02,x_max,0.38);
        info->focus_hist_rel = new TH1D("focus-hist-rel", "FOCUS", n_focus, -0.5, n_focus + 0.5);
        info->focus_hist_rel_max = new TH1D("focus-hist-rel-max", "FOCUS", n_focus, -0.5, n_focus + 0.5);
        info->pad_diff->Draw();

        TH1D *h = info->focus_hist_max;
        h->SetMarkerColor(kBlue);
        h->SetMarkerStyle(kOpenCircle);
        h->SetFillColor(kYellow);
        h->SetStats(kFALSE);

        h = info->focus_hist_rel_max;
        h->SetMarkerColor(kBlue);
        h->SetMarkerStyle(kOpenCircle);
        h->SetFillColor(kYellow);
        h->SetTitle("");
        h->SetStats(kFALSE);
        h->SetMinimum(-0.1);
        h->SetMaximum(2.1);

        char label[32];
        int bin;
        h = info->focus_hist_max;
        TAxis *axis = h->GetXaxis();
        for (bin = 1; bin <= (n_focus - 2); ++bin) {
            snprintf(label, sizeof (label), "ROI-%d", bin);
            axis->SetBinLabel(bin, label);
        }
        snprintf(label, sizeof (label), "IMG");
        axis->SetBinLabel(bin, label);
        ++bin;
        snprintf(label, sizeof (label), "ROI");
        axis->SetBinLabel(bin, label);
        ++bin;

        h = info->focus_hist;
        h->SetMarkerColor(kRed);
        h->SetMarkerStyle(kFullCircle);

        h = info->focus_hist_rel;
        h->SetMarkerColor(kRed);
        h->SetMarkerStyle(kFullCircle);

        info->focus = new double [ n_focus ];
        info->focus_max = new double [ n_focus ];
        info->n_focus = n_focus;
        for (int i = 0; i < n_focus; ++i) {
            info->focus_max[i] = 0.0;
            info->focus[i] = 0.0;
        }

#if 0
        info->pad_data->cd();
        info->focus_hist_max->Draw();
        info->focus_hist_max->Draw("same p");
        info->focus_hist->Draw("same p");

        info->pad_data->cd();
        info->focus_hist_rel->Draw();
        info->focus_hist_rel->Draw("same p");
#endif

#if 0
        c->Modified();
        c->Update();
        c->Draw();
#endif

        info->run = 1;
        pthread_create(&info->thread_id_app, NULL, graph_focus_app_looper, info);
        pthread_create(&info->thread_id, NULL, graph_focus_looper, info);

    }

    int display_focus_graph(double *focus)
    {
        FocusGraphInfo *info = &focus_graph_info;

        if (info->display_semaphore == 1) {
            // return 1;
        }

        // printf("display_focus_graph(semaphore=%d, n=%d)\n", info->new_data_semaphore, info->n_focus);
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

            // if ((f < 1.0) && i < (info->n_focus - 1)) { printf("display() : %d %f %f\n", i, info->focus[i], info->focus_max[i]); }
        }

        int bin = 1;
        for (i = 0; i < info->n_focus; ++i, ++bin) {
            double a = (info->focus_max[i] > 0) ? (info->focus[i] / info->focus_max[i]) : 0.0;
            info->focus_hist->SetBinContent(bin, info->focus[i]);
            info->focus_hist_max->SetBinContent(bin, info->focus_max[i]);
            info->focus_hist_rel->SetBinContent(bin, a);
            info->focus_hist_rel_max->SetBinContent(bin, 1.0);
        }

        info->new_data_semaphore = 1;
        return 0;
    }

}
