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


void tokenize(const string& str, vector<string>& tokens, const string& delimiters = ",") {
  // Skip delimiters at beginning.
  string::size_type lastPos = str.find_first_not_of(delimiters, 0);

  // Find first non-delimiter.
  string::size_type pos = str.find_first_of(delimiters, lastPos);

  while (string::npos != pos || string::npos != lastPos) {
    // Found a token, add it to the vector.
    tokens.push_back(str.substr(lastPos, pos - lastPos));

    // Skip delimiters.
    lastPos = str.find_first_not_of(delimiters, pos);

    // Find next non-delimiter.
    pos = str.find_first_of(delimiters, lastPos);
  }
}

int convert(const string& file_name) {
  cout << "Convert file " << file_name << ".csv\n";
  cout << "Root file will be created " << file_name << ".root\n";
  
  std::ifstream inputfile ((file_name + ".csv").c_str());
  
  TString root_name(file_name + ".root");
  TFile root_file(root_name, "RECREATE");
  TTree *tree = new TTree("data", "data for challenge");
 
  Float_t variables[60];
  std::vector<string> variables_name;
  Int_t ids;
  string str;
  char sep;
  
  inputfile >> str;
  tokenize(str, variables_name);
  tree->Branch("id", &ids);
  
  std::vector<string>::iterator it_n=variables_name.begin();
  it_n ++;
  int count_vars = 0;
  for (int i=0; it_n != variables_name.end(); it_n++, i++){
    variables[i] = 0.;
    count_vars ++;
    tree->Branch((*it_n).c_str(), &variables[i]);
  }
  inputfile >> ids;
  while (! inputfile.eof()) { 
    for (int i = 0; i < count_vars; i++){
      inputfile >> sep >> variables[i];
      if (sep != ',') {
        cout << 'Error in separation';
        return -1;
      }
    }
    tree->Fill();
    inputfile >> ids;
  }
  tree->Write();
  root_file.Close();
  inputfile.close();
  return 0;
}
