#include <cstdlib>
#include <cstdio>
#include <vector>
#include <iostream> // std
#include <iomanip>  // setw
#include <sstream>  // istream
#include <fstream>  // fopen, ifstream
#include <string>
#include <stdio.h>  // strcpy
//#include <string.h>
#include "canareq.hpp"

using namespace std; // g++ canareq.cpp -c

canareq::canareq(int argc, char** argv) {
    // storage
    int nrows=3; int ncols=3;
    aa = (double **) malloc(sizeof(double *)*nrows);
    bb = (double *) malloc(sizeof(double)*ncols);
    create_2d_double_array(nrows, ncols, aa);
    
    lxx=101; ndatx=21;
    kxtrm = (int *) malloc(sizeof(int)*lxx);
    vr = (float *) malloc(sizeof(float)*ndatx);
    tr = (float *) malloc(sizeof(float)*ndatx);
    
    cx = (float *) malloc(sizeof(float)*lxx);
    cz = (float *) malloc(sizeof(float)*lxx);
    cq = (float *) malloc(sizeof(float)*lxx);
    inc = (float *) malloc(sizeof(float)*lxx);
    
    filenamePolarDat = "canarpolar.dat";
    filenamePolarPrandtline = "canarpolar.prandtline";
    filenamePolarCdClCq = "canarpolar.cdclcq";
    filenameEqData = "canareq.data";
    filenameEqCdClCqm = "canareq.cdclcqm";
    filenameEqItres = "canareq.itres";
    filenameEqCdClCq = "canareq.cdclcq";
    filenameEqCdMcln = "canareq.cdmcln";
    filenameEqCmCg = "canareq.cmcg";
    filenameEqxtabCl = "canareq.xtabvcl";
    filenameEqCdClCqMeq = "canareq.cdclcqmeq";
    filenameEqCdClCqCmCgEq = "canareq.cdclcqcmcgeq";
    filenameEqList = "canareq.list";
    filenameEqStab = "canareq.stab";
    filenameEqDataOpt = "canareq.optimized";
    
    // constants
    pi=2.*asin(1.);
    eps=1.e-6;
    us3=1.0/3.0;
    cout << "pi = " << pi << endl;
    degrad=pi/180.;
    radeg=1./degrad;
    nal=30;

    // initial design results
    aleq0=0;
    Ueq0=0;
    beteq0=0;
    Cleq0=0;
    Cdeq0=0;
    Cmac0=0;
}

canareq::~canareq() {
    delete_2d_double_array(aa);
    free(bb);
}

double *canareq::create_1d_double_array(int n2, double *array) {
    // create a n2 x 1 matrix
    array = (double *) malloc(n2*sizeof(double));
    return array;
}

double **canareq::create_2d_double_array(int n1, int n2, double **array) {
    // create a n1 x n2 matrix
    int n=0;
    double *data = (double *) malloc(n1*n2*sizeof(double));
    for (int i=0; i<n1; i++) {
        array[i] = &data[n];
        n += n2;
    }
    return array;
}

void canareq::delete_2d_double_array(double **array) {
    free(array[0]);
    free(array);
}

float canareq::thrust(int ndat, float v){
    //int ndatx,ndat,i,im;
    //float v,T,dTdv,prod;
    //float vr(ndatx),tr(ndatx);
    int im;
    float prod;

    if(abs(v) < 1.0) {
        dTdv=(tr[1]-tr[0])/(vr[1]-vr[0]);
        T=tr[0]+(v-vr[0])*dTdv;
        return dTdv;
    }
    
    // else
    prod=v-vr[1];
    for (int i=1; i<ndat; i++) {
        prod=prod*(v-vr[i]);
        if(prod <= 0){
            dTdv=(tr[i]-tr[i-1])/(vr[i]-vr[i-1]);
            T=tr[i-1]+dTdv*(-vr[i-1]);
            return dTdv; // exit
        }
        prod=1.0;
        im=i;
    }
    dTdv=(tr[ndat]-tr[ndat-1])/(vr[ndat]-vr[ndat-1]);
    T=tr[ndat-1]+dTdv*(-vr[ndat-1]);
    return dTdv;
}

void canareq::mat3(double usdet, double **a, double *b) {
    //implicit none
    //doubleprecision usdet,b1,b2,b3
    //doubleprecision a(3,3),b(3);
    usdet=1./(a[0][0]*(a[1][1]*a[2][2]-a[1][2]*a[2][1])
            -a[1][0]*(a[0][1]*a[2][2]-a[0][2]*a[2][1])
            +a[2][0]*(a[0][1]*a[1][2]-a[0][2]*a[1][1]));
    
    b1=(b[0]*(a[1][1]*a[2][2]-a[1][2]*a[2][1])
        -b[1]*(a[0][1]*a[2][2]-a[0][2]*a[2][1])
        +b[2]*(a[0][1]*a[1][2]-a[0][2]*a[1][1]))*usdet;
    
    b2=(-b[0]*(a[1][0]*a[2][2]-a[1][2]*a[2][0])
        +b[1]*(a[0][0]*a[2][2]-a[0][2]*a[2][0])
        -b[2]*(a[0][0]*a[1][2]-a[0][2]*a[1][0]))*usdet;
    
    b3=(b[0]*(a[1][0]*a[2][1]-a[1][1]*a[2][0])
       -b[1]*(a[0][0]*a[2][1]-a[0][1]*a[2][0])
       +b[2]*(a[0][0]*a[1][1]-a[0][1]*a[1][0]))*usdet;
    
    b[0]=b1;
    b[1]=b2;
    b[2]=b3;
}

void canareq::linearModel() {
    //*****linear model results
    aleq=-(Cmac0+(xcg-xac)*Cl0/lref)
            /(dCmacda+(xcg-xac)*dClda/lref);
    Cleq=Cl0+dClda*aleq;
    Cweq=Cleq;
    if(Cleq <= 0.) cout << "################# no linear solution with Cleq<0" << endl;

    Ueq=sqrt(2.*mg/(rho*aref*Cleq));
    reyeq=Ref*Ueq/100.;
    dynaref=0.5*rho*pow(Ueq,2)*aref;

    dTdv = thrust(ndat,Ueq);

    Cteq=rcor*(T+dTdv*Ueq)/dynaref;
    //     search for point on wing polar and interpolate Cd
    m=0;
    prod=aleq+0.5*pi;

    for (int j = 0; j < kx; ++j) {
        prod=prod*(aleq-inc[j]);
        if(prod < -eps) break;
        prod=1.;
        m=j;
    }
    //m=8;
    Cdeq=cx[m]+(aleq-inc[m])*(cx[m+1]-cx[m])
            /(inc[m+1]-inc[m]);

    //     wing induced drag estimation
    Cdmeq=Cdeq;
    Clmeq=Clm0+dCldam*aleq;
    Cdim=pow(Clmeq,2)/(pi*em*arm);
    Cdm0=Cdmeq-Cdim;

    //     calculate fuselage friction drag
    reyf=reyeq*lf/cam;
    Cdf0=1.328/sqrt(reyf);

    if(reyf > 1.0E5) Cdf0=.072/pow(reyf,.2);

    Cdfeq=Cdf0;

    //     calculate canard induced drag and friction drag (2 elements)
    Clceq=Clc0+dCldac*aleq;
    Cdic=2.0*pow(Clceq,2)/(pi*ec*arc);
    reyc=reyeq*cac/cam;
    Cdc0=1.328/sqrt(reyc);
    if(reyc > 1.0E5) Cdc0=.072/pow(reyc,.2);
    Cdceq=Cdic+2.0*Cdc0;

    //     total drag
    Cdeq=(am*Cdmeq+af*Cdfeq+ac*Cdceq)/aref;
    Cdeq=Cdeq+Cdbrake;
    beteq=(Cteq-Cdeq)/Cweq;
    Cmac=Cmac0+dCmacda*aleq;
    Cmeq=Cm0+dCmda*aleq;
    bb[0]=-(Cmeq+xcg*Cweq*cos(aleq+beteq)/lref);
    bb[1]=-(Cleq-Cweq*cos(beteq)+Cteq*sin(aleq));
    bb[2]=-(Cdeq+Cweq*sin(beteq)-Cteq*cos(aleq));

    cout << "\n";
    cout << std::setprecision(6);
    cout << "********************************************" << endl;
    cout << "****linear model results: m = " << m << endl;
    cout << "aleq = " << left << setw(10) << aleq << " Ueq  = " << left << setw(10) << Ueq << " beteq = " << left << setw(10) <<  beteq << endl;
    cout << "Cleq = " << left << setw(10) << Cleq << " Cdeq = " << left << setw(10) << Cdeq << " Cteq = " << left << setw(10) <<  Cteq << " CM,ac = " << left << setw(10) << Cmac << endl;
    cout << "*****************non-linear equations residuals from linear solution:" << endl;
    cout << "equ1 = " << left << setw(10) << bb[1] << "equ2 = " << left << setw(10) << bb[2] << " equ3 = " << left << setw(10) << bb[3] << endl;
    cout << "*****given best design parameters:" << endl;
    cout << "aleq = " << left << setw(10) << aleqB << " Ueq  = " << left << setw(10) << UeqB << " beteq = " << left << setw(10) <<  beteqB << endl;
    cout << "Cleq = " << left << setw(10) << CleqB << " Cdeq = " << left << setw(10) << CdeqB << " CM,ac = " << left << setw(10) << CmacB << endl;

    // replace initial design parameters with ideal design parameters
    aleq0=aleqB; //aleq;
    Ueq0=UeqB; //Ueq;
    beteq0=beteqB; //beteq;
    Cleq0=CleqB; // Cleq;
    Cdeq0=CdeqB; // Cdeq;
    Cmac0=CmacB; // Cmac;

    // ...
    Cteq=CdeqB;
    Cweq=CleqB;
    Cmeq=CmacB-xac*CleqB*cos(aleqB)/lref;
    Cmcg=Cmeq+xcg*Cweq*cos(aleqB+beteqB)/lref;
    amarg=100.*(xac-xcg-eps)/lref;
}

void canareq::nonlinearModel() {
    cout << "begin non-linear equilibrium algorithm" << endl;
    //write(24,1002)
    //write(24,*)'begin non-linear equilibrium algorithm'

    // *****read canar setting angle tcd (deg)
    //write(6,*)'tcd=? (<-99 exit)'
    //write(24,*)'tcd=? (<-99 exit)'
    //read(5,*)tcd
    tcd = 5;

    cout << "canar tcd = " << tcd << " flap tfd = " << tfd << endl;
    //write(24,*)'canar tcd=',tcd,'flap tfd=',tfd
    //if(tcd <= -99.) break; //goto 200

    tc=degrad*tcd;

    //*****update with best design input data
    aleq=aleq0;
    Ueq=Ueq0;
    beteq=beteq0;
    Cleq=Cleq0;
    Cdeq=Cdeq0;
    Cmac=Cmac0;
    Cteq=Cdeq;
    Cweq=Cleq;
    Cmeq=Cmac-xac*Cleq*cos(aleq)/lref;
    Cmcg=Cmeq+xcg*Cweq*cos(aleq+beteq)/lref;

    //     aerodynamic center moment coefficient from design data:
    cout << "********************************************" << endl;
    cout << "xcg, xac (xcg calculated or given):" << endl;
    cout << "center of gravity:       xcg = " << xcg << " (m)" << endl;
    cout << "aerodynamic center:      xac = " << xac << " (m)" << endl;
    cout << "amarg = " << amarg << " lref = " << lref << endl;
    //write(24,*)'********************************************'
    //write(24,*)' xcg, xac (xcg calculated or given):'
    //write(24,1002)
    //write(24,*)'center of gravity:       xcg=',xcg,' (m)'
    //write(24,*)'aerodynamic center:      xac=',xac,' (m)'
    //write(24,1003)amarg,lref

    // *****equilibrium
    //     alpha at equilibrium, lift and moment
    iter=0;
    // *****iterations
    for (int i = 0; i < itx; ++i) {
        //do 9 it=1,itx
        // engine operating point
        dTdv = thrust(ndat, Ueq);
        dynaref = 0.5*rho*aref*pow(Ueq,2);
        Cteq = rcor * (T + dTdv * Ueq) / dynaref;
        Cweq = mg / dynaref;
        reyeq = Ref * Ueq / 100.;
        // lift coefficient of canar
        dCldac = 2.0 * pi / (1.0 + 2.0 / arc);
        Clc0 = dCldac * tc;
        Clceq = Clc0 + dCldac * aleq;
        // moment coefficients of single canard Cmacc and Cmco
        dCmacdac = 0.;
        Cmacc0 = 0.;
        dCmdac = dCmacdac - xacc * dCldac / cac;
        Cmc0 = Cmacc0 - xacc * Clc0 / cac;
        // search for point on polar of wing
        m = 1;
        prod = aleq + .5 * pi;
        for (int j = 0; j < (kx-1); ++j) {
            //do 7 k = 2, kx - 1
            prod = prod * (aleq - inc[j]);
            if (prod <= eps) break;//goto 8
            prod = 1.;
            m = j;
            //7 continue
        }
        //8 continue
        // lift coefficients of main wing
        dCldam = (cz[m + 1] - cz[m]) / (inc[m + 1] - inc[m]);
        Clm0 = cz[m] + dCldam * (-inc[m]);

        // add canar influence on Clm
        Clm0 = Clm0 + acw * dCldalind * Clceq / (pi * arc);

        // add flap influence on Clm
        Clm0 = Clm0 + dClmdtf * tf;

        // moment coefficients of the main wing Cmacm and Cmmo
        dCmacdam = (cq[m + 1] - cq[m]) / (inc[m + 1] - inc[m]);
        Cmac0 = cq[m] + dCmacdam * (-inc[m]);
        dCmdam = dCmacdam - xacm * dCldam / cam;
        Cmm0 = Cmac0 - xacm * Clm0 / cam;

        // add flap influence on Cmm
        Cmac0 = Cmac0 + dCmmdtf * tf;
        Cmm0 = Cmm0 + dCmmdtf * tf;

        // add influence of wing on canard(downwash)
        Clc0 = Clc0 + dCldac * awc * Clm0 / (pi * arm);

        // global coefficients wing + canard(ac is area of 2 canards)
        // global lift
        dClda = (am * dCldam + ac * dCldac) / aref;
        Cl0 = (am * Clm0 + ac * Clc0) / aref;

        // global moments(ac is area of 2 canards)
        dCmacda = (am * cam * dCmacdam + ac * cac * dCmacdac) / (aref * lref);
        Cmac0 = (am * cam * Cmacm0 + ac * cac * Cmacc0) / (aref * lref);
        dCmda = (am * cam * dCmdam + ac * cac * dCmdac) / (aref * lref);
        Cm0 = (am * cam * Cmm0 + ac * cac * Cmc0) / (aref * lref);

        // main wing drag
        Cdmeq = cx[m] + (aleq - inc[m]) * (cx[m + 1] - cx[m]) / (inc[m + 1] - inc[m]);

        // add drag of flap
        Cdmeq = Cdmeq + (dCdtf0 + dCdtf1 * aleq + dCdtf2 * pow(aleq,2)) * tf / 0.1744;

        // wing induced drag estimation
        Clmeq = Clm0 + dCldam * aleq;
        Cdim = pow(Clmeq,2) / (pi * em * arm);
        Cdm0 = Cdmeq - Cdim;

        // calculate fuselage friction drag
        reyf = reyeq * lf / cam;
        Cdf0 = 1.328 / sqrt(reyf);
        if (reyf > 1.0E5){
            Cdf0 = .072 / pow(reyf, .2);
        }
        Cdfeq = Cdf0;
        // calculate canard induced drag and friction drag
        Clceq = Clc0+dCldac*aleq;
        Cdic = 2.0*pow(Clceq,2) / (pi*ec*arc);
        reyc = reyeq*cac / cam;
        Cdc0 = 1.328 / sqrt(reyc);
        if (reyc > 1.0E5) {
            Cdc0 = .072 / pow(reyc,.2);
        }
        Cdceq = Cdic+2.0*Cdc0;

        // global lift and moment at x = 0
        Cleq = Cl0 + dClda * aleq;
        Cmeq = Cm0 + dCmda * aleq;

        //c calculate rudder drag for 2 sides
        reyr = reyeq * rav / cam;
        Cdr0 = 1.328 / sqrt(reyr);
        if (reyr > 1.0E5) {
            Cdr0 = .072 / pow(reyr,.2);
        }
        Cdreq = 2.0 * Cdr0;

        // if winglets are used as rudder, double contribution
        // Cdreq = 4.0 * Cdr0
        // calculate nacelles drag for 2 engines
        reyn = reyeq * lna / cam;
        Cdn0 = 1.328 / sqrt(reyn);
        if (reyn > 1.0E5) {
            Cdn0 = .072 / pow(reyn,.2);
        }
        Cdneq = 2.0 * Cdn0;

        // total drag
        Cdeq = (am * Cdmeq + af * Cdfeq + ac * Cdceq + ar * Cdreq + an * Cdneq) / aref;
        Cdeq = Cdeq + Cdbrake;

        // relaxation method equation by equation
        // equation 1 for daleq with correction for influence of canard on wing
        daleq = -(Cmeq + xcg * Cweq * cos(aleq + beteq) / lref - zeng * Cteq / lref)
                        /(dCmda - xcg * Cweq * sin(aleq + beteq) / lref);
        aleq = aleq + omega * daleq;

        // daleq = 0.
        // update global coefficients
        aleqd = radeg * aleq;
        Cleq = Cl0 + dClda * aleq;
        Cmeq = Cm0 + dCmda * aleq;
        Cmac = Cmac0 + dCmacda * aleq;
        Cmcg = Cmeq + xcg * Cweq * cos(aleq + beteq) / lref;

        // equation 2 for dUeq
        dUeq = -(Cleq - Cweq * cos(beteq) + Cteq * sin(aleq))
                / (-2. * (0. * Cleq - Cweq * cos(beteq) + 0. * Cteq * sin(aleq)) / Ueq
                +0. * dTdv * sin(aleq) / dynaref);

        // dUeq = 0.
        // velocity and Reynolds number at equilibrium
        Ueq = Ueq + omega * dUeq;
        reyeq = rho * Ueq * cam / amu;

        // wing Reynolds number, wing induced drag estimation
        reym = reyeq;
        Clmeq = Clm0 + dCldam * aleq;
        Cdim = pow(Clmeq,2) / (pi * em * arm);
        Cdm0 = Cdmeq - Cdim;

        // fuselage viscous drag
        reyf = reyeq * lf / cam;
        Cdf0 = 1.328 / sqrt(reyf);
        if (reyf > 1.0E5) {
            Cdf0 = .072 / pow(reyf,.2);
        }
        Cdfeq = Cdf0;

        // canard viscous drag
        reyc = reyeq * cac / cam;
        Cdc0 = 1.328 / sqrt(reyc);
        if (reyc > 1.0E5) {
            Cdc0 = .072 / pow(reyc,.2);
        }
        Cdceq = Cdic + 2.0 * Cdc0;

        // rudder viscous drag for 2 sides
        reyr = reyeq * rav / cam;
        Cdr0 = 1.328 / sqrt(reyr);
        if (reyr > 1.0E5) {
            Cdr0 = .072 / pow(reyr,.2);
        }
        Cdreq = 2.0 * Cdr0;

        // if winglets are used as rudder, double contribution
        // Cdreq = 4.0 * Cdr0
        // nacelles viscous drag for 2 engines
        reyn = reyeq * lna / cam;
        Cdn0 = 1.328 / sqrt(reyn);
        if (reyn > 1.0E5) {
            Cdn0 = .072 / pow(reyn,.2);
        }
        Cdneq = 2.0 * Cdn0;

        // - total drag
        Cdeq = (am * Cdmeq + af * Cdfeq + ac * Cdceq + ar * Cdreq + an * Cdneq) / aref;
        Cdeq = Cdeq + Cdbrake;

        // equation 3 for dbeteq
        dbeteq = -(Cdeq + Cweq * sin(beteq) - Cteq * cos(aleq))
                 / (Cweq * cos(beteq));

        // dbeteq = 0.
        // angle of descent
        beteq = beteq + omega * dbeteq;
        beteqd = radeg * beteq;

        // evaluation of moments
        Cmeq = Cm0 + dCmda * aleq;
        Cmac = Cmac0 + dCmacda * aleq;
        inewton = 0;
        det = 1.0;
        usdet = 1. / det;

        // residuals
        bb[1] = -(Cmeq + xcg * Cweq * cos(aleq + beteq) / lref - zeng * Cteq / lref);
        bb[2] = -(Cleq - Cweq * cos(beteq) + Cteq * sin(aleq));
        bb[3] = -(Cdeq + Cweq * sin(beteq) - Cteq * cos(aleq));
        reseq = pow(bb[1],2) + pow(bb[2],2) + pow(bb[3],2);
        reseq = sqrt(reseq);
        iter = iter + 1;
        reseq0 = reseq;

        if (reseq0 < 10.*eps) {
            // Newton's Method
            inewton = 1;
            b1 = bb[1];
            b2 = bb[2];
            b3 = bb[3];
            aa[1][1] = dCmda - xcg * Cweq * sin(aleq + beteq) / lref - zeng * Cteq / lref;
            aa[1][2] = 2. * bb[1] / Ueq + zeng * dTdv / (dynaref * lref);
            aa[1][3] = -xcg * Cweq * sin(aleq + beteq) / lref;
            aa[2][1] = dClda + Cteq * cos(aleq);
            aa[2][2] = 2. * Cweq * cos(beteq) / Ueq;
            aa[2][3] = Cweq * sin(beteq);
            aa[3][1] = 0. * Cleq / (pi * em * arm) * dClda + Cteq * sin(aleq);
            aa[3][2] = 2. * bb[3] / Ueq - dTdv * cos(aleq) / dynaref;
            aa[3][3] = Cweq * cos(beteq);
            //
            mat3(usdet, aa, bb);
            //
            aleq = aleq + bb[1];
            Ueq = Ueq + bb[2];
            beteq = beteq + bb[3];
            reseq = pow(b1,2) + pow(b2,2) + pow(b3,2);
            reseq = sqrt(reseq);
            det = 1.0 / usdet;
        }

        // engine operating point
        dTdv = thrust(ndat, Ueq);
        //write(17, *) iter, reseq, inewton
        if (reseq < epser) {
            cout << " done iterating " << endl;
            break; // goto 10
        }
        //9       continue
    }
    //10   continue
}

void canareq::readInputParams() {
    // open input file
    ifstream paramfile(filenameEqData);
    if (!paramfile.is_open()) {
        cout << "\n\tCannot Read " << filenameEqData;
        cout << " File - Error in: readInputParams()" << endl;
        abort();
    }

    std::string line;
    std::string a; float b; std::string c; float v; float t;
    for (int i=0; i<lxx; i++, std::getline(paramfile, line) ) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        if(!(iss >> a >> b >> c) && (a.compare("ENGPERF")!=0) ) break;
        
        // newton method parameters
        if(a.compare("ITX")==0) itx = b;
        if(a.compare("OMEGA")==0) omega = b;
        if(a.compare("EPSER")==0) epser = b;
        
        // airflow and weight params
        if(a.compare("RHO")==0) rho = b;
        if(a.compare("AMU")==0) amu = b;
        if(a.compare("MASS")==0) mass = b;
        if(a.compare("XCG")==0) xcg = b;
        if(a.compare("STATMARG")==0) statmarg = b;
        
        // data for wing
        if(a.compare("BM")==0) bm = b;
        if(a.compare("CXM")==0) cxm = b;
        if(a.compare("CAM")==0) cam = b;
        if(a.compare("AM")==0) am = b;
        if(a.compare("XACM")==0) xacm = b;
        if(a.compare("DM")==0) dm = b;
        if(a.compare("EM")==0) em = b;
        if(a.compare("ACW")==0) acw = b;
        if(a.compare("dCldAlfaInd")==0) dCldalind = b;
        if(a.compare("TFD")==0) tfd = b;
        if(a.compare("dClmdtf")==0) dClmdtf = b;
        if(a.compare("dCmmdtf")==0) dCmmdtf = b;
        if(a.compare("dCdtf0")==0) dCdtf0 = b;
        if(a.compare("dCdtf1")==0) dCdtf1 = b;
        if(a.compare("dCdtf2")==0) dCdtf2 = b;
        
        // data for fuselage
        if(a.compare("LF")==0) lf = b;
        if(a.compare("RF")==0) rf = b;
        
        // data for canard
        if(a.compare("BC")==0) bc = b;
        if(a.compare("CXC")==0) cxc = b;
        if(a.compare("CAC")==0) cac = b;
        if(a.compare("AC")==0) ac = b;
        if(a.compare("XACC")==0) xacc = b;
        if(a.compare("DC")==0) dc = b;
        if(a.compare("EC")==0) ec = b;
        if(a.compare("AWC")==0) awc = b;
        if(a.compare("TCD")==0) tcd = b;
        
        // airbrake data
        if(a.compare("CdBRAKE")==0) Cdbrake = b;
        
        // data for rudder
        if(a.compare("RAV")==0) rav = b;
        if(a.compare("RUH")==0) ruh = b;
        
        // engine data
        if(a.compare("ZENG")==0) zeng = b;
        if(a.compare("DNA")==0) dna = b;
        if(a.compare("LNA")==0) lna = b;
        if(a.compare("HPOWER")==0)hpower = b;
        if(a.compare("PCENT")==0) Pcent = b;
        if(a.compare("NDAT")==0) ndat = b;
        if(a.compare("ENGPERF")==0) {
            std::getline(paramfile, line);   // read engine name
            strcpy(prop,line.c_str());
            //cout << "\n prop: " << prop;
            std::getline(paramfile, line);   // read header
            strcpy(title, line.c_str());
            //cout << "\n title: " << title;
            for (int i=0; i<20; i++) {
                std::getline(paramfile, line);
                std::istringstream iss(line);
                if(!(iss >> v >> t)) break;
                vr[i] = v; tr[i] = t;
            }
        }

        // best design input params
        if(a.compare("ALEQ")==0) aleqB = b;
        if(a.compare("UEQ")==0) UeqB = b;
        if(a.compare("BETAQ")==0) beteqB = b;
        if(a.compare("CLEQ")==0) CleqB = b;
        if(a.compare("CDEQ")==0) CdeqB = b;
        if(a.compare("CMAC")==0) CmacB = b;
    }

    // airflow and weight calcs
    amlb=2.205*mass;
    mg=mass*9.81;
    
    // wing calcs
    tf=degrad*tfd;

    // fuselage calcs
    hf=2.*(pi-2.)*rf;
    af=lf*hf;
    
    // canard calcs
    tc=degrad*tcd;
    arc=2.0*pow((0.5*bc-rf),2)/ac;

    // data for rudder
    ar=rav*ruh;
    
    // reference parameters
    aref=am+ac;
    lref=lf;
    Ueq=100.;
    Ref=rho*Ueq*cam/amu;
    
    // engine data
    an=pi*dna*lna;
    hpokwatt=745.7*hpower/1000.;
    Ueq=0.;
    
    thrust(ndat,Ueq); // initialize dTdv
    
    // thrust and thrust slope correction with density
    rcor=pow((rho/1.225),us3);
    rcor=rcor*pow((Pcent*Pcent/10000.0), us3);
    tr0=rcor*(T+dTdv*Ueq);
    dtrdv=rcor*dTdv;
    arm=bm*bm/am;

    if(statmarg >0.) xcg=xac-statmarg*lref;
    
    paramfile.close();
}

void canareq::readPolarDat() {
    // readPolarDat must be read after reading input parameters
    float a, b, c, d, e;
    std::string line;
    
    // open input file
    ifstream polarfile(filenamePolarDat);
    if (!polarfile.is_open()) {
        cout << "\n\nCannot Read: " << filenamePolarDat << endl;
        cout << " File - Error in: readPolarDat()" << endl;
        abort();
    }

    cout << "\n\n";
    cout << "*********extrema of the Cl(alpha) function:" << endl;
    cout << left << setw(10) << " inc ";
    cout << left << setw(10) << " cz ";
    cout << left << setw(10) << " cx ";
    cout << left << setw(10) << " dum ";
    cout << left << setw(10) << " cq ";
    cout << " i ";

    // determine maximum number of polar points
    km=0; // index start
    prod=1.0;

    // read first line
    std::getline(polarfile, line);
    std::istringstream iss(line);
    //cout << "\nline: \'"<< line << "\'"<< endl;

    for (int i=0; i<lxx; i++) {
        std::getline(polarfile, line);
        std::istringstream iss(line);
        //cout << "\nline: \'"<< line << "\'"<< endl;
        if(!(iss >> a >> b >> c >> d >> e)) break;  // break loop if end of file reached
        inc[i] = a;
        cz[i] = b;
        cx[i] = c;
        dum = d;
        cq[i] = e;

        //
        cout << std::setprecision(6);
        cout << "\n";
        cout << left << setw(10) << a;
        cout << left << setw(10) << b;
        cout << left << setw(10) << c;
        cout << left << setw(10) << d;
        cout << left << setw(10) << e;
        cout << i;
        
        kxtrm[i]=0;
        dcz=cz[i]-cz[km];
        prod=prod*dcz;
        if(prod < 0.0) {
            cout << "\nkxtrm = " << km << " cz(kxtrm) = " << cz[km];
            kxtrm[km]=km;
        }
        prod=dcz;
        km=i;
    }
    
    //kx=k-1; // off by 1 -Cp 3/4/22
    kx = km+1; // +1 to include zero'th point -Cp 3/4/22

    //*****zero incidence coefficients with assumption of small angles
    for(k=0; k<kx; k++) {
        kp=k+1; // kplus
        if(kp > kx) kp=kx;
        km=k-1; // kminus
        if(km < 0) km=1;
        if((k>1) && (k<kx)) {
            dcxm=((cx[k]-cx[km])*(cz[kp]-cz[k])*(cz[k]+cz[kp]-2.*cz[km])
                    -(cx[kp]-cx[k])*pow(cz[k]-cz[km],2))
                    /((cz[kp]-cz[k])*(cz[kp]-cz[km])*(cz[k]-cz[km]));
            
            dcxp=((cx[k]-cx[kp])*(cz[km]-cz[k])*(cz[k]+cz[km]-2.*cz[kp])
                    -(cx[km]-cx[k])*pow(cz[k]-cz[kp],2))
                    /((cz[km]-cz[k])*(cz[km]-cz[kp])*(cz[k]-cz[kp]));
        }
        prod=dcxm*dcxp;
        if((prod< -eps) && (kxtrm[km]!=0 || kxtrm[kp]!=0)) cout << "bad data distribution: interpolate a new data\n";
        
        // send this to output CxCzCqInc (writing polar information read in from Xfoil)
        //write(13,*)cx(k),cz(k),cq(k),inc(k)
        incd=inc[k];
        inc[k]=degrad*inc[k];
        //         prod=prod*(aleq-inc(k))
        if((inc[km]*inc[k]<=0.0) && (k!=0)) {
            k0=km;
            //*****zero incidence coefficients with assumption of small angles
            //     lift coefficients of main wing
            dCldam0=(cz[k]-cz[km])/(inc[k]-inc[km]);
            Clm00=cz[km]+dCldam0*(-inc[km]);
            //     add flap influence on Clm
            Clm00=Clm00+dClmdtf*tf;
            //     moment coefficients of the main wing Cmacm and Cmmo
            dCmacdam0=(cq[k]-cq[km])/(inc[k]-inc[km]);
            Cmacm00=cq[km]+dCmacdam0*(-inc[km]);
            dCmdam0=dCmacdam0-xacm*dCldam0/cam;
            Cmm00=Cmacm00-xacm*Clm00/cam;
            //     add flap influence on Cmm
            Cmacm00=Cmacm00+dCmmdtf*tf;
            Cmm00=Cmm00+dCmmdtf*tf;
            //     lift coefficients of single canard
            dCldac0=2.0*pi/(1.0+2.0/arc);
            Clc00=dCldac0*tc;
            //     moment coefficients of single canard Cmacc and Cmco
            dCmacdac0=0.;
            Cmacc00=0.;
            dCmdac0=dCmacdac0-xacc*dCldac0/cac;
            Cmc00=Cmacc00-xacc*Clc00/cac;
            //     global coefficients wing+canards (ac is area of 2 canards)
            //     global lift
            dClda=(am*dCldam0+ac*dCldac0)/aref;
            Cl0=(am*Clm00+ac*Clc00)/aref;
            //     global moments
            dCmacda=(am*cam*dCmacdam0+ac*cac*dCmacdac0)/(aref*lref);
            Cmac0=(am*cam*Cmacm00+ac*cac*Cmacc00)/(aref*lref);
            dCmda=(am*cam*dCmdam0+ac*cac*dCmdac0)/(aref*lref);
            Cm0=(am*cam*Cmm00+ac*cac*Cmc00)/(aref*lref);
        }
        
        if((k==0) && (inc[0]>=0.0)) {
            k0=1;
            //     lift coefficients of main wing
            dCldam0=(cz[1]-cz[0])/(inc[1]-inc[0]);
            Clm00=cz[0]+dCldam0*(-inc[0]);
            //     add flap influence on Clm
            Clm00=Clm00+dClmdtf*tf;
            //     moment coefficients of the main wing Cmacm and Cmmo
            dCmacdam0=(cq[1]-cq[0])/(inc[1]-inc[0]);
            Cmacm00=cq[0]+dCmacdam0*(-inc[0]);
            dCmdam0=dCmacdam0-xacm*dCldam0/cam;
            Cmm00=Cmacm00-xacm*Clm00/cam;
            //     add flap influence on Cmc
            Cmac00=Cmac00+dCmmdtf*tf;
            Cmm00=Cmm00+dCmmdtf*tf;
            //     lift coefficient of single canard
            dCldac0=2.0*pi/(1.0+2.0/arc);
            Clc00=dCldac0*tc;
            //     moment coefficients of single canard Cmacc and Cmco
            dCmacdac0=0.;
            Cmacc00=0.;
            dCmdac0=dCmacdac0-xacc*dCldac0/cac;
            Cmc00=Cmacc00-xacc*Clc00/cac;
            //     global coefficients wing+canards (ac is area of 2 canards)
            //     global lift
            dClda=(am*dCldam0+ac*dCldac0)/aref;
            Cl0=(am*Clm00+ac*Clc00)/aref;
            //     global moments
            dCmacda=(am*cam*dCmacdam0+ac*cac*dCmacdac0)/(aref*lref);
            Cmac0=(am*cam*Cmacm00+ac*cac*Cmacc00)/(aref*lref);
            dCmda=(am*cam*dCmdam0+ac*cac*dCmdac0)/(aref*lref);
            Cm0=(am*cam*Cmm00+ac*cac*Cmc00)/(aref*lref);
        }
        
        //     aerodynamic center of airplane
        xac=lref*(dCmacda-dCmda)/dClda;
        // change inc[k] back into original angle
        //write(14,*)cx(k),cz(k),cq(k),incd // compose this in 'outputPolarCdClCq()'
        //cout << left << setw(10) << k;
        //cout << left << setw(10) << incd;
        //cout << left << setw(10) << cz[k];
        //cout << left << setw(10) << cx[k];
        //cout << cq[k] << endl;
    }
    
    // read input file
    /*
    string oneline;
    while (getline(polarfile,oneline)) {
        //cout << "Read in from file: " << oneline << endl;
        if ( oneline.empty() ) continue;
        double input = (double)atof(oneline.c_str());
    }
    */

    // note: do not change kx after this point
    polarfile.close();
}

int canareq::printInputParams() {
    // print to screen
    cout << "***********************\n";
    cout <<  "convergence parameters:" << endl;
    cout << right << setw(32) <<  "                        itx = " << itx << "\n";
    cout << right << setw(32) <<  "                      omega = " << omega << "\n";
    cout << right << setw(32) <<  "convergence tolerance epser = " << epser << "\n";
    cout << "***********************\n";
    cout << "air parameters:" << endl;
    cout << right << setw(32) << "                     density = " << rho <<  " (kg/m**3)\n";
    cout << right << setw(32) << "               dynamic visc. = " << amu <<  " (m**2/s)\n";
    cout << "***********************\n";
    cout << "gravity data:" << endl;
    cout << right << setw(32) << "                        mass = " << mass <<  " (kg) =  " << amlb <<  " (lb)\n";
    cout << right << setw(32) << "              location of CG = " << xcg <<  " (m)\n";
    cout << right << setw(32) << "               static margin = " << statmarg <<  " (if <0, not used to place xcg)\n";
    cout << "***********************\n";
    cout << "wing data:" << endl;
    cout << right << setw(32) << "                        span = " << bm <<  " (m)\n";
    cout << right << setw(32) << "          root chord of wing = " << cxm <<  " (m)\n";
    cout << right << setw(32) << "               average chord = " << cam <<  " (m)\n";
    cout << right << setw(32) << "                   wing area = " << am <<  " (m**2)\n";
    cout << right << setw(32) << "  aerodynamic center of wing = " << xacm <<  " (m)\n";
    cout << right << setw(32) << "     relative camber of wing = " << dm << "\n";
    cout << right << setw(32) << "             wing efficiency = " << em << "\n";
    cout << right << setw(32) << " influence fo canard on wing = " << acw << "\n";
    cout << right << setw(32) << "           wing aspect ratio = " << arm << "\n";
    cout << right << setw(32) << "influence of canar dCldalind = " << dCldalind << "\n";
    cout << right << setw(32) << "          flap setting angle = " << tfd <<  " (deg)\n";
    cout << right << setw(32) << "flap setting influence on Cl = " << dClmdtf << "\n";
    cout << right << setw(32) << "flap setting influence on Cm = " << dCmmdtf << "\n";
    cout << right << setw(32) << "flap setting influence on Cd = " << dCdtf0 << "\n";
    cout << right << setw(32) << "flap setting influence on Cd = " << dCdtf1 << "\n";
    cout << right << setw(32) << "flap setting influence on Cd = " << dCdtf2 << "\n";
    cout << "***********************\n";
    cout << "fuselage data:        " << endl;
    cout << right << setw(32) << "                     length = " << lf <<  " (m)\n";
    cout << right << setw(32) << "                      radius = " << rf <<  " (m)\n";
    cout << right << setw(32) << "        fuselage missed area = " << af <<  " (m**2)\n";
    cout << "***********************\n";
    cout << "canard data:" << endl;
    cout << right << setw(32) << "                 canard span = " << bc <<  " (m)\n";
    cout << right << setw(32) << "           canard root chord = " << cxc <<  " (m)\n";
    cout << right << setw(32) << "canar mean aerodynamic chord = " << cac <<  " (m)\n";
    cout << right << setw(32) << "        canard planform area = " << ac <<  " (m**2)\n";
    cout << right << setw(32) << "aerodynamic center of canard = " << xacc <<  " (m)\n";
    cout << right << setw(32) << "   relative camber of canard = " << dc << "\n";
    cout << right << setw(32) << "aspect ratio of single canar = " << arc << "\n";
    cout << right << setw(32) << "           canard efficiency = " << ec << "\n";
    cout << right << setw(32) << " influence of wing on canard = " << awc << "\n";
    cout << right << setw(32) << "        canard setting angle = " << tcd <<  " (deg)\n";
    cout << "***********************\n";
    cout << right << setw(32) << "           airbrake: CDbrake = " << Cdbrake << "\n";
    cout << "***********************\n";
    cout << "rudder data:" << endl;
    cout << right << setw(32) << "        rudder average chord = " << rav <<  " (m)\n";
    cout << right << setw(32) << "               rudder height = " << ruh <<  " (m)\n";
    cout << right << setw(32) << "                 rudder area = " << ar <<  " (m**2)\n";
    cout << "***********************\n";
    cout << "reference data:" << endl;
    cout << right << setw(32) << "                 ref. length = " << lref <<  " (m)\n";
    cout << right << setw(32) << "                   ref. area = " << aref <<  " (m**2)\n";
    cout << right << setw(32) << "   reference Reynolds number = " << Ref <<  "\n";
    cout << "***********************\n";
    cout << right << setw(32) << " propulsion system reference = " << prop <<  "\n";
    cout << right << setw(32) << "      z-coordinate of engine = " << zeng <<  " (m)\n";
    cout << right << setw(32) << "         diameter of nacelle = " << dna <<  " (m)\n";
    cout << right << setw(32) << "           length of nacelle = " << lna <<  " (m)\n";
    cout << right << setw(32) << "             static traction = " << tr0 <<  " (N)\n";
    cout << right << setw(32) << "              traction slope = " << dtrdv <<  " (Ns/m)\n";
    cout << right << setw(32) << "                engine power = " << hpokwatt <<  " (kW) = " << hpower <<  " (hp)\n";
    cout << right << setw(32) << "percent of engine power avai = " << Pcent <<  "\n";
    cout << right << setw(32) << "                        ndat = " << ndat <<  "\n";
    cout << " i  ";
    cout << " vr(i)  ";
    cout << " tr(i)";
    for(int i = 0; i < ndat; i++) {
        cout << std::setprecision(3);
        cout << "\n";
        cout << left << setw(8) << i;
        cout << left << setw(8) << vr[i];
        cout << tr[i];
    }
    cout << "\n***********************\n";
    cout << "equilibrium data:" << endl;
    cout << right << setw(32) << " aleq = " << aleqB <<  "\n";
    cout << right << setw(32) << " ueq = " << UeqB <<  "\n";
    cout << right << setw(32) << " beteq = " << beteqB <<  "\n";
    cout << right << setw(32) << " cleq = " << CleqB <<  "\n";
    cout << right << setw(32) << " cdeq = " << CdeqB <<  "\n";
    cout << right << setw(32) << " cmac = " << CmacB <<  "\n";
    return 1;
}

int canareq::printPolarDat() {
    cout << "\n\n";
    cout << "*************wing polar (profile data from Xfoil):\n";
    cout << left << setw(16) << " i ";
    cout << left << setw(16) << " Cx ";
    cout << left << setw(16) << " Cz ";
    cout << left << setw(16) << " Cq ";
    cout << left << " Incidence " << endl;
    for(int i=0; i<kx; i++) {
        cout << std::setprecision(8);
        cout << left << setw(16) << i;
        cout << left << setw(16) << cx[i];
        cout << left << setw(16) << cz[i];
        cout << left << setw(16) << cq[i];
        cout << left << inc[i] << endl;
    }
    return 1;
}

void canareq::printGlobalCoefs() {
    cout << "\n******extrema pointer:" << endl;
    for (int j = 0; j < kx; ++j) {
        cout << "kxtrm(" << j << ") = " << kxtrm[j] << endl;
    }

    //*****global coefficients
    cout << std::setprecision(8);
    cout << "\n";
    cout << "**********************************************"<< endl;
    cout << "zero incidence aerodynamic coefficients at k0 = " << k0<< endl;
    cout << "**************lift:" << endl;
    cout << " wing:     dCldam0 = " << left << setw(15) << dCldam0 << left << setw(15) << " Clm00 = " << Clm00 << endl;
    cout << "canar:     dCldac0 = " << left << setw(15) << dCldac0 << left << setw(15) << " Clc00 = " << Clc00 << endl;
    cout << "total:       dClda = " << left << setw(15) << dClda   << left << setw(15) << " Cl0 = " << Cl0 << endl;
    cout << "*****moment at xac:" << endl;
    cout << " wing:   dCmacdam0 = " << left << setw(15) << dCmacdam0 << left << setw(15) << " Cmacm00 = " << Cmacm00 << endl;
    cout << "canar:   dCmacdac0 = " << left << setw(15) << dCmacdac0 << left << setw(15) << " Cmacc00 = " << Cmacc00 << endl;
    cout << "total:     dCmacda = " << left << setw(15) << dCmacda << left << setw(15) << " Cmac0   = " << Cmac0 << endl;
    cout << "*****moment at xcg:" << endl;
    cout << " wing:     dCmdam0 = " << left << setw(15) << dCmdam0 << left << setw(15) << " Cmm00 = " << Cmm00 << endl;
    cout << "canar:     dCmdac0 = " << left << setw(15) << dCmdac0 << left << setw(15) << " Cmc00 = " << Cmc00 << endl;
    cout << "total:       dCmda = " << left << setw(15) << dCmda << left << setw(15) << " Cm0   = " << Cm0 << endl;
    cout << "**estimate aerodynamic center location:" << endl;
    cout << "           xac (m) = " << left << setw(15) << xac << left << setw(15) << "  xcg (m) = " << xcg << endl;
}

int canareq::outputPolarPrandtline() {
    ofstream file;

    // open new file
    file.open(filenamePolarPrandtline, std::fstream::out);
    if (!file.is_open()) {
        printf("unable to write outputPolarPrandtline()\n");
        return 0;
    }

    // write results to file
    for (k=0; k<kx; k++) {
        file << std::setprecision(8);
        file << left << setw(16) << cx[k];
        file << left << setw(16) << cz[k];
        file << left << setw(16) << cq[k];
        file << left << inc[k];
    }
    file.close();
    return 1;
}

int canareq::outputPolarCdClCq() {
    ofstream file;

    // open new file
    file.open(filenamePolarCdClCq, std::fstream::out);
    if (!file.is_open()) {
        printf("unable to write outputPolarPrandtline()\n");
        return 0;
    }

    // write results to file
    for (k=0; k<kx; k++) {
        file << std::setprecision(8);
        file << left << setw(16) << cx[k];
        file << left << setw(16) << cz[k];
        file << left << setw(16) << cq[k];
        file << left << inc[k];
    }
    file.close();
    //file << "\n" << title << "\n";
    //file << "*************wing polar (profile data from Xfoil):";
    return 1;
}

int canareq::outputEqList(){
    // formerly known as write(24,...)
    ofstream file;

    // open new file
    file.open(filenameEqList, std::fstream::out);
    if (!file.is_open()) {
        printf("unable to write outputEqList()\n");
        return 0;
    }
    file << std::setprecision(8);
    file << "**********************************************" << endl;
    file << "zero incidence aerodynamic coefficients at k0=" << k0 << endl;
    file << "**************lift:" << endl;
    file << " wing:     dCldam0 = " << left << setw(10) << dCldam0 << "   Clm00 = " << Clm00 << endl;
    file << "canar:     dCldac0 = " << left << setw(10) << dCldac0 << "   Clc00 = " << Clc00 << endl;
    file << "total:       dClda = " << left << setw(10) << dClda   << "     Cl0 = " << Cl0 << endl;
    file << "*****moment at xac:" << endl;
    file << " wing:   dCmacdam0 = " << left << setw(10) << dCmacdam0 << " Cmacm00 = " << Cmacm00 << endl;
    file << "canar:   dCmacdac0 = " << left << setw(10) << dCmacdac0 << " Cmacc00 = " << Cmacc00 << endl;
    file << "total:     dCmacda = " << left << setw(10) << dCmacda << "   Cmac0 = " << Cmac0 << endl;
    file << "*****moment at xcg:" << endl;
    file << " wing:     dCmdam0 = " << left << setw(10) << dCmdam0 << "   Cmm00 = " << Cmm00 << endl;
    file << "canar:     dCmdac0 = " << left << setw(10) << dCmdac0 << "   Cmc00 = " << Cmc00 << endl;
    file << "total:       dCmda = " << left << setw(10) << dCmda << "     Cm0 = " << Cm0 << endl;
    file << "**estimate aerodynamic center location:" << endl;
    file << "               xac = " << left << setw(10) << xac << " (m)  xcg = " << xcg << " (m)" << endl;

    file << "\n**********************************************" << endl;
    file << " aleq = " << aleq << " Ueq = " << Ueq << "beteq=" << beteq << endl;
    file << " Cleq = " << Cleq << " Cdeq = " << Cdeq << " Cteq = " << Cteq << " CM,ac=" << Cmac << endl;

    return 1;
}