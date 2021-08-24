#include "CRR.h"
#include <cmath>
#include <iostream>
using  namespace std;

// Some common payoff functions
double vanillaCall(double S, double K){
  return fmax(S-K, 0);
}

double vanillaPut(double S, double K){
  return fmax(K-S, 0);
}

double binCall(double S, double K){
  if (S > K){
    return 1;
  } else {
    return 0;
  }
}

double binPut(double S, double K){
  if (S < K){
    return 1;
  } else {
    return 0;
  }
}

Node::Node(){
  S = 0;
  p_up  = nullptr;
  p_dwn = nullptr;
  V = -1;
}

void Node::setS(double S_t){S = S_t;}

void Node::link(Node & up, Node & dwn){
  // Links the node with the nodes corresponing to
  // up/down movements in the next period, to allow for easy pricing
  p_up  = & up;
  p_dwn = & dwn;
}

void Node::NodeInfo(){
  cout << "\nUp-adress: "   << p_up <<
          "\nDown-adress: " << p_dwn <<
          "\nThis adress: " << this  <<
          "\n \nS:"         << S <<
          "\nV:"            << V << "\n";
}

void Node::shoutV(double q, double d, double (*payoff_f)(double, double),
                  double K, double T, double tau, double vol, int Per_left,
                  double r, double div){

  /* Calculates the value of a shout option at the given node by
     repricing the theoretical option (thus needing as input more pricing parameters)
     that arises due to the shout and evaluating whether that is optimal for the
     option holder.

     The new parameters are:
     - the time elapsed tau
     - the number of periods left until maturity

     Only compatible with European style options without barriers */

  Tree newOption(S, vol, T-tau, Per_left, r, div);
  bool ex[Per_left] = {false};

  double shoutVal = payoff_f(S, K) + newOption.price(S, payoff_f, ex); // Cash + Option with S_tau strike
  V = d*(q*(p_up->V) + (1-q)*(p_dwn->V)); // Risk neutral valuation
  V = fmax(shoutVal, V);

}

void Node::calcV(double q, double d, double (*payoff_f)(double, double),
                 double K, bool Ex, double uBarr, double lBarr){

  /* Calculates the option value at the given node
     Given: the risk neutral probability (q) and discount factor (d)
            the payoff function payoff_f(), as a function of S and K
            the strike price (K)
            if the option can be exercised at the node (Ex)
            any barriers on the option */

  double payoff = payoff_f(S, K); // Calculate the payoff from the input

  if (S > uBarr || S < lBarr){ // Outside of the barriers
    V = 0;

  } else if (p_up == nullptr){ // Maturity
    V = payoff;

  } else { // Risk neutral valuation
    V = d*(q*(p_up->V) + (1-q)*(p_dwn->V));

    if (Ex){ // Account for possible pre-exercise
      V = fmax(V, payoff);
    }
  }
}

double Node::retV(){ return V; }

Tree::Tree(double S0, double vol, double T, int Per, double r, double div){
  // Get and calculate pricing parameters
  Tmat = T; // Time (years) to Maturity
  IV = vol; // implied volatility
  rfr = r; // continuous risk free interest rate
  y = div; // continuous dividend yield
  deltaT = T/Per; // Time step between layers
  u = exp(vol*sqrt(deltaT)); // Up-factor (down factor is given as 1/u)
  q = (exp((r-div)*deltaT)-pow(u,-1))/(u-pow(u,-1)); // Risk neutral probability
  d = exp(-r*deltaT); // Discount factor

  // Size parameters
  nPer = Per;
  nNodes = (nPer+2)*(nPer+1)/2;

  // Allocate memory and store the tree structure in an array
  nodes = new Node[nNodes];

  int layer = 0;
  int depth = 0;
  for (size_t i = 0; i < nNodes; i++) {
    nodes[i].setS(S0*pow(u, layer-2*depth));
    if (layer < nPer){
      nodes[i].link(nodes[i+layer+1], nodes[i+layer+2]);
    }
    if (layer == depth){
      depth = 0;
      layer++;
    } else {
      depth++;
    }
  }
}

Tree::~Tree(){delete[] nodes;}

double Tree::priceCompound(double K1, double K2, double T1, //T2 is the Tmat attribute of the tree
                          double (*payoff_1)(double, double), double (*payoff_2)(double, double)){

  /* Calculates the option value today of a European style compound option given
     the two strike prices and payoffs and the maturity of the first option (the
     second option matures at time Tmat, given in the Tree constructor). */

  int layer = nPer;
  int depth = 0;
  int layer1 = nPer*T1/Tmat; // the layer at which the first option expires

  for (int i = nNodes-1; i >= 0; i--){
    if (layer >= layer1){ // Calculate the value of the second option
      nodes[i].calcV(q, d, payoff_2, K2, false, HUGE_VAL, 0);

      if (layer == layer1){
        // At the "inflection", make the second option the underlying
        // and restart the pricing, by first calculating the payoff
        nodes[i].setS(nodes[i].retV());
        nodes[i].calcV(q, 0, payoff_1, K1, true, HUGE_VAL, 0);
        // d = 0 -> payoff function is calculated, as the risk neutral value = 0
      }
    } else {
      // And then calculating the risk neutral value
      nodes[i].calcV(q, d, payoff_1, K1, false, HUGE_VAL, 0);
    }
    if (depth == layer){
      depth = 0;
      layer--;
    } else {
      depth++;
    }
  }
  return nodes[0].retV();
}

double Tree::price(double K, double (*payoff_f)(double, double),
             bool * PreExNodes, bool shout, double uBarr, double lBarr){

  /* Calculates the option value today given the:
     Strike price (K)
     Payoff function (depending on S and K only)
     Time points that the option can be pre-exercised,
     given by an array of booleans that is as long as the number of periods nPer
     (an american option for instance contains true everywhere) and
     any possible barriers (for up/down and out options).

     Compatible with shout options. For compound options, use the priceCompound()
     method instead, due to the different parameters needed for the pricing. */

  int layer = nPer;
  int depth = 0;
  bool exercise = true;
  for (int i = nNodes-1; i >= 0; i--){

    // Determine if the option can be exercised at this node
    if (layer != nPer){
      exercise = PreExNodes[layer];
    }

    // Calculate the option value
    if (shout && layer != nPer){ // evaluate the shout value
      nodes[i].shoutV(q, d, payoff_f, K, Tmat, layer*deltaT, IV, nPer-layer, rfr, y);
    } else {
      nodes[i].calcV(q, d, payoff_f, K, exercise, uBarr, lBarr);
    }
    if (depth == layer){
      depth = 0;
      layer--;
    } else {
      depth++;
    }
  }

  // Return the value today
  return nodes[0].retV();
}
