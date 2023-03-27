#include <TApplication.h>
#include <TGraph.h>
#include <TAxis.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TRootCanvas.h>
#include <TLatex.h>
#include <TText.h>

#include <pthread.h>

//#include <stdlib.h>
//#include <stdio.h>
#include <unistd.h>

#include "plot.hpp"

TText *g_text = 0;
int iter = 0;

void *app_looper(void *argv)
{
    while (1) {
        sleep(1);
        printf("iter = %d\n", ++iter);
        if (iter >= 1) {
            g_text = new TText(0.0, 0.0, "hello");
            g_text->Draw();
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    // TApplication app = TApplication("app", &argc, argv);
    TApplication app("app", &argc, argv);
    TCanvas* c = new TCanvas("c", "Something", 0, 0, 800, 600);

    double x = -5.0;
    int n_x = 16;
    int head = 0;
    int mask = n_x - 1;
    double *graph_x = new double[16];
    double *graph_y = new double[16];

    pthread_t tid;

    pthread_create(&tid, NULL, app_looper, 0);

    // TF1 *f1 = new TF1("f1","sin(x)", x, x + 10);
    TGraph *graph = new TGraph(n_x, graph_x, graph_y);
    graph->SetName("graph");
    graph->SetMarkerColor(kBlue);
    graph->SetMarkerStyle(kOpenCircle);
    graph->SetMinimum(0.0);
    graph->SetMaximum(1.0);
    // graph->GetXaxis()->SetRangeUser(-1.25, 1.25);
    graph->GetXaxis()->SetLimits(-1.25, 1.25);

    int i = 1;
    while (1)
    {

        if ((i % 2) == 0) {
            graph->SetMarkerColor(kBlue);
        } else {
            graph->SetMarkerColor(kRed);
        }

        for (int i = 0; i < n_x; ++i) {
            double phase = x + 0.1;
            graph_x[i] = sin(phase + i * 0.1);
            graph_y[i] = 0.5;
        }

        x = x + 0.1;

        for (int i = 0; i < n_x; ++i) {
            graph->SetPoint(i, graph_x[i], graph_y[i]);
        }

        // graph->GetXaxis()->SetRangeUser(-1.25, 1.25);
        graph->GetXaxis()->SetLimits(-1.25, 1.25);

        graph->Draw("ap");

        c->Modified();
        c->Update();
        c->Draw();

        c->WaitPrimitive();
        ++i;
        iter = 0;
    }

    TRootCanvas *rc = (TRootCanvas *) c->GetCanvasImp();
    rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
    app.Run();

#if 0
    TCanvas* c = new TCanvas("c", "Something", 0, 0, 800, 600);
    TF1 *f1 = new TF1("f1","sin(x)", -5, 5);
    f1->SetLineColor(kBlue+1);
    f1->SetTitle("My graph;x; sin(x)");
    f1->Draw();
    c->Print("demo1.pdf");
#endif

    return 0;
}
