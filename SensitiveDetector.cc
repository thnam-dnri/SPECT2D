#include "SensitiveDetector.hh"
#include "G4Step.hh"
#include "G4TouchableHistory.hh"
#include "G4SystemOfUnits.hh"
#include "G4EventManager.hh"

SensitiveDetector::SensitiveDetector(G4String name, G4String hcName)
 : G4VSensitiveDetector(name)
{
    collectionName.insert(hcName);
}

SensitiveDetector::~SensitiveDetector() {}

void SensitiveDetector::Initialize(G4HCofThisEvent*)
{
    // Initialize each crystal with -1.0 to indicate "no hit"
    for (auto& v : fEdep) {
        v = -1.0;
    }
}

G4bool SensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    G4double dE = step->GetTotalEnergyDeposit();
    if (dE <= 30. * keV) return false;

    const auto* touch = step->GetPreStepPoint()->GetTouchable();
    G4int copyNo = touch->GetVolume()->GetCopyNo();  // 0..15
    if (copyNo < 0 || copyNo >= 16) return false;    // Safety check

    if (fEdep[copyNo] < 0.0)
        fEdep[copyNo] = 0.0;  // First hit: reset from -1.0

    fEdep[copyNo] += dE;
    return true;
}

void SensitiveDetector::GetEdepArray(G4double outArray[16]) const
{
    for (G4int i = 0; i < 16; ++i) {
        outArray[i] = fEdep[i];
    }
}