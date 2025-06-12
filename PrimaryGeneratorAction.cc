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
#include "G4RotationMatrix.hh"
#include "G4PhysicalConstants.hh"   // for twopi

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
   fSourceOffset(0, 0, 0),
   fSourceRotation(0.0)
{
    fParticleGun = new G4ParticleGun(1);
    fParticleGun->SetParticleDefinition(G4Gamma::Definition());
    fParticleGun->SetParticleEnergy(364.0*keV);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
}

void PrimaryGeneratorAction::SetSourceOffset(const G4ThreeVector& offset) {
    fSourceOffset = offset;
}

void PrimaryGeneratorAction::SetSourceRotation(G4double angle) {
    fSourceRotation = angle;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    G4ThreeVector pos;
    G4double r = G4UniformRand();

    if (r < 0.5) {
        // 50% in thyroid
        do {
            G4double x = (2.0*G4UniformRand() - 1.0) * kThyX;
            G4double y = (2.0*G4UniformRand() - 1.0) * kThyY;
            G4double z = (2.0*G4UniformRand() - 1.0) * kThyZ;
            if ((x*x)/(kThyX*kThyX) + (y*y)/(kThyY*kThyY) + (z*z)/(kThyZ*kThyZ) <= 1.0) {
                pos = G4ThreeVector(x, y, z);
                break;
            }
        } while(true);
    } else if (r < 0.8) {
        // 30% in test box
        G4double x = 0.8*cm + (2.0*G4UniformRand() - 1.0)*0.25*cm;
        G4double y = (2.0*G4UniformRand() - 1.0)*0.25*cm;
        G4double z = (2.0*G4UniformRand() - 1.0)*0.5*cm;
        pos = G4ThreeVector(x, y, z);
    } else {
        // 20% in body
        do {
            G4double x = (2.0*G4UniformRand() - 1.0) * kBodyX;
            G4double y = (2.0*G4UniformRand() - 1.0) * kBodyY;
            G4double z = (2.0*G4UniformRand() - 1.0) * kBodyZ;
            bool inBody = ((x*x)/(kBodyX*kBodyX) + (y*y)/(kBodyY*kBodyY) + (z*z)/(kBodyZ*kBodyZ) <= 1.0);
            bool inThy  = ((x*x)/(kThyX*kThyX) + (y*y)/(kThyY*kThyY) + (z*z)/(kThyZ*kThyZ) <= 1.0);
            bool inBox  = (x > 0.8*cm-0.25*cm && x < 0.8*cm+0.25*cm && y > -0.25*cm && y < 0.25*cm && z > -0.5*cm && z < 0.5*cm);
            if (inBody && !inThy && !inBox) {
                pos = G4ThreeVector(x, y, z);
                break;
            }
        } while(true);
    }

    // Apply source rotation about X
    G4RotationMatrix rot;
    rot.rotateX(fSourceRotation);
    pos = rot * pos + fSourceOffset;

    fParticleGun->SetParticlePosition(pos);

    	// Sample isotropic direction in lower hemisphere
/*
    G4ThreeVector dir;
    do {
        dir = G4RandomDirection();
    } while(dir.z() > 0);
    fParticleGun->SetParticleMomentumDirection(dir);
*/
	// Emitting in the negative Z direction
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0.0, 0.0, -1.0));
 
 	// Sample direction within a 45° cone around the –Z axis
/*	G4double halfAngle = 30 * deg;
	G4double cosA = std::cos(halfAngle);
	G4double cosTheta = cosA + (1.0 - cosA) * G4UniformRand();  // uniform in solid angle
	G4double sinTheta = std::sqrt(1.0 - cosTheta*cosTheta);
	G4double phi      = twopi * G4UniformRand();                // twopi from G4PhysicalConstants

	G4ThreeVector dir(
	  sinTheta * std::cos(phi),
	  sinTheta * std::sin(phi),
	  -cosTheta            // negative to point down the –Z axis
	);
	dir = dir.unit();  // just in case
	fParticleGun->SetParticleMomentumDirection(dir);
*/

    fParticleGun->GeneratePrimaryVertex(anEvent);
}

