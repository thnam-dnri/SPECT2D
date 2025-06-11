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
    /// Default was {0, 0, 20*cm}.
    void SetPhantomOffset(const G4ThreeVector& off) { fPhantomOffset = off; }
	const G4ThreeVector& GetPhantomOffset() const { return fPhantomOffset; }
    /// Build the world, phantom, detector, and collimator.
    G4VPhysicalVolume* Construct() override;

    /// Define sensitive detectors (no fields in this example).
    void ConstructSDandField() override;

    /// Return a const reference to the vector of all pinhole centers
    /// (in detector‐coordinates). Use this in EventAction to write (x,y).
    const std::vector<G4ThreeVector>& GetPinholeCenters() const { return fPinholeCenters; }

private:
    void DefineMaterials();   // not used in this version, but kept for future expansion
    void SetupGeometry();     // not used separately, logic is in Construct()

    // --- User‐configurable offset of the phantom (in world coordinates) ---
    G4ThreeVector    fPhantomOffset{0, 0, 20*cm};

    // --- Predefined materials (if you want to cache them) ---
    G4Material* air;
    G4Material* csiTl;
    G4Material* teflon;
    G4Material* lead;
    G4Material* water;

    // --- World volume pointers ---
    G4LogicalVolume* worldLV;
    G4VPhysicalVolume* worldPV;

    // --- Detector pointers (4×4 crystal array) ---
    G4LogicalVolume* detectorLV;
    std::vector<G4LogicalVolume*> crystalLVs;  // will not actually store each crystal separately here

    // --- Phantom pointers ---
    G4LogicalVolume* phantomLV;

    // --- Collimator pointers ---
    G4LogicalVolume* collimatorLV;

    // --- Storage for all 16 pinhole centers (in detector coordinates) ---
    std::vector<G4ThreeVector> fPinholeCenters;
};

#endif  // DETECTORCONSTRUCTION_HH
