// This code implementation is the intellectual property of
// the RD44 GEANT4 collaboration.
//
// By copying, distributing or modifying the Program (or any work
// based on the Program) you indicate your acceptance of this statement,
// and all its terms.
//
// $Id: ExN03RunAction.hh,v 2.3 1998/10/09 15:48:01 japost Exp $
// GEANT4 tag $Name: geant4-00 $
//
// 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#ifndef ExN03RunAction_h
#define ExN03RunAction_h 1

#include "G4UserRunAction.hh"
#include "globals.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

class G4Run;

class ExN03RunAction : public G4UserRunAction
{
  public:
    ExN03RunAction();
   ~ExN03RunAction();

  public:
    void BeginOfRunAction(G4Run*);
    void EndOfRunAction(G4Run*);

  private:
    G4int runIDcounter;
};

#endif
