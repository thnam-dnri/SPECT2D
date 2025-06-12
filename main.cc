#include <sys/stat.h>    // mkdir
#include <string>

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4String.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"            // ← Added for visualization

#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "PhysicsList.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"
#include "SimMessenger.hh"

int main(int argc, char** argv)
{
    // UI executive (for interactive mode)
    G4UIExecutive* ui = nullptr;
    const bool batchMode = (argc > 1);
    if (!batchMode) {
        ui = new G4UIExecutive(argc, argv);
    }

    // Determine macro file & base name
    G4String macroFile;
    G4String base;
    if (batchMode) {
        macroFile = argv[1];
        std::string fname = macroFile;
        auto slash = fname.find_last_of("/\\");
        if (slash != std::string::npos) fname = fname.substr(slash + 1);
        auto dot = fname.rfind('.');
        base = (dot != std::string::npos ? fname.substr(0, dot) : fname);
    } else {
        base = "interactive";
    }

    // Prepare output directory
    const std::string outDir = "./Output_Root";
    mkdir(outDir.c_str(), 0755);
    G4String outFile = outDir + "/" + base + ".root";

    // --- Set up the run manager
    auto* runManager = new G4RunManager;

    // Detector & physics
    auto* detector = new DetectorConstruction();
    runManager->SetUserInitialization(detector);
    runManager->SetUserInitialization(new PhysicsList());

    // Primary generator
    auto* genAction = new PrimaryGeneratorAction();
    runManager->SetUserAction(genAction);

    // RunAction (needs output file)
    auto* runAction = new RunAction(outFile);
    runManager->SetUserAction(runAction);

    // EventAction (needs RunAction & DetectorConstruction)
    auto* eventAction = new EventAction(runAction, detector);
    runManager->SetUserAction(eventAction);

    // SteppingAction (needs EventAction)
    auto* steppingAction = new SteppingAction(eventAction);
    runManager->SetUserAction(steppingAction);

    // Messenger for UI commands (keep alive)
//    static auto* simMessenger = new SimMessenger(detector, genAction);
	(void) new SimMessenger(detector, genAction);
    // Initialize Geant4 kernel (build geometry, physics, etc.)
    runManager->Initialize();

    // --- Visualization setup (NEW)
    auto* visManager = new G4VisExecutive;
    visManager->Initialize();

    // UI manager: execute macros
    auto* UImanager = G4UImanager::GetUIpointer();
    if (batchMode) {
        UImanager->ApplyCommand("/control/execute " + G4String(macroFile));
    } else {
        // make sure you have an "init_vis.mac" in your working dir
        UImanager->ApplyCommand("/control/execute init_vis.mac");
        ui->SessionStart();
        delete ui;
    }

    // Clean up
    delete visManager;       // ← Remove visualization manager
    delete runManager;       // ← Remove run manager

    return 0;
}

