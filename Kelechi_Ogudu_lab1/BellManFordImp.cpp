//Name: Kelechi Ogudu
//USC_ID: 5413285132

#include <iostream>
#include <vector>
#include <cassert>
#include <limits>
#include <unordered_set>
#include "BellManFordImp.h"
using namespace std;

BellManFordImp::BellManFordImp(vector<int>& vec, int numRows){
  assert(numRows != 0);
  V = numRows;
  d = new int[V];
  pd = new int[V];
  ns = new int[V];
  pi = new int[V];
  epoch = 0;
  g.resize(V);
  int incrementor = 0;
  int inf = numeric_limits<int>::max();
  for(int ii = 0; ii < V; ii++){
    for(int jj = 0; jj < V; jj++){
      if(vec[incrementor] != inf && vec[incrementor] != 0){
        Node n(jj, vec[incrementor]);
        g[ii].push_back(n);
      }
      incrementor++;
    }
  }
  vec.clear(); //Deallocate vector resource
}

BellManFordImp::~BellManFordImp(){ //Deallocate
  delete [] d;
  delete [] pd;
  delete [] ns;
  delete [] pi;
}

vector<bool> BellManFordImp::OptBellManFord(int s){
  int inf = numeric_limits<int>::max();
  /**
   if (flag[0] == true Negative cycle present)
   else if (flag[1] == true Solution found)
  */
  vector<bool> flag;
  flag.push_back(false);
  flag.push_back(false);
  for(int v = 0; v < V; v++){
    d[v] = pd[v] = pi[v] = inf;
    ns[v] = 0;
  }
  d[s] = 0;
  int source = s;
  unordered_set<int> labeled_old;
  unordered_set<int> labeled_new;
  int min_d_old, min_d_new, min_v_old, min_v_new;

  for(epoch = 1; epoch <= (2 * V); epoch++){
    ns[source] += 1;
    min_d_old = inf;
    cycle.push_back(source); //push in cycle nodes
    if(ns[source] > 2 || d[s] < 0){
      cycleStart = source;
      flag[0] = true;
      break;
    }
    for(Node v : g[source]){
      pd[v.node] = d[v.node];
      if(d[v.node] > d[source] + v.wt){
        d[v.node] = d[source] + v.wt;
        pi[v.node] = source;
      }
      if(pd[v.node] > d[v.node]){
        labeled_new.insert(v.node);
        labeled_old.erase(v.node);
        min_d_new = d[v.node];
        min_v_new = v.node;
      }
      else{
        min_d_new = inf;
      }
      if(min_d_new < min_d_old){
        min_d_old = d[v.node];
        min_v_old = v.node;
      }
    }
    if(min_v_old == source){
      ns[source] -= 1;
      --epoch;
    }
    for(auto v = labeled_old.begin(); v != labeled_old.end(); v++){
      if(min_d_old > d[*v]){
        min_d_old = d[*v];
        min_v_old = *v;
      }
    }
    source = min_v_old;
    labeled_old.insert(labeled_new.begin(), labeled_new.end()); //Take union of both sets
    labeled_new.clear();
    if(labeled_old.empty()){
      flag[1] = true;
      break;
    }
    labeled_old.erase(source);
  }
  return flag;
}

int* BellManFordImp::getDistanceArr() const{
  return d;
}

vector<vector<int> > BellManFordImp::getPredecessors(int s){
  vector<vector<int> > pred;
  pred.resize(V);
  getPredecessorsUtil(s, V - 1, V - 1, pred);
  pred[s].push_back(s);
  return pred;
}

void BellManFordImp::getPredecessorsUtil(int s, int t, int idx, std::vector<std::vector<int> >& predecessors){
  if(t >= 1){
    while(idx != s){
      predecessors[t].push_back(idx);
      idx = pi[idx];
    }
    predecessors[t].push_back(s);
    getPredecessorsUtil(s, t - 1, t - 1, predecessors);
  }
}

vector<int> BellManFordImp::getNegCycle(){
  vector<int> tmp; //temporary vector to hold cycle elements
  for(int ii = cycle.size() - 1; ii >= 0; ii--){
    tmp.push_back(cycle[ii]);
    if(cycle[ii - 1] == cycleStart){
      tmp.push_back(cycle[ii - 1]);
      break;
    }
  }
  return tmp;
}

int BellManFordImp::getEpoch() const{
  return epoch;
}
