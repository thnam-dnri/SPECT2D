#ifndef RUNACTION_HH
#define RUNACTION_HH

#include "G4UserRunAction.hh"
#include "G4Run.hh"
#include "G4String.hh"
#include "G4RootAnalysisManager.hh"

class RunAction : public G4UserRunAction {
public:
    /// Constructor: provide desired output ROOT filename (including path)
    explicit RunAction(const G4String& outFileName);
    ~RunAction() override;

    // Called at start of each run
    void BeginOfRunAction(const G4Run* run) override;
    // Called at end of each run
    void EndOfRunAction(const G4Run* run) override;

private:
    G4RootAnalysisManager* fAnalysisManager;  ///< pointer to analysis manager
    G4String               fOutFileName;      ///< ROOT file name to open
};

#endif // RUNACTION_HH

