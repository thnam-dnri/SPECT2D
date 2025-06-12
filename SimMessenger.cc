
// SimMessenger.cc
#include "SimMessenger.hh"
#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"

#include "G4UIdirectory.hh"
#include "G4UIcmdWith3Vector.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4SystemOfUnits.hh"

SimMessenger::SimMessenger(DetectorConstruction* det,
                           PrimaryGeneratorAction* gen)
 : G4UImessenger(),
   fDetector(det),
   fGenerator(gen)
{
    // Create /simulation/ command directory
    auto* simDir = new G4UIdirectory("/simulation/");
    simDir->SetGuidance("Simulation control commands");

    // /simulation/offset command: set phantom & source offset
    fOffsetCmd = new G4UIcmdWith3Vector("/simulation/offset", this);
    fOffsetCmd->SetGuidance("Set global offset for phantom & source");
    fOffsetCmd->SetParameterName("X", "Y", "Z", false);
    fOffsetCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    // /simulation/rotate command: set global rotation angle
    fRotationCmd = new G4UIcmdWithADoubleAndUnit("/simulation/rotate", this);
    fRotationCmd->SetGuidance("Set global rotation angle for phantom & source");
    fRotationCmd->SetParameterName("angle", false);
    fRotationCmd->SetUnitCategory("Angle");
    fRotationCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

SimMessenger::~SimMessenger()
{
    delete fOffsetCmd;
    delete fRotationCmd;
}

void SimMessenger::SetNewValue(G4UIcommand* cmd, G4String newValue)
{
    if (cmd == fOffsetCmd) {
        G4ThreeVector off = fOffsetCmd->GetNew3VectorValue(newValue);
        fDetector->SetPhantomOffset(off);
        fGenerator->SetSourceOffset(off);
    }
    else if (cmd == fRotationCmd) {
        G4double angle = fRotationCmd->GetNewDoubleValue(newValue);
        fDetector->SetPhantomRotation(angle);
        fGenerator->SetSourceRotation(angle);
    }
}

