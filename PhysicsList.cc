#include "PhysicsList.hh"

// Essential physics constructors
#include "G4EmStandardPhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4SystemOfUnits.hh"
PhysicsList::PhysicsList()
{
    // Electromagnetic interactions (e.g., gamma absorption/scattering)
    RegisterPhysics(new G4EmStandardPhysics());

    // Generic decays (e.g., beta decay, if present)
    //RegisterPhysics(new G4DecayPhysics());

    // Radioactive decay (e.g., I-131 gamma emissions)
    RegisterPhysics(new G4RadioactiveDecayPhysics());
}

PhysicsList::~PhysicsList() {}

void PhysicsList::SetCuts()
{
    //SetCutsWithDefault();  // Use default production cuts
    SetCutValue(0.5 * mm, "gamma");     // or 1 mm
    SetCutValue(0.5 * mm, "e-");
    SetCutValue(0.5 * mm, "e+");
}