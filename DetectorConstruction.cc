#include "DetectorConstruction.hh"
#include "SensitiveDetector.hh"

#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Ellipsoid.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SubtractionSolid.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

DetectorConstruction::DetectorConstruction()
 : G4VUserDetectorConstruction(),
   air(nullptr),
   csiTl(nullptr),
   teflon(nullptr),
   lead(nullptr),
   water(nullptr),
   worldLV(nullptr),
   worldPV(nullptr),
   detectorLV(nullptr),
   phantomLV(nullptr),
   collimatorLV(nullptr)
{
    // (Optional) You could call DefineMaterials() here if you wanted to cache materials.
}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume* DetectorConstruction::Construct() {
    // ------------------------------------------------------------
    // User-defined parameters for detector geometry
    // ------------------------------------------------------------
    const G4double kZOffset             = -10 * cm;
    const G4double kCrystalThickness    = 30 * mm;
    const G4double kReflectorThickness  = 30 * mm;
    const G4double kCollimatorThickness = 5  * cm;
//    const G4double kPinholeRadius       = 0.84 * mm; // radius for 1.68 mm diameter
    const G4double kPinholeRadius       = 1 * mm; // radius for 2 mm diameter
    // ------------------------------------------------------------
    // 1) Define / fetch materials via NIST database
    // ------------------------------------------------------------
    G4NistManager* nist = G4NistManager::Instance();
    air    = nist->FindOrBuildMaterial("G4_AIR");
    csiTl  = nist->FindOrBuildMaterial("G4_CESIUM_IODIDE");
    teflon = nist->FindOrBuildMaterial("G4_TEFLON");
    lead   = nist->FindOrBuildMaterial("G4_Pb");
    water  = nist->FindOrBuildMaterial("G4_WATER");

    // ------------------------------------------------------------
    // 2) World: a 30 cm × 30 cm × 30 cm box of air
    // ------------------------------------------------------------
    G4double halfWorld = 15 * cm;
    G4Box* worldS = new G4Box("World", halfWorld, halfWorld, halfWorld);
    worldLV = new G4LogicalVolume(worldS, air, "WorldLV");
    worldPV = new G4PVPlacement(
        nullptr,
        G4ThreeVector(),
        worldLV,
        "WorldPV",
        nullptr,
        false,
        0
    );

    // ------------------------------------------------------------
    // 3) Multi‐organ phantom: body + thyroid at fPhantomOffset
    // ------------------------------------------------------------
    // 3a) Body (ellipsoid)
    G4double bodyX = 3.5*cm, bodyY = 1.5*cm, bodyZ = 1.5*cm;
    G4Ellipsoid* bodyS = new G4Ellipsoid("BodyS", bodyX, bodyY, bodyZ);
    G4LogicalVolume* bodyLV = new G4LogicalVolume(bodyS, water, "BodyLV");
    new G4PVPlacement(
        nullptr,
        fPhantomOffset,
        bodyLV,
        "BodyPV",
        worldLV,
        false,
        0
    );
    G4VisAttributes* bodyVis = new G4VisAttributes(G4Colour(0.8,0.8,0.8,0.2));
    bodyVis->SetForceSolid(true);
    bodyLV->SetVisAttributes(bodyVis);

    // 3b) Thyroid (smaller ellipsoid)
    G4double thyX=0.4*cm, thyY=0.4*cm, thyZ=0.2*cm;
    G4Ellipsoid* thyroidS = new G4Ellipsoid("ThyroidS", thyX, thyY, thyZ);
    G4LogicalVolume* thyroidLV = new G4LogicalVolume(thyroidS, water, "ThyroidLV");
    new G4PVPlacement(
        nullptr,
        fPhantomOffset,
        thyroidLV,
        "ThyroidPV",
        worldLV,
        false,
        0
    );
    G4VisAttributes* thyVis = new G4VisAttributes(G4Colour(1.0,0.0,0.0,1.0));
    thyVis->SetForceSolid(true);
    thyroidLV->SetVisAttributes(thyVis);

    // 3c) Test box inside body
    G4Box* testBoxS = new G4Box("TestBox",0.25*cm,0.25*cm,0.5*cm);
    G4LogicalVolume* testBoxLV = new G4LogicalVolume(testBoxS, water, "TestBoxLV");
    G4ThreeVector boxGlobalPos = fPhantomOffset + G4ThreeVector(0.8*cm,0,0);
    new G4PVPlacement(
        nullptr,
        boxGlobalPos,
        testBoxLV,
        "TestBoxPV",
        worldLV,
        false,
        0
    );
    G4VisAttributes* boxVis = new G4VisAttributes(G4Colour(0.2,0.2,1.0,1.0));
    boxVis->SetForceSolid(true);
    testBoxLV->SetVisAttributes(boxVis);

    // ------------------------------------------------------------
    // 4) Detector array: 4×4 CsI(Tl) crystals + Teflon reflector
    // ------------------------------------------------------------
    const G4int nX=4, nY=4;
    const G4double cX=3.16*mm, cY=3.16*mm;
    const G4double gap=0.2*mm;
    const G4double detX = nX*(cX+gap)-gap;
    const G4double detY = nY*(cY+gap)-gap;

    // 4a) Reflector
    G4VSolid* reflS = new G4Box("ReflectorFull", detX/2, detY/2, kReflectorThickness/2);
    G4Box* holeS = new G4Box("Hole", cX/2, cY/2, kCrystalThickness/2);
    for(int i=0;i<nX;++i) for(int j=0;j<nY;++j) {
        G4double x=-detX/2 + cX/2 + i*(cX+gap);
        G4double y=-detY/2 + cY/2 + j*(cY+gap);
        reflS=new G4SubtractionSolid("ReflectorSub", reflS, holeS, nullptr, G4ThreeVector(x,y,0));
    }
    G4LogicalVolume* reflLV = new G4LogicalVolume(reflS, teflon, "ReflectorLV");
    new G4PVPlacement(
        nullptr,
        G4ThreeVector(0,0,-kReflectorThickness/2 + kZOffset),
        reflLV,
        "ReflectorPV",
        worldLV,
        false,
        0
    );
    G4VisAttributes* reflVis = new G4VisAttributes(G4Colour(1,1,1,0.5)); reflVis->SetForceSolid(true);
    reflLV->SetVisAttributes(reflVis);

    // ------------------------------------------------------------
    // 5) Collimator: lead block with pinholes
    // ------------------------------------------------------------
    G4Box* colBase = new G4Box("CollimatorBase", detX/2+2.5*cm, detY/2+2.5*cm, kCollimatorThickness/2);
    G4Tubs* pinS = new G4Tubs("Pinhole", 0, kPinholeRadius, kCollimatorThickness/2+1*mm, 0, 360*deg);
    G4VSolid* colS=colBase;
    fPinholeCenters.clear();
    for(int i=0;i<nX;++i) for(int j=0;j<nY;++j) {
        G4double x=-detX/2 + cX/2 + i*(cX+gap);
        G4double y=-detY/2 + cY/2 + j*(cY+gap);
        fPinholeCenters.emplace_back(x,y,0);
        colS=new G4SubtractionSolid("CollimatorHoles", colS, pinS, nullptr, G4ThreeVector(x,y,0));
    }
    G4LogicalVolume* colLV = new G4LogicalVolume(colS, lead, "CollimatorLV");
    new G4PVPlacement(
        nullptr,
        G4ThreeVector(0,0,kCollimatorThickness/2 + kZOffset),
        colLV,
        "CollimatorPV",
        worldLV,
        false,
        0
    );
    G4VisAttributes* colVis = new G4VisAttributes(G4Colour(0.5,0.5,0.5,1)); colVis->SetForceSolid(true);
    colLV->SetVisAttributes(colVis);

    // ------------------------------------------------------------
    // 6) Crystals: place CsI(Tl) volumes
    // ------------------------------------------------------------
    G4LogicalVolume* crystalLV = new G4LogicalVolume(holeS, csiTl, "CrystalLV");
    for(int i=0;i<nX;++i) for(int j=0;j<nY;++j) {
        G4double x=-detX/2 + cX/2 + i*(cX+gap);
        G4double y=-detY/2 + cY/2 + j*(cY+gap);
        new G4PVPlacement(
            nullptr,
            G4ThreeVector(x,y,-kCrystalThickness/2 + kZOffset),
            crystalLV,
            "CrystalPV",
            worldLV,
            false,
            i*nY+j
        );
    }
    G4VisAttributes* cryVis = new G4VisAttributes(G4Colour(0,1,0,1)); cryVis->SetForceSolid(true);
    crystalLV->SetVisAttributes(cryVis);

    return worldPV;
}

void DetectorConstruction::ConstructSDandField() {
    G4SDManager* sdMan = G4SDManager::GetSDMpointer();
    SensitiveDetector* sd = new SensitiveDetector("CrystalSD", "CrystalSD_Hits");
    sdMan->AddNewDetector(sd);
    SetSensitiveDetector("CrystalLV", sd);
}

