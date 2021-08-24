# CRR-option-pricing
C++ code for pricing options using the Cox-Ross-Rubenstein model, also known as the Binomial model. Done as a side project to boost my skills in C++, particularly with poiners and OOP.

The model works by first building a tree specified by the dynamics of the underlying (price today, implied volatility, time to maturity, risk free interest rate, continuous dividend yield and number of periods), and then pricing the option given the payoff rules (strike price, payoff function, pre-excersise scheme, barriers and shout feature).

The tree is stored, node by node, in a single array, and can be reused to price several options (given that the underlying follows the same dynamics), making the program fast and memory efficient.

The pricing function is fairly general as it can price any payoff function (as long as it is a function of S_T and K only) and all pre-exercise schemes (provided the user calculates at which nodes the option can be exercised). Some other exotic features that can be priced are: compound options (which have their own method in the Tree class due to the different input parameters needed), shout options and up/down-and-out options. As the CRR model generally struggles with heavily path dependent options, these are not incorporated in the code.

The file demo.cpp shows a usecase of the Tree-class and some of the options that can be priced with it.
