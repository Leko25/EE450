#include "STDM_imp.h"
#include <iostream>
#include <fstream>
#include <vector>



int main(int argc, char * argv[]){
  try{
    STDM_IMP STDM;
    STDM.readFile(argv[1]);
    std::vector<std::vector<int> >  input = STDM.inputBuffer();
    STDM.outputBuffer(input);
  }
  catch(std::ifstream::failure e){
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
