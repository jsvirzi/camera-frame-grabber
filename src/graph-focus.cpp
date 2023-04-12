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
    } FocusGraphInfo;

    static FocusGraphInfo focus_graph_info;

    void *graph_focus_looper(void *arg)
    {
        FocusGraphInfo *info = (FocusGraphInfo *) arg;
        while (info->run) {
            sleep(1);

            info->focus_hist_max->FillRandom("gaus", 10000);

            TCanvas *c = info->canvas;

            c->cd();

            info->focus_hist_max->Draw();
            info->focus_hist_max->Draw("same p");
            info->focus_hist->Draw("same p");

            c->Modified();
            c->Update();
            c->Draw();

        }
    }

    int initialize_focus_graph(int n_focus, int *argc, char **argv)
    {
        FocusGraphInfo *info = &focus_graph_info;
        memset(&focus_graph_info, 0, sizeof (focus_graph_info));

        info->app = new TApplication("app", argc, argv);
        // TApplication app("app", &argc, argv);
        info->canvas = new TCanvas("c", "focus-analysis", 0, 0, 800, 600);

        info->focus_hist = new TH1D("focus-hist", "FOCUS", n_focus, -0.5, n_focus + 0.5);
        info->focus_hist_max = new TH1D("focus-hist-max", "FOCUS", n_focus, -0.5, n_focus + 0.5);

        TH1D *h = info->focus_hist_max;

        h->SetMarkerColor(kBlue);
        h->SetMarkerStyle(kOpenCircle);
        h->SetFillColor(kYellow);
        // h->SetFillStyle(kFillSolid);
        // h->SetMaximum(100.0); /* token maximum. reset later with real data */
        h->FillRandom("gaus", 20000);

        h = info->focus_hist;

        h->SetMarkerColor(kRed);
        h->SetMarkerStyle(kFullCircle);
        h->FillRandom("gaus", 10000);

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
        pthread_create(&info->thread_id, NULL, graph_focus_looper, info);

        info->app->Run();
    }

    int display_focus_graph(double *focus)
    {

    }

}

