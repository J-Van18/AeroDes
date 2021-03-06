/*
 * ©2022 The Regents of the University of California.  All rights reserved.
 */

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <iostream> // std
#include <iomanip>  // setw
#include <fstream>  // fopen
#include <string>
#include <math.h>   // pow

#include "variables.hpp"
#include "wake.hpp"

/*
 * compile:
 * 	g++ -Xpreprocessor -fopenmp -framework Accelerate -o test main.cpp algorithmic.cpp
 * 	g++ -o test main.cpp wake.cpp
 *  ./test -in wake.data
 *	valgrind --leak-check=yes ./test
 */

using namespace std;

int main(int argc, char** argv) {
    variables *vars = new variables();
    wake *wk = new wake(vars);

    wk->cmdInput(argc, argv);
    wk->readInputParams();
    wk->printInputParams();

    wk->readInputPolar("polarbl.dat");
    //wk->printXFoilValues();

    wk->setMesh();
    //wk->printGeomSummary();

    wk->solveLiftingLine();
    wk->printResults();
    //wk->printDistributions();

    wk->readInputCanardGeom("geocanard.xzmses");
    wk->integrate_canard();
    wk->printCanarWake();

    // output results to file
    wk->outputGammaDownwash("prandtline.ygw");
    wk->outputCanardWake("canarwake.xz");

    wk->readInputWingGeom("wing.yxlexte");

    delete wk;
    delete vars;
}
