#ifndef CRR_h
#define CRR_h

#include <cmath>
// Some common payoff functions
double vanillaCall(double S, double K);
double vanillaPut(double S, double K);
double binCall(double S, double K);
double binPut(double S, double K);

class Node{
  private:
    double S;
    double V;
    Node * p_up;
    Node * p_dwn;
  public:
    Node();
    void setS(double S_t);
    void link(Node & up, Node & down);
    void calcV(double q, double d, double (*payoff_f)(double, double),
              double K, bool Ex, double uBarr, double lBarr);
    void shoutV(double q, double d, double (*payoff_f)(double, double),
                double K, double T, double tau, double vol, int Per_left,
                double r, double div);
    double retV();
    void NodeInfo();
};

class Tree{
  private:
    Node * nodes;
    double deltaT;
    int nPer;
    int nNodes;
    double q;
    double u;
    double d;
    double Tmat;
    double IV;
    double y;
    double rfr;
  public:
    Tree(double S0, double vol, double T, int Per, double r, double div = 0);
   ~Tree();
    double price(double K, double (*payoff_f)(double, double),
                 bool * PreExNodes, bool shout = false, double uBarr = HUGE_VAL, double lBarr = 0);
    double priceCompound(double K1, double K2, double T1, //T2 is the Tmat attribute of the tree
                         double (*payoff_1)(double, double), double (*payoff_2)(double, double));
};


#endif
