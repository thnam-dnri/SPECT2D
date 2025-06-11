#include "EventAction.hh"
#include "RunAction.hh"
#include "DetectorConstruction.hh"
#include "SensitiveDetector.hh"

#include "G4RootAnalysisManager.hh"
#include "G4Event.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"

EventAction::EventAction(RunAction* runAction, DetectorConstruction* detCon)
  : G4UserEventAction(),
    fRunAction(runAction),
    fDetCon(detCon)
{
    for (G4int i = 0; i < 16; ++i) {
        fEdep[i] = -1.0;
        fXpos[i] = 0.0;
        fYpos[i] = 0.0;
    }
}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event*)
{
    for (G4int i = 0; i < 16; ++i) {
        fEdep[i] = -1.0;  // “no hit” default
        fXpos[i] = 0.0;
        fYpos[i] = 0.0;
    }
}

void EventAction::EndOfEventAction(const G4Event*)
{
    G4SDManager* sdMan = G4SDManager::GetSDMpointer();
    auto* crystalSD = dynamic_cast<SensitiveDetector*>(
        sdMan->FindSensitiveDetector("CrystalSD"));

    if (!crystalSD) {
        G4Exception("EventAction::EndOfEventAction", "NoSD", FatalException,
                    "Could not find or cast 'CrystalSD'");
    }

    // Retrieve energy deposition per crystal
    crystalSD->GetEdepArray(fEdep);

    // Skip event if no crystal was hit
    bool hitDetected = false;
    for (G4int i = 0; i < 16 && !hitDetected; ++i) {
        if (fEdep[i] >= 0.0) hitDetected = true;
    }
    if (!hitDetected) return;

    // Retrieve (x, y) of pinhole centers from geometry
    const auto& pinCenters = fDetCon->GetPinholeCenters();
    for (std::size_t i = 0; i < pinCenters.size(); ++i) {
        fXpos[i] = pinCenters[i].x() / mm;
        fYpos[i] = pinCenters[i].y() / mm;
    }

    // Apply phantom offset (from user macro command)
    G4ThreeVector offset = fDetCon->GetPhantomOffset();
    double offX = offset.x() / mm;
    double offY = offset.y() / mm;

    auto* analysisManager = G4RootAnalysisManager::Instance();
    const G4int ntID = 0;

    for (G4int i = 0; i < 16; ++i) {
        analysisManager->FillNtupleDColumn(ntID, i, fEdep[i]);                // edep
        analysisManager->FillNtupleDColumn(ntID, 16 + i, fXpos[i] - offX);    // xpos
        analysisManager->FillNtupleDColumn(ntID, 32 + i, fYpos[i] - offY);    // ypos
    }

    analysisManager->AddNtupleRow(ntID);
}
