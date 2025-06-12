#ifndef DETECTORCONSTRUCTION_HH
#define DETECTORCONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"
#include <vector>

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    DetectorConstruction();
    ~DetectorConstruction() override;

    /// Set an offset to translate the phantom in (x, y, z). 
    void SetPhantomOffset(const G4ThreeVector& off) { fPhantomOffset = off; }
    const G4ThreeVector& GetPhantomOffset() const { return fPhantomOffset; }

    /// Set a rotation angle (around Z) for the phantom
    void SetPhantomRotation(G4double angle);

    /// Build the world, phantom, detector, and collimator.
    G4VPhysicalVolume* Construct() override;

    /// Define sensitive detectors (no fields in this example).
    void ConstructSDandField() override;

    /// Return a const reference to the vector of all pinhole centers
    const std::vector<G4ThreeVector>& GetPinholeCenters() const { return fPinholeCenters; }

private:
    void DefineMaterials();
    void SetupGeometry();

    // --- User‐configurable offset & rotation of the phantom ---
    G4ThreeVector    fPhantomOffset{0, 0, 0*cm};
    G4double         fPhantomRotation{0.0};   ///< rotation around Z

    // --- Predefined materials ---
    G4Material* air;
    G4Material* csiTl;
    G4Material* teflon;
    G4Material* lead;
    G4Material* water;

    // --- World volume pointers ---
    G4LogicalVolume* worldLV;
    G4VPhysicalVolume* worldPV;

    // --- Detector pointers ---
    G4LogicalVolume* detectorLV;
    std::vector<G4LogicalVolume*> crystalLVs;

    // --- Phantom pointers ---
    G4LogicalVolume* phantomLV;

    // --- Collimator pointers ---
    G4LogicalVolume* collimatorLV;

    // --- Storage for all 16 pinhole centers ---
    std::vector<G4ThreeVector> fPinholeCenters;
};

#endif  // DETECTORCONSTRUCTION_HH

