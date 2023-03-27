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
        if (iter > 5) {
            g_text = new TText(0.0, 0.0, "hello");
            g_text->Draw();
//            g_text->SetName("blah");
//            if (g_text->GetName() != "lat") {
//                g_text->SetName("lat");
//                g_text->Draw();
//            }
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

    pthread_t tid;

    pthread_create(&tid, NULL, app_looper, 0);

    TF1 *f1 = new TF1("f1","sin(x)", x, x + 10);
    f1->Draw("alp");
    int i = 1;
    while (1)
    {
        if ((i % 2) == 0) {
            f1->SetLineColor(kBlue+1);
            f1->SetMarkerColor(kBlue + 1);
        } else {
            f1->SetLineColor(kRed);
            f1->SetMarkerColor(kRed);
        }
        f1->SetTitle("My graph;x; sin(x)");
        f1->Draw("lp");
        c->Modified();
        c->Update();
        c->Draw();
        // c->WaitPrimitive("lat", "Text");
        c->WaitPrimitive();
        ++i;
        iter = 0;
    }

    TRootCanvas *rc = (TRootCanvas *) c->GetCanvasImp();
    rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
    app.Run();

    while (1)
    {
        f1->SetLineColor(kBlue+1);
        f1->SetMarkerColor(kBlue + 1);
        f1->SetTitle("My graph;x; sin(x)");
        f1->Draw("alp");
        c->Modified();
        c->Update();
        c->Draw();
    }


    // TGraph *graph = new TGraph();
    // grap
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
