// -------- RunAction.cc --------
#include "RunAction.hh"
#include "G4RootAnalysisManager.hh"
#include "G4Run.hh"
#include "G4SystemOfUnits.hh"  // optional, if you use units in messages
#include <unistd.h>
#include <limits.h>
#include <string>
#include "G4String.hh"
#include "G4ios.hh"
// Constructor: store filename and get analysis manager instance
RunAction::RunAction(const G4String& outFileName)
 : G4UserRunAction(),
   fAnalysisManager(G4RootAnalysisManager::Instance()),
   fOutFileName(outFileName)
{
}

// Destructor: clean up the analysis manager instance
RunAction::~RunAction() {
    delete G4RootAnalysisManager::Instance();
}

void RunAction::BeginOfRunAction(const G4Run* /*run*/) {
    // Print working directory and file for clarity
    char cwd_buf[PATH_MAX];
    if (getcwd(cwd_buf, sizeof(cwd_buf))) {
        G4cout << ">>> Opening ROOT file: " << fOutFileName
               << " in directory: " << cwd_buf << G4endl;
    }
    // Open the ROOT file
    fAnalysisManager->OpenFile(fOutFileName);

    // Create ntuple: 48 columns (edep[0..15], xpos[0..15], ypos[0..15])
    fAnalysisManager->CreateNtuple("datatree", "Crystal Energies + Pinhole Positions");
    for (G4int i = 0; i < 16; ++i) {
        fAnalysisManager->CreateNtupleDColumn("edep" + std::to_string(i));
    }
    for (G4int i = 0; i < 16; ++i) {
        fAnalysisManager->CreateNtupleDColumn("xpos" + std::to_string(i));
    }
    for (G4int i = 0; i < 16; ++i) {
        fAnalysisManager->CreateNtupleDColumn("ypos" + std::to_string(i));
    }
    fAnalysisManager->FinishNtuple();
}

void RunAction::EndOfRunAction(const G4Run* /*run*/) {
    fAnalysisManager->Write();
    fAnalysisManager->CloseFile();
}
