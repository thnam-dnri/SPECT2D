#include <sys/stat.h>    // mkdir
#include <string>

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4String.hh"

#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "PhysicsList.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"
#include "SimMessenger.hh"

int main(int argc, char** argv) {
    if (argc != 2) {
        G4cerr << "Usage: " << argv[0] << " <macroFile.mac>" << G4endl;
        return 1;
    }

    // Extract the base name (no path, no extension)
    std::string macroPath(argv[1]);
    size_t slash = macroPath.find_last_of("/\\");
    std::string filename = (slash == std::string::npos
                            ? macroPath
                            : macroPath.substr(slash + 1));
    std::string base = filename;
    size_t dot = filename.rfind('.');
    if (dot != std::string::npos && filename.substr(dot) == ".mac") {
        base = filename.substr(0, dot);
    }

    // Prepare output directory
    const std::string outDir = "./Output_Root";
    mkdir(outDir.c_str(), 0755);

    // Build full ROOT filename
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
    static auto* simMessenger = new SimMessenger(detector, genAction);

    // Initialize Geant4 kernel
    runManager->Initialize();

    // Execute the macro
    G4UImanager::GetUIpointer()->ApplyCommand(
        "/control/execute " + G4String(argv[1])
    );

    delete runManager;
    return 0;
}

