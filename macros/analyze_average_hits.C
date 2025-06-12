#include <TFile.h>
#include <TTree.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TString.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TList.h>
#include <TROOT.h>
#include <iostream>
#include <vector>
#include <cstring>

// Helper: list all .root files in the current directory
std::vector<TString> listRootFiles(const TString& dirPath = ".") {
    std::vector<TString> rootFiles;
    TSystemDirectory dir("rootDir", dirPath);
    TList* files = dir.GetListOfFiles();
    if (!files) return rootFiles;
    TSystemFile* file;
    TIter next(files);
    while ((file = (TSystemFile*)next())) {
        TString fname = file->GetName();
        if (!file->IsDirectory() && fname.EndsWith(".root")) {
            rootFiles.push_back(dirPath + "/" + fname);
        }
    }
    return rootFiles;
}

void analyze_average_hits() {
    // Run in batch mode
    gROOT->SetBatch(kTRUE);

    // Gather all .root files
    auto fileNames = listRootFiles(".");
    if (fileNames.empty()) {
        std::cerr << "No .root files found.\n";
        return;
    }

    // Histogram parameters
    const int    nbins     = 42;
    const double binW     = 1.68; 
    const double totalR   = nbins * binW;
    const double halfR    = totalR / 2.0;
    const int    totalBins= nbins + 2;  // under/overflow

    // Create only the average-hits histogram
    TH2D* hAvg = new TH2D(
        "hAvg",
        "Average Hits per Visit;X (mm);Y (mm)",
        nbins, -halfR, halfR,
        nbins, -halfR, halfR
    );
    hAvg->SetOption("COLZ");

    // Global accumulators
    static long long globalHits[107][107]  = {{0}};
    static long long visitCounts[107][107] = {{0}};

    // Loop over files
    for (auto& fname : fileNames) {
        TFile* file = TFile::Open(fname, "READ");
        if (!file || file->IsZombie()) {
            std::cerr << "Cannot open " << fname << "\n";
            continue;
        }
        TTree* tree = dynamic_cast<TTree*>(file->Get("datatree"));
        if (!tree) {
            std::cerr << "No TTree 'datatree' in " << fname << "\n";
            file->Close();
            continue;
        }

        // Local counts for this file
        static int localCounts[107][107];
        std::memset(localCounts, 0, sizeof(localCounts));

        // Branches
        double edep[16], xpos[16], ypos[16];
        for (int i=0; i<16; ++i) {
            tree->SetBranchAddress(Form("edep%d", i), &edep[i]);
            tree->SetBranchAddress(Form("xpos%d", i), &xpos[i]);
            tree->SetBranchAddress(Form("ypos%d", i), &ypos[i]);
        }

        // Loop events
        Long64_t nE = tree->GetEntries();
        for (Long64_t ev=0; ev<nE; ++ev) {
            tree->GetEntry(ev);

            // find index of crystal with max edep
            int    maxIdx = 0;
            double maxE   = edep[0];
            for (int i=1; i<16; ++i) {
                if (edep[i] > maxE) {
                    maxE   = edep[i];
                    maxIdx = i;
                }
            }
            // only count if above threshold
            if (maxE <= 0.3) continue;

            // find the bin
            int xb = hAvg->GetXaxis()->FindBin(xpos[maxIdx]);
            int yb = hAvg->GetYaxis()->FindBin(ypos[maxIdx]);
            // manual clamp
            xb = (xb<0 ? 0 : (xb>nbins+1 ? nbins+1 : xb));
            yb = (yb<0 ? 0 : (yb>nbins+1 ? nbins+1 : yb));

            localCounts[xb][yb] += 1;
        }
        file->Close();

        // update globals
        for (int i=0; i<totalBins; ++i) {
            for (int j=0; j<totalBins; ++j) {
                int c = localCounts[i][j];
                if (c>0) {
                    globalHits[i][j]  += c;
                    visitCounts[i][j]++;
                }
            }
        }
    }

    // Fill the average histogram
    for (int i=0; i<totalBins; ++i) {
        for (int j=0; j<totalBins; ++j) {
            if (visitCounts[i][j] > 0) {
                double avg = double(globalHits[i][j]) / visitCounts[i][j];
                hAvg->SetBinContent(i, j, avg);
            }
        }
    }

    // Draw & save
    TCanvas* c = new TCanvas("c","Avg Hits",800,600);
    hAvg->Draw("COLZ");
    c->SaveAs("average_hits.png");
    std::cout << "Saved average_hits.png\n";
}

