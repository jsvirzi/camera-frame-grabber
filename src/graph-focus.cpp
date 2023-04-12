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
    } FocusGraphInfo;

    static FocusGraphInfo focus_graph_info;

    int initialize_focus_graph(int n_focus)
    {
        FocusGraphInfo *info = &focus_graph_info;
        info->app = new TApplication("app", NULL, NULL);
        // TApplication app("app", &argc, argv);
        info->canvas = new TCanvas("c", "focus-analysis", 0, 0, 800, 600);

        info->focus_hist = new TH1D("focus-hist", "FOCUS", n_focus, -0.5, n_focus + 0.5);
        info->focus_hist_max = new TH1D("focus-hist", "FOCUS", n_focus, -0.5, n_focus + 0.5);

        TH1D *h = info->focus_hist_max;

        h->SetMarkerColor(kBlue);
        h->SetMarkerStyle(kOpenCircle);
        // h->SetMaximum(100.0); /* token maximum. reset later with real data */
        h->FillRandom("gaus", 20000);

        h = info->focus_hist;

        h->SetMarkerColor(kRed);
        h->SetMarkerStyle(kFullCircle);

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
        // info->focus_hist->Draw("same");

        c->Modified();
        c->Update();
        c->Draw();
    }

    int display_focus_graph(double *focus)
    {

    }

}
