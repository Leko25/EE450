//Name: Kelechi Ogudu
//USC_ID: 5413285132

/**
 Main file - This file reads an associated text file containing an Adjacency
            matrix with respective costs for each node. It creates an output file
            <output- + <inputfile> > with shortest path distances from the source
            node and the shortest path from the source node to target nodes
*/

#include <iostream>
//#include <ios::exceptions>
#include <fstream>
#include <cstring>
#include <vector>
#include <limits>
#include "BellManFordImp.h"
#define SOURCE 0
#define NEG_CYCLE 0
#define SOLUTION 1
using namespace std;

/** Reads input file and parses it into a vector
    @param fileName: input text file
    @param numRows: reference to number of rows to be determined
    @return vector<int>: vector containing values within input file
*/
vector<int> readFiles(const char * fileName, int & numRows);

/**
  @param value: character whos value is to be determined
  @return true if the character is [0 - 9], otherwise false
*/
bool match(char value);

/**
 @param fileName: cstring contiaing input file name
 @return a string of the input filename
*/
string getOutFileName(char * fileName);

/*
  Writes the values form the Optimized BellManFord to an output file
  @param g: reference to object
  @param n: number of nodes
*/
void writeToFile(string fileName, BellManFordImp &g, int n);

int main(int argc, char * argv[]){
  vector<int> fileContent;
  int numRows;
  numRows = 0;
  if(argc > 2){
    cout << argc;
    cerr << "Must pass in 1 valid <filename.txt> argument." << endl;
  }
  else{
    try{
      fileContent = readFiles(argv[1], numRows);
      string outFile = getOutFileName(argv[1]);
      BellManFordImp g(fileContent, numRows);
      writeToFile(outFile, g, numRows);
    }
    catch(ifstream::failure e){
      cerr << e.what() << endl;
    }
  }
  return 0;
}

bool match(char value){
  bool isMatch = false;
  switch(value){
    case '0':
      isMatch = true;
    case '1':
      isMatch = true;
    case '2':
      isMatch = true;
    case '3':
      isMatch = true;
    case '4':
      isMatch = true;
    case '5':
      isMatch = true;
    case '6':
      isMatch = true;
    case '7':
      isMatch = true;
    case '8':
      isMatch = true;
    case '9':
      isMatch = true;
    default:
      break;
  }
  return isMatch;
}

vector<int> readFiles(const char * fileName, int & numRows){
  vector<int> fileContent;
  ifstream file;
  file.open(fileName);
  if(file.fail()){
    throw ifstream::failure("Exception opening/reading/closing file");
  }else{
    char value;
    while(!file.eof()){
      value = file.get();
      if(match(value) || value == '-'){
        string cost = "";
        while(!isspace(value)){
          cost += value;
          value = file.get();
        }
        fileContent.push_back(stoi(cost));
      }
      if(value == 'i'){
        int max = numeric_limits<int>::max();
        fileContent.push_back(max);
      }
      if(value == '['){
        int sum = 0;
        while(value != ']'){
          string cost = "";
          value = file.get();
          while(match(value) || value == '-'){
            cost += value;
            value = file.get();
          }
          sum += stoi(cost);
        }
        fileContent.push_back(sum);
      }
      if(value == '\n'){
        numRows++;
      }
    }
  }
  file.close();
  return fileContent;
}

void writeToFile(string fileName, BellManFordImp& g, int n){
  string buffer = "output-";
  fileName = buffer + fileName;
  ofstream file(fileName.c_str(), ios::out);
  vector<bool> flag = g.OptBellManFord(0);
  int iter = g.getEpoch();
  if(flag[NEG_CYCLE] == true){//Negative Cycle Detected
    if(file.is_open()){
      file << "Negative Loop Detected\n";
      vector<int> cycle = g.getNegCycle();
      for(int ii = cycle.size() - 1; ii > 0; ii--){
        file << cycle[ii] << "->";
      }
      file << cycle[0]<< "\n";
      file.close();
    }
  }


  if(flag[SOLUTION] == true){//Solution found
    int * dist = g.getDistanceArr();
    vector<vector<int> > pred = g.getPredecessors(SOURCE);
    if(file.is_open()){
      for(int ii = 0; ii < n - 1; ii++){
        file << dist[ii] << ", ";
      }
      file << dist[n - 1] << "\n";
      for(vector<int> predecessors : pred){
        int cnt = predecessors.size();
        for(vector<int>::reverse_iterator t = predecessors.rbegin(); t != predecessors.rend(); t++){
          cnt--;
          if(cnt != 0){
            file << *t << "->";
          }
        }
        file << predecessors[SOURCE] << "\n";
      }
      file << "Iteration: "<< iter << "\n";
      file.close();
    }
  }
}

string getOutFileName(char * fileName){
  string s(fileName);
  return s;
}
