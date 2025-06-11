#ifndef EVENTACTION_HH
#define EVENTACTION_HH

#include "G4UserEventAction.hh"
#include "globals.hh"

class RunAction;
class DetectorConstruction;

class EventAction : public G4UserEventAction
{
public:
    EventAction(RunAction* runAction, DetectorConstruction* detCon);
    ~EventAction() override;

    virtual void BeginOfEventAction(const G4Event*) override;
    virtual void EndOfEventAction(const G4Event*) override;

private:
    RunAction*            fRunAction;   // (unused for ROOT I/O itself)
    DetectorConstruction* fDetCon;      // so we can get pinhole centers each event

    G4double fEdep[16];   // energy deposit in each of 16 crystals
    G4double fXpos[16];   // pinhole X coord (in mm) for each of 16 holes
    G4double fYpos[16];   // pinhole Y coord (in mm) for each of 16 holes
};

#endif  // EVENTACTION_HH
