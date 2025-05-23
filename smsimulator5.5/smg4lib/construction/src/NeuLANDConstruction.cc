
#include "NeuLANDConstruction.hh"
#include "NeuLANDConstructionMessenger.hh"

#include "SimDataManager.hh"
#include "TNeuLANDSimParameter.hh"
#include "TDetectorSimParameter.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include "G4RunManager.hh"

#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4UnitsTable.hh"
#include "G4ios.hh"
#include "G4ThreeVector.hh"

#include "G4NistManager.hh"

#include "G4SDManager.hh"
#include "G4VSensitiveDetector.hh"

#include "globals.hh"

#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "G4SystemOfUnits.hh"//Geant4.10
//______________________________________________________________________________

NeuLANDConstruction::NeuLANDConstruction()
  : fNeuLANDSimParameter(0),
    fPosition(0,0,0),
    fLogicNeut(0), fLogicVeto(0),
    fNeutExist(false), fVetoExist(false), fNeuLANDSD(0)
{
  fNeuLANDConstructionMessenger = new NeuLANDConstructionMessenger();

  fVacuumMaterial = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");
  fNeutMaterial = G4NistManager::Instance()->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
}

//______________________________________________________________________________
NeuLANDConstruction::~NeuLANDConstruction()
{;}
//______________________________________________________________________________
G4VPhysicalVolume* NeuLANDConstruction::Construct()
{
  // Cleanup old geometry
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();

  // -------------------------------------------------
  // World LV
  G4ThreeVector worldSize(1.1* 7.0 *m,
			  1.1* 5.0 *m,
			  1.1*2.*fabs(0.5*fNeuLANDSimParameter->fSize.z()+fNeuLANDSimParameter->fPosition.z())*mm );

  G4Box* solidWorld = new G4Box("World",
				0.5*worldSize.x(),
				0.5*worldSize.y(),
				0.5*worldSize.z());
  G4LogicalVolume *LogicWorld = new G4LogicalVolume(solidWorld,fVacuumMaterial,"World");

  // World PV
  G4ThreeVector worldPos(0,0,0);
  G4VPhysicalVolume* world = new G4PVPlacement(0,
					       worldPos,
					       LogicWorld,
					       "World",
					       0,
					       false,
					       0);
  LogicWorld->SetVisAttributes(G4VisAttributes::Invisible);
  //-------------------------------------------------------
  // NeuLAND
  ConstructSub();
  PutNeuLAND(LogicWorld);

  return world;
}
//______________________________________________________________________________
G4LogicalVolume* NeuLANDConstruction::ConstructSub()
{
  G4cout<< "Creating NeuLAND"<<G4endl;

  SimDataManager *sman = SimDataManager::GetSimDataManager();
  fNeuLANDSimParameter = (TNeuLANDSimParameter*)sman->FindParameter("NeuLANDParameter");
  if (fNeuLANDSimParameter==0) {
    fNeuLANDSimParameter = new TNeuLANDSimParameter("NeuLANDParameter");
    sman->AddParameter(fNeuLANDSimParameter);
  }

  // define logical volumes
  G4Box* solidNeut = new G4Box("NeutronDetector",
			       fNeuLANDSimParameter->fNeutSize.x()*0.5*mm,
			       fNeuLANDSimParameter->fNeutSize.y()*0.5*mm,
			       fNeuLANDSimParameter->fNeutSize.z()*0.5*mm);
  fLogicNeut   = new G4LogicalVolume(solidNeut,fNeutMaterial,"NeutronDetector");


  G4Box* solidVeto = new G4Box("VetoDetector",
			       fNeuLANDSimParameter->fVetoSize.x()*0.5*mm,
			       fNeuLANDSimParameter->fVetoSize.y()*0.5*mm,
			       fNeuLANDSimParameter->fVetoSize.z()*0.5*mm);
  fLogicVeto   = new G4LogicalVolume(solidVeto,fNeutMaterial,"VetoDetector");


  if (fNeuLANDSimParameter->fNeutNum>0)
    fLogicNeut->SetVisAttributes(new G4VisAttributes(G4Colour::Magenta()));

  if (fNeuLANDSimParameter->fVetoNum>0)
    fLogicVeto->SetVisAttributes(new G4VisAttributes(G4Colour::Magenta()));

  return 0;
}
//______________________________________________________________________________
G4LogicalVolume* NeuLANDConstruction::PutNeuLAND(G4LogicalVolume* ExpHall_log)
{

  fPosition = G4ThreeVector(fNeuLANDSimParameter->fPosition.x()*mm,
			    fNeuLANDSimParameter->fPosition.y()*mm,
			    fNeuLANDSimParameter->fPosition.z()*mm);


  std::map<int, TDetectorSimParameter> ParaMap 
    = fNeuLANDSimParameter->fNeuLANDDetectorParameterMap;
  std::map<int,TDetectorSimParameter>::iterator it = ParaMap.begin();
  while (it != ParaMap.end()){
    TDetectorSimParameter para = it->second;

    G4ThreeVector detPos(para.fPosition.x()*mm + fPosition.x(),
			 para.fPosition.y()*mm + fPosition.y(), 
			 para.fPosition.z()*mm + fPosition.z());

    // Rotation
    G4RotationMatrix RotMat = G4RotationMatrix();
    RotMat.rotateX(para.fAngle.x()*deg);
    RotMat.rotateY(para.fAngle.y()*deg);
    RotMat.rotateZ(para.fAngle.z()*deg);

    G4LogicalVolume *lv=0;
    if      (para.fDetectorType=="NeuLAND") {
      lv = fLogicNeut;
      fNeutExist = true;
    }else if (para.fDetectorType=="NeuLANDVeto") {
      lv = fLogicVeto;
      fVetoExist = true;
    }else{
      G4cout<<"strange DetectorType="
	    <<para.fDetectorType
	    <<G4endl;
    }

    new G4PVPlacement(G4Transform3D(RotMat, detPos),
		      lv,
		      (para.fDetectorType).Data(),
		      ExpHall_log,
		      false,// Many <- should be checked
		      para.fID);// Copy num <- should be checked
    ++it;
  }

  return 0;
}
//______________________________________________________________________________
