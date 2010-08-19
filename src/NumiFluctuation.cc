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
// $Id: NumiFluctuation.cc,v 1.1.2.1 2010/08/19 19:50:54 minervacvs Exp $
// GEANT4 tag $Name:  $
//
// -------------------------------------------------------------------
//
// GEANT4 Class file
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
// 28-12-02 add method Dispersion (V.Ivanchenko)
// 07-02-03 change signature (V.Ivanchenko)
// 13-02-03 Add name (V.Ivanchenko)
// 16-10-03 Changed interface to Initialisation (V.Ivanchenko)
// 07-11-03 Fix problem of rounding of double in NumiFluctuations
// 06-02-04 Add control on big sigma > 2*meanLoss (V.Ivanchenko)
// 26-04-04 Comment out the case of very small step (V.Ivanchenko)
// 07-02-05 define problim = 5.e-3 (mma)
// 03-05-05 conditions of Gaussian fluctuation changed (bugfix)
//          + smearing for very small loss (L.Urban)
// 03-10-05 energy dependent rate -> cut dependence of the
//          distribution is much weaker (L.Urban)
// 17-10-05 correction for very small loss (L.Urban)
// 20-03-07 'GLANDZ' part rewritten completely, no 'very small loss'
//          regime any more (L.Urban)
// 03-04-07 correction to get better width of eloss distr.(L.Urban)
// 13-07-07 add protection for very small step or low-density material (VI)
//          

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "NumiFluctuation.hh"
#include "Randomize.hh"
#include "G4Poisson.hh"
#include "G4Step.hh"
#include "G4Material.hh"
#include "G4DynamicParticle.hh"
#include "G4ParticleDefinition.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

using namespace std;

NumiFluctuation::NumiFluctuation(const G4String& nam)
 :G4VEmFluctuationModel(nam),
  particle(0),
  minNumberInteractionsBohr(10.0),
  theBohrBeta2(50.0*keV/proton_mass_c2),
  minLoss(10.*eV),
  nmaxCont1(4.),
  nmaxCont2(16.)
{
  lastMaterial = 0;
  facwidth     = 1.000/keV;
  oldloss = 0.;
  samestep = 0.;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

NumiFluctuation::~NumiFluctuation()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void NumiFluctuation::InitialiseMe(const G4ParticleDefinition* part)
{
  particle       = part;
  particleMass   = part->GetPDGMass();
  G4double q     = part->GetPDGCharge()/eplus;
  chargeSquare   = q*q;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4double NumiFluctuation::SampleFluctuations(const G4Material* material,
                                                const G4DynamicParticle* dp,
				                      G4double& tmax,
					              G4double& length,
                                                      G4double& meanLoss)
{

   G4cout << "I am here" << G4endl;
// Calculate actual loss from the mean loss.
// The model used to get the fluctuations is essentially the same
// as in Glandz in Geant3 (Cern program library W5013, phys332).
// L. Urban et al. NIM A362, p.416 (1995) and Geant4 Physics Reference Manual

  // shortcut for very very small loss (out of validity of the model)
  //
  if (meanLoss < minLoss)
  { 
    oldloss = meanLoss;
    return meanLoss;
  }

  if(!particle) InitialiseMe(dp->GetDefinition());

  G4double tau   = dp->GetKineticEnergy()/particleMass;
  G4double gam   = tau + 1.0;
  G4double gam2  = gam*gam;
  G4double beta2 = tau*(tau + 2.0)/gam2;

  G4double loss(0.), siga(0.);
  
  // Gaussian regime
  // for heavy particles only and conditions
  // for Gauusian fluct. has been changed 
  //
  if ((particleMass > electron_mass_c2) &&
      (meanLoss >= minNumberInteractionsBohr*tmax))
  {
    G4double massrate = electron_mass_c2/particleMass ;
    G4double tmaxkine = 2.*electron_mass_c2*beta2*gam2/
                        (1.+massrate*(2.*gam+massrate)) ;
    if (tmaxkine <= 2.*tmax)   
    {
      electronDensity = material->GetElectronDensity();
      siga  = (1.0/beta2 - 0.5) * twopi_mc2_rcl2 * tmax * length
                                * electronDensity * chargeSquare;
      siga = sqrt(siga);
      G4double twomeanLoss = meanLoss + meanLoss;
      if (twomeanLoss < siga) {
        G4double x;
        do {
          loss = twomeanLoss*G4UniformRand();
       	  x = (loss - meanLoss)/siga;
        } while (1.0 - 0.5*x*x < G4UniformRand());
      } else {
        do {
          loss = G4RandGauss::shoot(meanLoss,siga);
        } while (loss < 0. || loss > twomeanLoss);
      }
      return loss;
    }
  }

  // Glandz regime : initialisation
  //
  if (material != lastMaterial) {
    f1Fluct      = material->GetIonisation()->GetF1fluct();
    f2Fluct      = material->GetIonisation()->GetF2fluct();
    e1Fluct      = material->GetIonisation()->GetEnergy1fluct();
    e2Fluct      = material->GetIonisation()->GetEnergy2fluct();
    e1LogFluct   = material->GetIonisation()->GetLogEnergy1fluct();
    e2LogFluct   = material->GetIonisation()->GetLogEnergy2fluct();
    ipotFluct    = material->GetIonisation()->GetMeanExcitationEnergy();
    ipotLogFluct = material->GetIonisation()->GetLogMeanExcEnergy();
    e0 = material->GetIonisation()->GetEnergy0fluct();
    lastMaterial = material;
  }

  // very small step or low-density material
  if(tmax <= e0) return meanLoss;

  G4double a1 = 0. , a2 = 0., a3 = 0. ;

  // correction to get better width even using stepmax
  if(abs(meanLoss- oldloss) < 1.*eV)
    samestep += 1;
  else
    samestep = 1.;
  oldloss = meanLoss;
  G4double width = 1.+samestep*facwidth*meanLoss;
  if(width > 4.50) width = 4.50;
  e1 = width*e1Fluct;
  e2 = width*e2Fluct;

  // cut and material dependent rate 
  G4double rate = 1.0;
  if(tmax > ipotFluct) {
    G4double w2 = log(2.*electron_mass_c2*beta2*gam2)-beta2;

    if(w2 > ipotLogFluct && w2 > e2LogFluct) {

      rate = 0.03+0.23*log(log(tmax/ipotFluct));
      G4double C = meanLoss*(1.-rate)/(w2-ipotLogFluct);
      a1 = C*f1Fluct*(w2-e1LogFluct)/e1;
      a2 = C*f2Fluct*(w2-e2LogFluct)/e2;
    }
  }

  G4double w1 = tmax/e0;
  if(tmax > e0) 
    a3 = rate*meanLoss*(tmax-e0)/(e0*tmax*log(w1));

  //'nearly' Gaussian fluctuation if a1>nmaxCont2&&a2>nmaxCont2&&a3>nmaxCont2  
  G4double emean = 0.;
  G4double sig2e = 0., sige = 0.;
  G4double p1 = 0., p2 = 0., p3 = 0.;
 
  // excitation of type 1
  if(a1 > nmaxCont2)
  {
    emean += a1*e1;
    sig2e += a1*e1*e1;
  }
  else if(a1 > 0.)
  {
    p1 = G4double(G4Poisson(a1));
    loss += p1*e1;
    if(p1 > 0.) 
      loss += (1.-2.*G4UniformRand())*e1;
  }

  // excitation of type 2
  if(a2 > nmaxCont2)
  {
    emean += a2*e2;
    sig2e += a2*e2*e2;
  }
  else if(a2 > 0.)
  {
    p2 = G4double(G4Poisson(a2));
    loss += p2*e2;
    if(p2 > 0.) 
      loss += (1.-2.*G4UniformRand())*e2;
  }

  // ionisation 
  G4double lossc = 0.;
  if(a3 > 0.)
  {
    p3 = a3;
    G4double alfa = 1.;
    if(a3 > nmaxCont2)
    {
       alfa            = w1*(nmaxCont2+a3)/(w1*nmaxCont2+a3);
       G4double alfa1  = alfa*log(alfa)/(alfa-1.);
       G4double namean = a3*w1*(alfa-1.)/((w1-1.)*alfa);
       emean          += namean*e0*alfa1;
       sig2e          += e0*e0*namean*(alfa-alfa1*alfa1);
       p3              = a3-namean;
    }

    G4double w2 = alfa*e0;
    G4double w  = (tmax-w2)/tmax;
    G4int nb = G4Poisson(p3);
    if(nb > 0)
      for (G4int k=0; k<nb; k++) lossc += w2/(1.-w*G4UniformRand());
  }

  if(emean > 0.)
  {
    sige   = sqrt(sig2e);
    loss += max(0.,G4RandGauss::shoot(emean,sige));
  }

  loss += lossc;

  return loss;

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


G4double NumiFluctuation::Dispersion(
                          const G4Material* material,
                          const G4DynamicParticle* dp,
 				G4double& tmax,
			        G4double& length)
{
  if(!particle) InitialiseMe(dp->GetDefinition());

  electronDensity = material->GetElectronDensity();

  G4double gam   = (dp->GetKineticEnergy())/particleMass + 1.0;
  G4double beta2 = 1.0 - 1.0/(gam*gam);

  G4double siga  = (1.0/beta2 - 0.5) * twopi_mc2_rcl2 * tmax * length
                 * electronDensity * chargeSquare;

  return siga;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
