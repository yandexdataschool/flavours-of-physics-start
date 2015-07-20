#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TROOT.h"

#if not defined(__CINT__) || defined(__MAKECINT__)
needs to be included when makecint runs (ACLIC)
#include "TMVA/Factory.h"
#include "TMVA/Tools.h"
#endif


void TMVAClassification()
{
   TMVA::Tools::Instance();

   std::cout << "==> Start TMVAClassification" << std::endl;
   TString outfileName( "TMVA.root" );

   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );
   TMVA::Factory *factory = new TMVA::Factory( "TMVAClassification", outputFile,
                                               "AnalysisType=Classification" );
   factory->AddVariable("LifeTime", 'F');
   factory->AddVariable("FlightDistance", 'F');
   factory->AddVariable("pt", 'F');
   
   TString fname = "../tau_data/training.root";
   TFile *input = TFile::Open( fname );
   
   TTree *tree     = (TTree*)input->Get("data");
   
   factory->AddTree(tree, "Signal", 1., "signal == 1", "Training");
   factory->AddTree(tree, "Signal", 1., "signal == 1", "Test");
   factory->AddTree(tree, "Background", 1., "signal == 0", "Training");
   factory->AddTree(tree, "Background", 1., "signal == 0", "Test");
   
   // gradient boosting training
   factory->BookMethod(TMVA::Types::kBDT, "GBDT",
                       "NTrees=40:BoostType=Grad:Shrinkage=0.01:MaxDepth=7:UseNvars=6:nCuts=20:MinNodeSize=10");
   factory->TrainAllMethods();
   input->Close();
   outputFile->Close();

   delete factory;
}


void TMVAPredict()
{
  std::ofstream outfile ("baseline_c.csv");
  outfile << "id,prediction\n";

  TMVA::Tools::Instance();

  std::cout << "==> Start TMVAPredict" << std::endl;
  TMVA::Reader *reader = new TMVA::Reader( "!Color:!Silent" );  
  string variables_name[3] = {"LifeTime",
                           "FlightDistance",
                           "pt"}
  Float_t variables[3];
  for (int i=0; i < 3; i++){
    reader->AddVariable(variables_name[i].c_str(), &variables[i]);
    variables[i] = 0.0;
  }

  TString dir    = "weights/";
  TString prefix = "TMVAClassification";
  TString method_name = "GBDT";
  TString weightfile = dir + prefix + TString("_") + method_name + TString(".weights.xml");
  reader->BookMVA( method_name, weightfile ); 

  TFile *input(0);
  input = TFile::Open("../tau_data/test.root");
  TTree* tree = (TTree*)input->Get("data");
  
  Int_t ids;
  Float_t prediction;
  tree->SetBranchAddress("id", &ids);

  for (int i=0; i < 3; i++){
    tree->SetBranchAddress(variables_name[i].c_str(), &variables[i]);
  }
 
  for (Long64_t ievt=0; ievt < tree->GetEntries(); ievt++) {
    tree->GetEntry(ievt);
    prediction = reader->EvaluateMVA(method_name);
    outfile << ids << "," << (prediction + 1.) / 2. << "\n";
  }

  outfile.close();
  input->Close();
  delete reader;
}


int tmva()
{
  TMVAClassification();
  cout << "Classifier have been trained\n";
  TMVAPredict();
  cout << "Submission is ready: baseline_c.csv; send it\n";
  return 0;
}