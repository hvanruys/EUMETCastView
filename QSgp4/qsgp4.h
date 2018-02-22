#ifndef QSGP4_H
#define QSGP4_H

#include "qtle.h"
#include "Vectors.h"
#include "qeci.h"


//////////////////////////////////////////////////////////////////////////////
class QSgp4
{
public:

   QSgp4(const QTle &tle, QTle::eOperationmode opsmode = QTle::opsmode_afspc_code);
   virtual ~QSgp4();
   bool getPosition(double tsince, QEci &eci);
   QTle        m_tle;
   double r[3], v[3];

private:
    bool sgp4init( const double epoch, QTle::eOperationmode opsmode );
    void initl( double epoch );

    void dscom
    (double epoch,  double ep,     double argpp,   double tc,     double inclp,
     double nodep,  double np);

    void dpper(char init);
    void dsinit();

    void dspace(
            double& atime, double& em,    double& argpm,  double& inclm, double& xli,
            double& mm,    double& xni,   double& nodem,  double& dndt,  double& nm );

    double ao, ainv,   con42, cosio, sinio, cosio2, eccsq,
    omeosq, posq,   rp,     rteosq,
    cnodm , snodm , cosim , sinim , cosomm, sinomm, cc1sq ,
    cc2   , cc3   , coef  , coef1 , cosio4, day   , dndt  ,
    em    , emsq  , eeta  , etasq , gam   , argpm , nodem ,
    inclm , mm    , nm    , perige, pinvsq, psisq , qzms24,
    rtemsq, s1    , s2    , s3    , s4    , s5    , s6    ,
    s7    , sfour , ss1   , ss2   , ss3   , ss4   , ss5   ,
    ss6   , ss7   , sz1   , sz2   , sz3   , sz11  , sz12  ,
    sz13  , sz21  , sz22  , sz23  , sz31  , sz32  , sz33  ,
    tc    , temp  , temp1 , temp2 , temp3 , tsi   , xpidot,
    xhdot1, z1    , z2    , z3    , z11   , z12   , z13   ,
    z21   , z22   , z23   , z31   , z32   , z33,
    qzms2t, ss, x2o3,
    delmotemp, qzms2ttemp, qzms24temp;

    double ep, xincp, inclp, nodep, argpp, mp;

};

#endif
