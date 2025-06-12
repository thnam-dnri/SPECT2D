#ifndef PRIMARYGENERATORACTION_HH
#define PRIMARYGENERATORACTION_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"

// ─── Ensure G4ThreeVector and units are available ───
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
public:
    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;

    /// Called by the messenger to shift the source position
    void SetSourceOffset(const G4ThreeVector& off);

    /// Called by the messenger to rotate the source around Z
    void SetSourceRotation(G4double angle);

    /// This is where primaries are generated each event
    void GeneratePrimaries(G4Event* event) override;

private:
    G4ParticleGun*  fParticleGun;

    // User‐driven source center offset and rotation angle
    G4ThreeVector   fSourceOffset{0, 0, 0*cm};
    G4double        fSourceRotation{0.0};   ///< rotation angle (Z axis)
};

#endif // PRIMARYGENERATORACTION_HH

