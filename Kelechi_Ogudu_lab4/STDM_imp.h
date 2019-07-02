#ifndef _STDM_IMP_H
#define _STDM_IMP_H

#include <map>
#include <vector>
#include <string>


class STDM_IMP{
private:
  int duration;
  double inputRate;
  std::map<char, std::vector<std::string> > contents;
public:
  /** Base Constructor **/
  STDM_IMP();

  /**
    Function reads the file and returns
    a hashmap - mapping the source to frame
  */

void readFile(const char * fileName);


/**
  Parse file contents
*/
void parseFile(std::vector<char> file_content);


/**
 Get the end time of the last frame
*/
int setDuration(char lastSource);

/**
Get contents
*/
std::map<char, std::vector<std::string> > getContents();

std::vector<std::vector<int> > inputBuffer();

std::string getFlag(int num);

void outputBuffer(std::vector<std::vector<int> > input);

void print();
};

#endif
