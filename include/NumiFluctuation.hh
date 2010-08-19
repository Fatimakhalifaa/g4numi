//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: NumiFluctuation.hh,v 1.1.2.1 2010/08/19 19:50:54 minervacvs Exp $
// GEANT4 tag $Name:  $
//
// -------------------------------------------------------------------
//
// GEANT4 Class header file
//
//
// File name:     NumiFluctuation
//
// Author:        Vladimir Ivanchenko
//
// Creation date: 03.01.2002
//
// Modifications:
//
// 09-12-02 remove warnings (V.Ivanchenko)
// 28-12-02 add method Dispersion (V.Ivanchenko)
// 07-02-03 change signature (V.Ivanchenko)
// 13-02-03 Add name (V.Ivanchenko)
// 16-10-03 Changed interface to Initialisation (V.Ivanchenko)
// 07-02-05 define problim = 5.e-3 (mma)
//
// Class Description:
//
// Implementation of energy loss fluctuations

// -------------------------------------------------------------------
//

#ifndef NumiFluctuation_h
#define NumiFluctuation_h 1


#include "G4VEmFluctuationModel.hh"

class NumiFluctuation : public G4VEmFluctuationModel
{

public:

  NumiFluctuation(const G4String& nam = "NumiFluc");

  virtual ~NumiFluctuation();

  G4double SampleFluctuations(const G4Material*,
                          const G4DynamicParticle*,
 				G4double&,
                                G4double&,
                                G4double&);

  G4double Dispersion(    const G4Material*,
                          const G4DynamicParticle*,
 				G4double&,
                                G4double&);

  void InitialiseMe(const G4ParticleDefinition*);

protected:

private:

  // hide assignment operator
  NumiFluctuation & operator=(const  NumiFluctuation &right);
  NumiFluctuation(const  NumiFluctuation&);

  const G4ParticleDefinition* particle;
  const G4Material* lastMaterial;

  G4double particleMass;
  G4double chargeSquare;

  // data members to speed up the fluctuation calculation
  G4double ipotFluct;
  G4double electronDensity;
  
  G4double f1Fluct;
  G4double f2Fluct;
  G4double e1Fluct;
  G4double e2Fluct;
  G4double e1LogFluct;
  G4double e2LogFluct;
  G4double ipotLogFluct;
  G4double e0;

  G4double facwidth;
  G4double oldloss;
  G4double samestep;
  G4double e1,e2;

  G4double minNumberInteractionsBohr;
  G4double theBohrBeta2;
  G4double minLoss;
  G4double nmaxCont1;
  G4double nmaxCont2;

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

