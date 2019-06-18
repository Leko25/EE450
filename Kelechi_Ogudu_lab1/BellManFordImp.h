//Name: Kelechi Ogudu
//USC_ID: 5413285132

#ifndef BELLMANFORDIMP_H
#define BELLMANFORDIMP_H
#include <vector>

/**
 Associated Node structure
*/
struct Node{
  int node, wt;
  Node(int n, int weight){
    node = n;
    wt = weight;
  }
};

class BellManFordImp{
private:
  //Graph in the form of an adjacency list
  std::vector<std::vector<Node> > g;
  //Number of nodes in the graph
  int V;
  //Array containing the distance vector from source to v
  int * d;
  //Array containing the distance of a vertex in the previous iteration
  int *  pd;
  //Shortest path which maintains the predecessors from v
  int *  pi;
  //Array containing information about the number of times a node has been scannned
  int *  ns;
  //Negative cycle
  std::vector<int> cycle;
  //start of the cycle
  int cycleStart;
  //number of iterations 
  int epoch;
public:
  /** Destructor */
  ~BellManFordImp();

  /**
   Constructor initializes d, pd, pi --> inf and ns --> 0
   Adjacency List g is also initialized from values in vector parameter
   @param vec: vector containing values read from text file
   @param numRows: number of rows in text file
  */
  BellManFordImp(std::vector<int>& vec, int numRows);

  /**
   Optimized BellManFord implementation
   @param s: source node
   @return: returns vector v, such that v.size() == 2 and v[0] == true - implies
   a negative cycle, while v[1] == true implies a solution was found
  */
  std::vector<bool> OptBellManFord(int s);

  /**
   @return shortests paths from source node to all vertices
  */
  int * getDistanceArr() const;

  /**
   @returns the number of iterations for the optimized BellmanFord
  */
  int getEpoch() const;

  /**
   @returns a vector containing the negative cycle if flag[0] = true
  */
  std::vector<int> getNegCycle();

  /**
  @param s: source node
   @return recursively gets the predecessors form the source node to all
           other nodes in hte graph
  */
  std::vector<std::vector<int> > getPredecessors(int s);

  /**
   Utility function for getPredecessors
  */
  void getPredecessorsUtil(int s, int t, int idx, std::vector<std::vector<int> >& predecessors);
};

#endif
