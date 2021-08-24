#include "CRR.h"
#include <iostream>
using namespace std;


int main() {
  // Pricing parameters
  double S0 = 62;
  double vol = 0.25;
  double K = 60;
  double r = 0.05;
  double div = 0.01;
  double T = 1.0/12;
  int per = 200;

  //  Determine exercise scemes
  bool Euro[50] = {0};
  bool Am[50];
  bool Bermuda[50] = {0};

  double DayStep = 252*T/per;
  for (size_t i = 0; i < 50; i++) {
    Am[i] = 1;
    // Ex: Bermudan option that can be exercised on the 5th day
    if (DayStep*i < 6 && DayStep*i > 5){
      Bermuda[i] = 1;
    }
  }

  // One tree suffices, if we assume the underlying follows the same dynamics
  // for all the different options to be priced (IV, dividend, rfr etc.)
  Tree binTree(S0, vol, T, per, r, div);

  // Some examples of option prices
  std::cout << "Price of a European corridor call option (barriers: 68, 56): "
            << binTree.price(K, vanillaCall, Euro, false, 68, 56) << '\n';
  std::cout << "Price of an American binary put option: "
            << binTree.price(K, binPut, Am) << '\n';
  std::cout << "Price of a Bermudan put option: "
            << binTree.price(K, vanillaPut, Bermuda) << '\n';
  std::cout << "Price of a shout call option: "
            << binTree.price(K, vanillaCall, Euro, true) << '\n';
  std::cout << "Price of a CoP option (strikes 1, 60, T1 in half a month): "
            << binTree.priceCompound(1, K, 0.5/12,
                                     vanillaCall, vanillaPut) << '\n';

  return 0;
}
