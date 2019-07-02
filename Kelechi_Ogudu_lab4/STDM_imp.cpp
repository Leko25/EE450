#include "STDM_imp.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <bitset>
#include <queue>


STDM_IMP::STDM_IMP(){
  duration = 0;
  inputRate = 0;
}



void STDM_IMP::readFile(const char * fileName){
  std::ifstream file;
  file.open(fileName);
  if(file.fail()){
    throw std::ifstream::failure("Exception opening/reading/closing file");
  }else{
    std::string value = "";
    std::vector<char> file_content;
    while(!file.eof()){
      char c = file.get();
      if(c != file.eof()){
        file_content.push_back(c);
      }
    }
    file_content.push_back('\n');
    parseFile(file_content);
  }
  file.close();
}

void STDM_IMP::parseFile(std::vector<char> file_content){
  char source;
  for(unsigned int ii = 0; ii < file_content.size(); ii++){
    if(file_content[ii + 1] == ':'){
      source = file_content[ii];
      std::vector<std::string> frames;
      std::string frame = "";
      unsigned int jj;
      for(jj = ii+2; file_content[jj] != '\n'; jj++){
        if(file_content[jj] == ','){
          frames.push_back(frame);
          frame = "";
          jj++;
        }
        if(!isspace(file_content[jj])){
          frame += file_content[jj];
        }
      }
      frames.push_back(frame);
      contents[source] = frames;
    }
  }
  setDuration(source);
}

int STDM_IMP::setDuration(char lastSource){
  std::vector<std::string> frame = contents[lastSource];
  std::string time = frame[frame.size() - 1];
  duration = stoi(time.substr(1));
  return 0;
}

std::map<char, std::vector<std::string> > STDM_IMP::getContents(){
  return contents;
}

std::vector<std::vector<int> > STDM_IMP::inputBuffer(){
  int b[contents.size()][duration + 1] = {};
  std::vector<std::vector<int> > buffer;
  buffer.resize(contents.size());
  int input_num = 0;
  for(auto it = contents.begin(); it != contents.end(); it++){
    std::vector<std::string> frames = it->second;
    for(unsigned int ii = 0; ii < frames.size(); ii++){
      std::string frame = frames[ii];
      std::cout << frame[0] << ", " << frame[1] << std::endl;
      int start = (int)frame[0] - 48;
      int end = (int)frame[1] - 48;
      //std::cout << start << ", " << end << std::endl;
      int packet_num = frame[3] - '0';
      for(int ii = start; ii < end; ii++){
        b[input_num][ii] = packet_num;
      }
      inputRate += ((end - start)) * 1.0/duration;
    }
    input_num++;
  }
  for(unsigned int ii = 0; ii < contents.size(); ii++){
    for(int jj = 0; jj < duration + 1; jj++){
      buffer[ii].push_back(b[ii][jj]);
    }
  }
  for(unsigned int ii = 0; ii < buffer.size(); ii++){
    std::vector<int> frame = buffer[ii];
    for(unsigned int jj = 0; jj < frame.size(); jj++){
      std::cout << frame[jj] << " ";
    }
    std::cout << std::endl;
  }
  std::cout << inputRate << std::endl;
  return buffer;
}

void STDM_IMP::print(){
  for(auto it = contents.begin(); it != contents.end(); it++){
    std::cout << it->first << ": ";
    std::vector<std::string> value = it->second;
    for(std::vector<std::string>::iterator it = value.begin(); it != value.end(); it++){
      std::cout << *it << " ";
    }
    std::cout << std::endl;
  }
}


std::string STDM_IMP::getFlag(int num){
  int addrs = (int)ceil(log(contents.size())/log(2));
  std::string frame = "";
  for(int ii = 0; ii < addrs; ii++){
    frame += std::to_string(num % 2);
    num/=2;
  }
  return frame;
}

void STDM_IMP::outputBuffer(std::vector<std::vector<int> > input){
  std::string start = "1";
  std::string end = "1";
  int outputRate = (int)ceil(inputRate);
  std::queue<std::map<int, int> > buffer;
  for(int jj = 0; jj < duration || !buffer.empty(); jj++){
    std::string frame = "";
    frame += start;
    int count = 0;
    for(unsigned int ii = 0; ii < input.size(); ii++){
      int packet = input[ii].at(jj);
      //std::cout << ii << ": " << packet << std::endl;
      if(packet != 0){
        std::map<int, int> curr;
        curr[ii] = packet;
        buffer.push(curr);
      }
    }
    while(count++ < outputRate){
      if(!buffer.empty()){
        std::map<int, int> val = buffer.front();
        buffer.pop();
        for(auto it = val.begin(); it != val.end(); it++){
          frame += getFlag(it->first);
          char s = (char)(65 + it->first);
          frame += s;
          frame += std::to_string(it->second);
        }
      }
    }
    frame += end;
    if(frame.size() == 2){
      continue;
    }
    std::cout << "(" << jj << ", " << jj + 1 << ")" << "-->" << frame << std::endl;
  }
}
