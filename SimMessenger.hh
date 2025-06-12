// SimMessenger.hh
#ifndef SIM_MESSENGER_H
#define SIM_MESSENGER_H

#include "G4UImessenger.hh"
#include "G4UIcmdWith3Vector.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4ThreeVector.hh"

class DetectorConstruction;
class PrimaryGeneratorAction;

class SimMessenger : public G4UImessenger {
public:
  SimMessenger(DetectorConstruction* det, PrimaryGeneratorAction* gen);
  ~SimMessenger() override;

  void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
  DetectorConstruction*        fDetector;
  PrimaryGeneratorAction*      fGenerator;
  G4UIcmdWith3Vector*          fOffsetCmd;
  G4UIcmdWithADoubleAndUnit*   fRotationCmd;  // new rotation command
};

#endif // SIM_MESSENGER_H

