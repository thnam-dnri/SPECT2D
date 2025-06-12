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
#include <map>
#include <string>
#include <cstring>

// List all .root files in dirPath
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
    // Batch mode
    gROOT->SetBatch(kTRUE);

    // Gather files
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

    // Data structures: map angle→(globalHits,visitCounts)
    std::map<int, std::vector<std::vector<long long>>> globalHitsMap;
    std::map<int, std::vector<std::vector<long long>>> visitCountsMap;

    // Prepare per-file localCounts
    int localCounts[107][107];

    // Loop over each file
    for (auto& fname : fileNames) {
        // 1) Parse rotation angle from filename, e.g. "..._rot_30.root"
        std::string s = std::string(fname.Data());
        int angle = 0;
        size_t p = s.find("_rot_");
        if (p != std::string::npos) {
            size_t start = p + 5;
            size_t end = s.find_first_not_of("0123456789", start);
            if (end == std::string::npos) end = s.find(".root", start);
            angle = std::stoi(s.substr(start, end - start));
        }

        // 2) Ensure maps have entries for this angle
        if (!globalHitsMap.count(angle)) {
            globalHitsMap[angle]  = std::vector<std::vector<long long>>(totalBins,
                                          std::vector<long long>(totalBins, 0));
            visitCountsMap[angle] = globalHitsMap[angle];  // same dims, zeroed
        }

        // 3) Open file and tree
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

        // 4) Zero localCounts
        std::memset(localCounts, 0, sizeof(localCounts));

        // 5) Set branch addresses
        double edep[16], xpos[16], ypos[16];
        for (int i = 0; i < 16; ++i) {
            tree->SetBranchAddress(Form("edep%d", i), &edep[i]);
            tree->SetBranchAddress(Form("xpos%d", i), &xpos[i]);
            tree->SetBranchAddress(Form("ypos%d", i), &ypos[i]);
        }

        // 6) Fill localCounts: pick crystal with max edep > 0.3 MeV
        Long64_t nE = tree->GetEntries();
        for (Long64_t ev = 0; ev < nE; ++ev) {
            tree->GetEntry(ev);
            int    maxIdx = 0;
            double maxE   = edep[0];
            for (int i = 1; i < 16; ++i) {
                if (edep[i] > maxE) {
                    maxE   = edep[i];
                    maxIdx = i;
                }
            }
            if (maxE <= 0.3) continue;
            int xb = tree->GetXaxis()->FindBin(xpos[maxIdx]);
            int yb = tree->GetYaxis()->FindBin(ypos[maxIdx]);
            // clamp
            xb = (xb < 0 ? 0 : (xb > nbins+1 ? nbins+1 : xb));
            yb = (yb < 0 ? 0 : (yb > nbins+1 ? nbins+1 : yb));
            localCounts[xb][yb] += 1;
        }
        file->Close();

        // 7) Update globalHitsMap and visitCountsMap
        auto& gH = globalHitsMap[angle];
        auto& vC = visitCountsMap[angle];
        for (int i = 0; i < totalBins; ++i) {
            for (int j = 0; j < totalBins; ++j) {
                int c = localCounts[i][j];
                if (c > 0) {
                    gH[i][j] += c;
                    vC[i][j] += 1;
                }
            }
        }
    } // end file loop

    // 8) For each angle, build & save its histogram
    for (auto& kv : globalHitsMap) {
        int angle = kv.first;
        auto& gH = kv.second;
        auto& vC = visitCountsMap[angle];

        // Create histogram
        TH2D* hAvg = new TH2D(
            Form("hAvg_%d", angle),
            Form("Avg Hits (rot=%d°);X (mm);Y (mm)", angle),
            nbins, -halfR, halfR,
            nbins, -halfR, halfR
        );
        hAvg->SetOption("COLZ");

        // Fill average
        for (int i = 0; i < totalBins; ++i) {
            for (int j = 0; j < totalBins; ++j) {
                if (vC[i][j] > 0) {
                    double avg = double(gH[i][j]) / vC[i][j];
                    hAvg->SetBinContent(i, j, avg);
                }
            }
        }

        // Draw & save
        TCanvas* c = new TCanvas(
            Form("c_rot_%d", angle),
            Form("Avg Hits rot %d°", angle),
            800, 600
        );
        hAvg->Draw("COLZ");
        c->SaveAs(Form("average_hits_rot_%d.png", angle));
        delete c;
        delete hAvg;
    }

    std::cout << ">> Saved average-hits maps for all rotation angles.\n";
}

