#ifndef PRIMARYGENERATORACTION_HH
#define PRIMARYGENERATORACTION_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"

// ─── Add these two lines so that G4ThreeVector and units (cm, mm, etc.) are known ───
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
public:
    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;

    /// Called by the messenger to shift the source position
    void SetSourceOffset(const G4ThreeVector& off) { fSourceOffset = off; }

    /// This is where primaries are generated each event
    void GeneratePrimaries(G4Event* event) override;

private:
    G4ParticleGun*  fParticleGun;

    // User‐driven source center (default still +20 cm in Z)
    G4ThreeVector   fSourceOffset{0, 0, 20*cm};
};

#endif // PRIMARYGENERATORACTION_HH
