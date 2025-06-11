#include "PrimaryGeneratorAction.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4Gamma.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4ThreeVector.hh"
#include "G4UnitsTable.hh"
#include "G4ios.hh"
#include <cmath>
#include "G4RandomDirection.hh"

// Thyroid ellipsoid half-axes and center
static const G4double kThyX = 0.4*cm;
static const G4double kThyY = 0.4*cm;
static const G4double kThyZ = 0.2*cm;

// Body ellipsoid (phantom) half-axes and center
static const G4double kBodyX = 3.5*cm;
static const G4double kBodyY = 1.5*cm;
static const G4double kBodyZ = 1.5*cm;

PrimaryGeneratorAction::PrimaryGeneratorAction()
 : G4VUserPrimaryGeneratorAction(),
   fParticleGun(nullptr),
   fSourceOffset(0, 0, 0)  // Default source position now at origin
{
    fParticleGun = new G4ParticleGun(1);
    fParticleGun->SetParticleDefinition(G4Gamma::Definition());
    fParticleGun->SetParticleEnergy(364.0*keV);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    G4ThreeVector pos;
    G4double r = G4UniformRand();

    if (r < 0.5) {
        // 50% of events in thyroid
        do {
            G4double x = (2.0*G4UniformRand() - 1.0) * kThyX;
            G4double y = (2.0*G4UniformRand() - 1.0) * kThyY;
            G4double z = (2.0*G4UniformRand() - 1.0) * kThyZ;
            if ((x*x)/(kThyX*kThyX) + (y*y)/(kThyY*kThyY) + (z*z)/(kThyZ*kThyZ) <= 1.0) {
                pos = fSourceOffset + G4ThreeVector(x, y, z);
                break;
            }
        } while (true);
    } else if (r < 0.8) {
        // 30% of events in the test box (0.5cm x 0.5cm x 1cm box at +0.8 cm X)
        do {
            G4double x = (2.0 * G4UniformRand() - 1.0) * 0.25*cm;
            G4double y = (2.0 * G4UniformRand() - 1.0) * 0.25*cm;
            G4double z = (2.0 * G4UniformRand() - 1.0) * 0.5*cm;
            pos = fSourceOffset + G4ThreeVector(0.8*cm + x, y, z);
            break;
        } while (true);
    } else {
        // 20% of events in phantom (excluding thyroid and test box)
        do {
            G4double x = (2.0 * G4UniformRand() - 1.0) * kBodyX;
            G4double y = (2.0 * G4UniformRand() - 1.0) * kBodyY;
            G4double z = (2.0 * G4UniformRand() - 1.0) * kBodyZ;

            bool insideBody = ((x*x)/(kBodyX*kBodyX) + (y*y)/(kBodyY*kBodyY) + (z*z)/(kBodyZ*kBodyZ) <= 1.0);
            bool outsideThyroid = ((x*x)/(kThyX*kThyX) + (y*y)/(kThyY*kThyY) + (z*z)/(kThyZ*kThyZ) > 1.0);
            bool outsideBox = !(
                (x > +0.8*cm - 0.25*cm) && (x < +0.8*cm + 0.25*cm) &&
                (y > -0.25*cm) && (y < +0.25*cm) &&
                (z > -0.5*cm) && (z < +0.5*cm)
            );

            if (insideBody && outsideThyroid && outsideBox) {
                pos = fSourceOffset + G4ThreeVector(x, y, z);
                break;
            }
        } while (true);
    }

    fParticleGun->SetParticlePosition(pos);

    // Sample isotropic direction in the lower hemisphere (Z < 0)
    G4ThreeVector dir;
    do {
        dir = G4RandomDirection();  // uniformly on full sphere
    } while (dir.z() > 0);          // reject if not in lower hemisphere
    fParticleGun->SetParticleMomentumDirection(dir);

    fParticleGun->GeneratePrimaryVertex(anEvent);
}

