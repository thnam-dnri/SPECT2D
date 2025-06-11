#ifndef SENSITIVEDETECTOR_HH
#define SENSITIVEDETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include <array>

class SensitiveDetector : public G4VSensitiveDetector
{
public:
    SensitiveDetector(G4String name, G4String hcName);
    ~SensitiveDetector() override;

    /// Called at the start of each event: reset fEdep[i] = –1.0 for all crystals
    void Initialize(G4HCofThisEvent* HCE) override;

    /// Called for each step in a crystal: accumulate energy deposit
    G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;

    /// Copy the entire 16-element energy array into outArray[16].
    /// fEdep[i] = –1.0 means “no hit”; otherwise it’s the total energy deposited.
    void GetEdepArray(G4double outArray[16]) const;

private:
    /// fEdep[i] holds (for this event) the total energy deposited in crystal i:
    ///   - initialized to –1.0 in Initialize()
    ///   - if a step deposits >0 in crystal i, fEdep[i] is set to 0.0 (first hit) then accumulates
    std::array<G4double, 16> fEdep;
};

#endif  // SENSITIVEDETECTOR_HH
