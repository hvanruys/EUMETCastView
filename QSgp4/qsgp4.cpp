#include "qsgp4.h"
#include "qeci.h"
#include <QDebug>

//////////////////////////////////////////////////////////////////////
QSgp4::QSgp4(const QTle &tle, QTle::eOperationmode opsmode) :
   m_tle(tle)
{
    // Julian day 2433281.5 = jan 0, 1950. 0 hr = dec 31, 1950 0 hr
    sgp4init( tle.jdsatepoch-2433281.5, opsmode);
}

/////////////////////////////////////////////////////////////////////////////
QSgp4::~QSgp4()
{

}

/*-----------------------------------------------------------------------------
*
*                             procedure sgp4init
*
*  this procedure initializes variables for sgp4.
*
*  author        : david vallado                  719-573-2600   28 jun 2005
*
*  inputs        :
*    opsmode     - mode of operation afspc or improved 'a', 'i'
*    whichconst  - which set of constants to use  72, 84
*    satn        - satellite number
*    bstar       - sgp4 type drag coefficient              kg/m2er
*    ecco        - eccentricity
*    epoch       - epoch time in days from jan 0, 1950. 0 hr
*    argpo       - argument of perigee (output if ds)
*    inclo       - inclination
*    mo          - mean anomaly (output if ds)
*    no          - mean motion
*    nodeo       - right ascension of ascending node
*
*  outputs       :
*    satrec      - common values for subsequent calls
*    return code - non-zero on error.
*                   1 - mean elements, ecc >= 1.0 or ecc < -0.001 or a < 0.95 er
*                   2 - mean motion less than 0.0
*                   3 - pert elements, ecc < 0.0  or  ecc > 1.0
*                   4 - semi-latus rectum < 0.0
*                   5 - epoch elements are sub-orbital
*                   6 - satellite has decayed
*
*  locals        :
*    cnodm  , snodm  , cosim  , sinim  , cosomm , sinomm
*    cc1sq  , cc2    , cc3
*    coef   , coef1
*    cosio4      -
*    day         -
*    dndt        -
*    em          - eccentricity
*    emsq        - eccentricity squared
*    eeta        -
*    etasq       -
*    gam         -
*    argpm       - argument of perigee
*    nodem       -
*    inclm       - inclination
*    mm          - mean anomaly
*    nm          - mean motion
*    perige      - perigee
*    pinvsq      -
*    psisq       -
*    qzms24      -
*    rtemsq      -
*    s1, s2, s3, s4, s5, s6, s7          -
*    sfour       -
*    ss1, ss2, ss3, ss4, ss5, ss6, ss7         -
*    sz1, sz2, sz3
*    sz11, sz12, sz13, sz21, sz22, sz23, sz31, sz32, sz33        -
*    tc          -
*    temp        -
*    temp1, temp2, temp3       -
*    tsi         -
*    xpidot      -
*    xhdot1      -
*    z1, z2, z3          -
*    z11, z12, z13, z21, z22, z23, z31, z32, z33         -
*
*  coupling      :
*    getgravconst-
*    initl       -
*    dscom       -
*    dpper       -
*    dsinit      -
*    sgp4        -
*
*  references    :
*    hoots, roehrich, norad spacetrack report #3 1980
*    hoots, norad spacetrack report #6 1986
*    hoots, schumacher and glover 2004
*    vallado, crawford, hujsak, kelso  2006
  ----------------------------------------------------------------------------*/

bool QSgp4::sgp4init(const double epoch , QTle::eOperationmode opsmode)
{
    /* --------------------- local variables ------------------------ */
/*    double ao, ainv,   con42, cosio, sinio, cosio2, eccsq,
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
            qzms2t, ss, x2o3, r[3], v[3],
            delmotemp, qzms2ttemp, qzms24temp;
*/
    double cc1sq;
    /* ------------------------ initialization --------------------- */
    // sgp4fix divisor for divide by zero check on inclination
    // the old check used 1.0 + cos(pi-1.0e-9), but then compared it to
    // 1.5 e-12, so the threshold was changed to 1.5e-12 for consistency
    const double temp4    =   1.5e-12;

    /* ----------- set all near earth variables to zero ------------ */
    m_tle.isimp   = 0;   m_tle.method = 'n'; m_tle.aycof    = 0.0;
    m_tle.con41   = 0.0; m_tle.cc1    = 0.0; m_tle.cc4      = 0.0;
    m_tle.cc5     = 0.0; m_tle.d2     = 0.0; m_tle.d3       = 0.0;
    m_tle.d4      = 0.0; m_tle.delmo  = 0.0; m_tle.eta      = 0.0;
    m_tle.argpdot = 0.0; m_tle.omgcof = 0.0; m_tle.sinmao   = 0.0;
    m_tle.t       = 0.0; m_tle.t2cof  = 0.0; m_tle.t3cof    = 0.0;
    m_tle.t4cof   = 0.0; m_tle.t5cof  = 0.0; m_tle.x1mth2   = 0.0;
    m_tle.x7thm1  = 0.0; m_tle.mdot   = 0.0; m_tle.nodedot  = 0.0;
    m_tle.xlcof   = 0.0; m_tle.xmcof  = 0.0; m_tle.nodecf   = 0.0;

    /* ----------- set all deep space variables to zero ------------ */
    m_tle.irez  = 0;   m_tle.d2201 = 0.0; m_tle.d2211 = 0.0;
    m_tle.d3210 = 0.0; m_tle.d3222 = 0.0; m_tle.d4410 = 0.0;
    m_tle.d4422 = 0.0; m_tle.d5220 = 0.0; m_tle.d5232 = 0.0;
    m_tle.d5421 = 0.0; m_tle.d5433 = 0.0; m_tle.dedt  = 0.0;
    m_tle.del1  = 0.0; m_tle.del2  = 0.0; m_tle.del3  = 0.0;
    m_tle.didt  = 0.0; m_tle.dmdt  = 0.0; m_tle.dnodt = 0.0;
    m_tle.domdt = 0.0; m_tle.e3    = 0.0; m_tle.ee2   = 0.0;
    m_tle.peo   = 0.0; m_tle.pgho  = 0.0; m_tle.pho   = 0.0;
    m_tle.pinco = 0.0; m_tle.plo   = 0.0; m_tle.se2   = 0.0;
    m_tle.se3   = 0.0; m_tle.sgh2  = 0.0; m_tle.sgh3  = 0.0;
    m_tle.sgh4  = 0.0; m_tle.sh2   = 0.0; m_tle.sh3   = 0.0;
    m_tle.si2   = 0.0; m_tle.si3   = 0.0; m_tle.sl2   = 0.0;
    m_tle.sl3   = 0.0; m_tle.sl4   = 0.0; m_tle.gsto  = 0.0;
    m_tle.xfact = 0.0; m_tle.xgh2  = 0.0; m_tle.xgh3  = 0.0;
    m_tle.xgh4  = 0.0; m_tle.xh2   = 0.0; m_tle.xh3   = 0.0;
    m_tle.xi2   = 0.0; m_tle.xi3   = 0.0; m_tle.xl2   = 0.0;
    m_tle.xl3   = 0.0; m_tle.xl4   = 0.0; m_tle.xlamo = 0.0;
    m_tle.zmol  = 0.0; m_tle.zmos  = 0.0; m_tle.atime = 0.0;
    m_tle.xli   = 0.0; m_tle.xni   = 0.0;


    // sgp4fix add opsmode
    m_tle.operationmode = opsmode;

    ss     = 78.0 / m_tle.radiusearthkm + 1.0;
    // sgp4fix use multiply for speed instead of pow
    qzms2ttemp = (120.0 - 78.0) / m_tle.radiusearthkm;
    qzms2t = qzms2ttemp * qzms2ttemp * qzms2ttemp * qzms2ttemp;
    x2o3   =  2.0 / 3.0;

    m_tle.init = 'y';
    m_tle.t	 = 0.0;


    initl( epoch );

    m_tle.error = 0;
    m_tle.errormsg = "";

    // sgp4fix remove this check as it is unnecessary
    // the mrt check in sgp4 handles decaying satellite cases even if the starting
    // condition is below the surface of te earth
    //     if (rp < 1.0)
    //       {
    //         printf("# *** satn%d epoch elts sub-orbital ***\n", satn);
    //         satrec.error = 5;
    //       }

    if ((omeosq >= 0.0 ) || ( m_tle.no >= 0.0))
    {
        m_tle.isimp = 0;
        if (rp < (220.0 / m_tle.radiusearthkm + 1.0))
            m_tle.isimp = 1;
        sfour  = ss;
        qzms24 = qzms2t;
        perige = (rp - 1.0) * m_tle.radiusearthkm;

        /* - for perigees below 156 km, s and qoms2t are altered - */
        if (perige < 156.0)
        {
            sfour = perige - 78.0;
            if (perige < 98.0)
                sfour = 20.0;
            // sgp4fix use multiply for speed instead of pow
            qzms24temp =  (120.0 - sfour) / m_tle.radiusearthkm;
            qzms24 = qzms24temp * qzms24temp * qzms24temp * qzms24temp;
            sfour  = sfour / m_tle.radiusearthkm + 1.0;
        }
        pinvsq = 1.0 / posq;

        tsi  = 1.0 / (ao - sfour);
        m_tle.eta  = ao * m_tle.ecco * tsi;
        etasq = m_tle.eta * m_tle.eta;
        eeta  = m_tle.ecco * m_tle.eta;
        psisq = fabs(1.0 - etasq);
        coef  = qzms24 * pow(tsi, 4.0);
        coef1 = coef / pow(psisq, 3.5);
        cc2   = coef1 * m_tle.no * (ao * (1.0 + 1.5 * etasq + eeta *
                                           (4.0 + etasq)) + 0.375 * m_tle.j2 * tsi / psisq * m_tle.con41 *
                                     (8.0 + 3.0 * etasq * (8.0 + etasq)));
        m_tle.cc1   = m_tle.bstar * cc2;
        cc3   = 0.0;
        if (m_tle.ecco > 1.0e-4)
            cc3 = -2.0 * coef * tsi * m_tle.j3oj2 * m_tle.no * sinio / m_tle.ecco;
        m_tle.x1mth2 = 1.0 - cosio2;
        m_tle.cc4    = 2.0* m_tle.no * coef1 * ao * omeosq *
                (m_tle.eta * (2.0 + 0.5 * etasq) + m_tle.ecco *
                 (0.5 + 2.0 * etasq) - m_tle.j2 * tsi / (ao * psisq) *
                 (-3.0 * m_tle.con41 * (1.0 - 2.0 * eeta + etasq *
                                         (1.5 - 0.5 * eeta)) + 0.75 * m_tle.x1mth2 *
                  (2.0 * etasq - eeta * (1.0 + etasq)) * cos(2.0 * m_tle.argpo)));
        m_tle.cc5 = 2.0 * coef1 * ao * omeosq * (1.0 + 2.75 *
                                                  (etasq + eeta) + eeta * etasq);
        cosio4 = cosio2 * cosio2;
        temp1  = 1.5 * m_tle.j2 * pinvsq * m_tle.no;
        temp2  = 0.5 * temp1 * m_tle.j2 * pinvsq;
        temp3  = -0.46875 * m_tle.j4 * pinvsq * pinvsq * m_tle.no;
        m_tle.mdot     = m_tle.no + 0.5 * temp1 * rteosq * m_tle.con41 + 0.0625 *
                temp2 * rteosq * (13.0 - 78.0 * cosio2 + 137.0 * cosio4);
        m_tle.argpdot  = -0.5 * temp1 * con42 + 0.0625 * temp2 *
                (7.0 - 114.0 * cosio2 + 395.0 * cosio4) +
                temp3 * (3.0 - 36.0 * cosio2 + 49.0 * cosio4);
        xhdot1            = -temp1 * cosio;
        m_tle.nodedot = xhdot1 + (0.5 * temp2 * (4.0 - 19.0 * cosio2) +
                                   2.0 * temp3 * (3.0 - 7.0 * cosio2)) * cosio;
        xpidot            =  m_tle.argpdot+ m_tle.nodedot;
        m_tle.omgcof   = m_tle.bstar * cc3 * cos(m_tle.argpo);
        m_tle.xmcof    = 0.0;
        if (m_tle.ecco > 1.0e-4)
            m_tle.xmcof = -x2o3 * coef * m_tle.bstar / eeta;
        m_tle.nodecf = 3.5 * omeosq * xhdot1 * m_tle.cc1;
        m_tle.t2cof   = 1.5 * m_tle.cc1;
        // sgp4fix for divide by zero with xinco = 180 deg
        if (fabs(cosio+1.0) > 1.5e-12)
            m_tle.xlcof = -0.25 * m_tle.j3oj2 * sinio * (3.0 + 5.0 * cosio) / (1.0 + cosio);
        else
            m_tle.xlcof = -0.25 * m_tle.j3oj2 * sinio * (3.0 + 5.0 * cosio) / temp4;
        m_tle.aycof   = -0.5 * m_tle.j3oj2 * sinio;
        // sgp4fix use multiply for speed instead of pow
        delmotemp = 1.0 + m_tle.eta * cos(m_tle.mo);
        m_tle.delmo   = delmotemp * delmotemp * delmotemp;
        m_tle.sinmao  = sin(m_tle.mo);
        m_tle.x7thm1  = 7.0 * cosio2 - 1.0;

        /* --------------- deep space initialization ------------- */
        if ((2*PI / m_tle.no) >= 225.0)
        {
            m_tle.method = 'd';
            m_tle.isimp  = 1;
            tc    =  0.0;
            inclm = m_tle.inclo;


            dscom( epoch, m_tle.ecco, m_tle.argpo, tc, m_tle.inclo,
                        m_tle.nodeo, m_tle.no
                        );

            ep = m_tle.ecco;
            inclp = m_tle.inclo;
            nodep = m_tle.nodeo;
            argpp = m_tle.argpo;
            mp = m_tle.mo;

            dpper( m_tle.init);

            m_tle.ecco = ep;
            m_tle.inclo = inclp;
            m_tle.nodeo = nodep;
            m_tle.argpo = argpp;
            m_tle.mo = mp;

            argpm  = 0.0;
            nodem  = 0.0;
            mm     = 0.0;

            dsinit();
        }

        /* ----------- set variables if not deep space ----------- */
        if (m_tle.isimp != 1)
        {
            cc1sq          = m_tle.cc1 * m_tle.cc1;
            m_tle.d2    = 4.0 * ao * tsi * cc1sq;
            temp           = m_tle.d2 * tsi * m_tle.cc1 / 3.0;
            m_tle.d3    = (17.0 * ao + sfour) * temp;
            m_tle.d4    = 0.5 * temp * ao * tsi * (221.0 * ao + 31.0 * sfour) *
                    m_tle.cc1;
            m_tle.t3cof = m_tle.d2 + 2.0 * cc1sq;
            m_tle.t4cof = 0.25 * (3.0 * m_tle.d3 + m_tle.cc1 *
                                   (12.0 * m_tle.d2 + 10.0 * cc1sq));
            m_tle.t5cof = 0.2 * (3.0 * m_tle.d4 +
                                  12.0 * m_tle.cc1 * m_tle.d3 +
                                  6.0 * m_tle.d2 * m_tle.d2 +
                                  15.0 * cc1sq * (2.0 * m_tle.d2 + cc1sq));
        }
    } // if omeosq = 0 ...

    /* finally propogate to zero epoch to initialize all others. */
    // sgp4fix take out check to let satellites process until they are actually below earth surface
    //       if(satrec.error == 0)

    QEci eci;
    getPosition( 0.0, eci);

    m_tle.init = 'n';

    //printf("in qsgp4.cpp");
    //#include "debug6.cpp"
    //sgp4fix return boolean. satrec.error contains any error codes
    return true;
}  // end sgp4init


/*-----------------------------------------------------------------------------
*
*                           procedure initl
*
*  this procedure initializes the spg4 propagator. all the initialization is
*    consolidated here instead of having multiple loops inside other routines.
*
*  author        : david vallado                  719-573-2600   28 jun 2005
*
*  inputs        :
*    ecco        - eccentricity                           0.0 - 1.0
*    epoch       - epoch time in days from jan 0, 1950. 0 hr
*    inclo       - inclination of satellite
*    no          - mean motion of satellite
*    satn        - satellite number
*
*  outputs       :
*    ainv        - 1.0 / a
*    ao          - semi major axis
*    con41       -
*    con42       - 1.0 - 5.0 cos(i)
*    cosio       - cosine of inclination
*    cosio2      - cosio squared
*    eccsq       - eccentricity squared
*    method      - flag for deep space                    'd', 'n'
*    omeosq      - 1.0 - ecco * ecco
*    posq        - semi-parameter squared
*    rp          - radius of perigee
*    rteosq      - square root of (1.0 - ecco*ecco)
*    sinio       - sine of inclination
*    gsto        - gst at time of observation               rad
*    no          - mean motion of satellite
*
*  locals        :
*    ak          -
*    d1          -
*    del         -
*    adel        -
*    po          -
*
*  coupling      :
*    getgravconst
*    gstime      - find greenwich sidereal time from the julian date
*
*  references    :
*    hoots, roehrich, norad spacetrack report #3 1980
*    hoots, norad spacetrack report #6 1986
*    hoots, schumacher and glover 2004
*    vallado, crawford, hujsak, kelso  2006
  ----------------------------------------------------------------------------*/
/* initl(satn, m_tle.ecco, epoch, m_tle.inclo,
            m_tle.no, m_tle.method,
            ainv, ao, m_tle.con41, con42, cosio, cosio2, eccsq, omeosq,
            posq, rp, rteosq, sinio, m_tle.gsto, m_tle.operationmode
            );
*/

void QSgp4::initl(double epoch )
{
     // --------------------- local variables ------------------------
     double ak, d1, del, adel, po, x2o3;

     // sgp4fix use old way of finding gst
     double ds70;
     double ts70, tfrac, c1, thgr70, fk5r, c1p2p;

     // ----------------------- earth constants ----------------------
     // sgp4fix identify constants and allow alternate values
     //m_tle.getgravconst( whichconst, tumin, mu, radiusearthkm, xke, j2, j3, j4, j3oj2 );

     x2o3   = 2.0 / 3.0;

     // ------------- calculate auxillary epoch quantities ----------
     eccsq  = m_tle.ecco * m_tle.ecco;
     omeosq = 1.0 - eccsq;
     rteosq = sqrt(omeosq);
     cosio  = cos(m_tle.inclo);
     cosio2 = cosio * cosio;

     // ------------------ un-kozai the mean motion -----------------
     ak    = pow(m_tle.xke / m_tle.no, x2o3);
     d1    = 0.75 * m_tle.j2 * (3.0 * cosio2 - 1.0) / (rteosq * omeosq);
     del   = d1 / (ak * ak);
     adel  = ak * (1.0 - del * del - del *
             (1.0 / 3.0 + 134.0 * del * del / 81.0));
     del   = d1/(adel * adel);
     m_tle.no    = m_tle.no / (1.0 + del);

     ao    = pow(m_tle.xke / m_tle.no, x2o3);
     sinio = sin(m_tle.inclo);
     po    = ao * omeosq;
     con42 = 1.0 - 5.0 * cosio2;
     m_tle.con41 = -con42-cosio2-cosio2;
     ainv  = 1.0 / ao;
     posq  = po * po;
     rp    = ao * (1.0 - m_tle.ecco);
     m_tle.method = 'n';

     // sgp4fix modern approach to finding sidereal time
     if (m_tle.operationmode == QTle::opsmode_afspc_code)
        {
         // sgp4fix use old way of finding gst
         // count integer number of days from 0 jan 1970
         ts70  = epoch - 7305.0;
         ds70 = floor(ts70 + 1.0e-8);
         tfrac = ts70 - ds70;
         // find greenwich location at epoch
         c1    = 1.72027916940703639e-2;
         thgr70= 1.7321343856509374;
         fk5r  = 5.07551419432269442e-15;
         c1p2p = c1 + TWOPI;
         m_tle.gsto  = fmod( thgr70 + c1*ds70 + c1p2p*tfrac + ts70*ts70*fk5r, TWOPI);
         if ( m_tle.gsto < 0.0 )
             m_tle.gsto = m_tle.gsto + TWOPI;
       }
       else
        m_tle.gsto = m_tle.gstime(epoch + 2433281.5);

//printf("in qsgp4.cpp\n");
//#include "qsgpdebug5.cpp"
}  // end initl


/*-----------------------------------------------------------------------------
*
*                           procedure dscom
*
*  this procedure provides deep space common items used by both the secular
*    and periodics subroutines.  input is provided as shown. this routine
*    used to be called dpper, but the functions inside weren't well organized.
*
*  author        : david vallado                  719-573-2600   28 jun 2005
*
*  inputs        :
*    epoch       -
*    ep          - eccentricity
*    argpp       - argument of perigee
*    tc          -
*    inclp       - inclination
*    nodep       - right ascension of ascending node
*    np          - mean motion
*
*  outputs       :
*    sinim  , cosim  , sinomm , cosomm , snodm  , cnodm
*    day         -
*    e3          -
*    ee2         -
*    em          - eccentricity
*    emsq        - eccentricity squared
*    gam         -
*    peo         -
*    pgho        -
*    pho         -
*    pinco       -
*    plo         -
*    rtemsq      -
*    se2, se3         -
*    sgh2, sgh3, sgh4        -
*    sh2, sh3, si2, si3, sl2, sl3, sl4         -
*    s1, s2, s3, s4, s5, s6, s7          -
*    ss1, ss2, ss3, ss4, ss5, ss6, ss7, sz1, sz2, sz3         -
*    sz11, sz12, sz13, sz21, sz22, sz23, sz31, sz32, sz33        -
*    xgh2, xgh3, xgh4, xh2, xh3, xi2, xi3, xl2, xl3, xl4         -
*    nm          - mean motion
*    z1, z2, z3, z11, z12, z13, z21, z22, z23, z31, z32, z33         -
*    zmol        -
*    zmos        -
*
*  locals        :
*    a1, a2, a3, a4, a5, a6, a7, a8, a9, a10         -
*    betasq      -
*    cc          -
*    ctem, stem        -
*    x1, x2, x3, x4, x5, x6, x7, x8          -
*    xnodce      -
*    xnoi        -
*    zcosg  , zsing  , zcosgl , zsingl , zcosh  , zsinh  , zcoshl , zsinhl ,
*    zcosi  , zsini  , zcosil , zsinil ,
*    zx          -
*    zy          -
*
*  coupling      :
*    none.
*
*  references    :
*    hoots, roehrich, norad spacetrack report #3 1980
*    hoots, norad spacetrack report #6 1986
*    hoots, schumacher and glover 2004
*    vallado, crawford, hujsak, kelso  2006
  ----------------------------------------------------------------------------*/

void QSgp4::dscom
     (
       double epoch,  double ep,     double argpp,   double tc,     double inclp,
       double nodep,  double np
     )
{
     /* -------------------------- constants ------------------------- */
     const double zes     =  0.01675;
     const double zel     =  0.05490;
     const double c1ss    =  2.9864797e-6;
     const double c1l     =  4.7968065e-7;
     const double zsinis  =  0.39785416;
     const double zcosis  =  0.91744867;
     const double zcosgs  =  0.1945905;
     const double zsings  = -0.98088458;

     /* --------------------- local variables ------------------------ */
     int lsflg;
     double a1    , a2    , a3    , a4    , a5    , a6    , a7    ,
        a8    , a9    , a10   , betasq, cc    , ctem  , stem  ,
        x1    , x2    , x3    , x4    , x5    , x6    , x7    ,
        x8    , xnodce, xnoi  , zcosg , zcosgl, zcosh , zcoshl,
        zcosi , zcosil, zsing , zsingl, zsinh , zsinhl, zsini ,
        zsinil, zx    , zy;

     nm     = np;
     em     = ep;
     snodm  = sin(nodep);
     cnodm  = cos(nodep);
     sinomm = sin(argpp);
     cosomm = cos(argpp);
     sinim  = sin(inclp);
     cosim  = cos(inclp);
     emsq   = em * em;
     betasq = 1.0 - emsq;
     rtemsq = sqrt(betasq);

     /* ----------------- initialize lunar solar terms --------------- */
     m_tle.peo    = 0.0;
     m_tle.pinco  = 0.0;
     m_tle.plo    = 0.0;
     m_tle.pgho   = 0.0;
     m_tle.pho    = 0.0;
     day    = epoch + 18261.5 + tc / 1440.0;
     xnodce = fmod(4.5236020 - 9.2422029e-4 * day, TWOPI);
     stem   = sin(xnodce);
     ctem   = cos(xnodce);
     zcosil = 0.91375164 - 0.03568096 * ctem;
     zsinil = sqrt(1.0 - zcosil * zcosil);
     zsinhl = 0.089683511 * stem / zsinil;
     zcoshl = sqrt(1.0 - zsinhl * zsinhl);
     gam    = 5.8351514 + 0.0019443680 * day;
     zx     = 0.39785416 * stem / zsinil;
     zy     = zcoshl * ctem + 0.91744867 * zsinhl * stem;
     zx     = atan2(zx, zy);
     zx     = gam + zx - xnodce;
     zcosgl = cos(zx);
     zsingl = sin(zx);

     /* ------------------------- do solar terms --------------------- */
     zcosg = zcosgs;
     zsing = zsings;
     zcosi = zcosis;
     zsini = zsinis;
     zcosh = cnodm;
     zsinh = snodm;
     cc    = c1ss;
     xnoi  = 1.0 / nm;

     for (lsflg = 1; lsflg <= 2; lsflg++)
       {
         a1  =   zcosg * zcosh + zsing * zcosi * zsinh;
         a3  =  -zsing * zcosh + zcosg * zcosi * zsinh;
         a7  =  -zcosg * zsinh + zsing * zcosi * zcosh;
         a8  =   zsing * zsini;
         a9  =   zsing * zsinh + zcosg * zcosi * zcosh;
         a10 =   zcosg * zsini;
         a2  =   cosim * a7 + sinim * a8;
         a4  =   cosim * a9 + sinim * a10;
         a5  =  -sinim * a7 + cosim * a8;
         a6  =  -sinim * a9 + cosim * a10;

         x1  =  a1 * cosomm + a2 * sinomm;
         x2  =  a3 * cosomm + a4 * sinomm;
         x3  = -a1 * sinomm + a2 * cosomm;
         x4  = -a3 * sinomm + a4 * cosomm;
         x5  =  a5 * sinomm;
         x6  =  a6 * sinomm;
         x7  =  a5 * cosomm;
         x8  =  a6 * cosomm;

         z31 = 12.0 * x1 * x1 - 3.0 * x3 * x3;
         z32 = 24.0 * x1 * x2 - 6.0 * x3 * x4;
         z33 = 12.0 * x2 * x2 - 3.0 * x4 * x4;
         z1  =  3.0 *  (a1 * a1 + a2 * a2) + z31 * emsq;
         z2  =  6.0 *  (a1 * a3 + a2 * a4) + z32 * emsq;
         z3  =  3.0 *  (a3 * a3 + a4 * a4) + z33 * emsq;
         z11 = -6.0 * a1 * a5 + emsq *  (-24.0 * x1 * x7-6.0 * x3 * x5);
         z12 = -6.0 *  (a1 * a6 + a3 * a5) + emsq *
                (-24.0 * (x2 * x7 + x1 * x8) - 6.0 * (x3 * x6 + x4 * x5));
         z13 = -6.0 * a3 * a6 + emsq * (-24.0 * x2 * x8 - 6.0 * x4 * x6);
         z21 =  6.0 * a2 * a5 + emsq * (24.0 * x1 * x5 - 6.0 * x3 * x7);
         z22 =  6.0 *  (a4 * a5 + a2 * a6) + emsq *
                (24.0 * (x2 * x5 + x1 * x6) - 6.0 * (x4 * x7 + x3 * x8));
         z23 =  6.0 * a4 * a6 + emsq * (24.0 * x2 * x6 - 6.0 * x4 * x8);
         z1  = z1 + z1 + betasq * z31;
         z2  = z2 + z2 + betasq * z32;
         z3  = z3 + z3 + betasq * z33;
         s3  = cc * xnoi;
         s2  = -0.5 * s3 / rtemsq;
         s4  = s3 * rtemsq;
         s1  = -15.0 * em * s4;
         s5  = x1 * x3 + x2 * x4;
         s6  = x2 * x3 + x1 * x4;
         s7  = x2 * x4 - x1 * x3;

         /* ----------------------- do lunar terms ------------------- */
         if (lsflg == 1)
           {
             ss1   = s1;
             ss2   = s2;
             ss3   = s3;
             ss4   = s4;
             ss5   = s5;
             ss6   = s6;
             ss7   = s7;
             sz1   = z1;
             sz2   = z2;
             sz3   = z3;
             sz11  = z11;
             sz12  = z12;
             sz13  = z13;
             sz21  = z21;
             sz22  = z22;
             sz23  = z23;
             sz31  = z31;
             sz32  = z32;
             sz33  = z33;
             zcosg = zcosgl;
             zsing = zsingl;
             zcosi = zcosil;
             zsini = zsinil;
             zcosh = zcoshl * cnodm + zsinhl * snodm;
             zsinh = snodm * zcoshl - cnodm * zsinhl;
             cc    = c1l;
          }
       }

     m_tle.zmol = fmod(4.7199672 + 0.22997150  * day - gam, TWOPI);
     m_tle.zmos = fmod(6.2565837 + 0.017201977 * day, TWOPI);

     /* ------------------------ do solar terms ---------------------- */
     m_tle.se2  =   2.0 * ss1 * ss6;
     m_tle.se3  =   2.0 * ss1 * ss7;
     m_tle.si2  =   2.0 * ss2 * sz12;
     m_tle.si3  =   2.0 * ss2 * (sz13 - sz11);
     m_tle.sl2  =  -2.0 * ss3 * sz2;
     m_tle.sl3  =  -2.0 * ss3 * (sz3 - sz1);
     m_tle.sl4  =  -2.0 * ss3 * (-21.0 - 9.0 * emsq) * zes;
     m_tle.sgh2 =   2.0 * ss4 * sz32;
     m_tle.sgh3 =   2.0 * ss4 * (sz33 - sz31);
     m_tle.sgh4 = -18.0 * ss4 * zes;
     m_tle.sh2  =  -2.0 * ss2 * sz22;
     m_tle.sh3  =  -2.0 * ss2 * (sz23 - sz21);

     /* ------------------------ do lunar terms ---------------------- */
     m_tle.ee2  =   2.0 * s1 * s6;
     m_tle.e3   =   2.0 * s1 * s7;
     m_tle.xi2  =   2.0 * s2 * z12;
     m_tle.xi3  =   2.0 * s2 * (z13 - z11);
     m_tle.xl2  =  -2.0 * s3 * z2;
     m_tle.xl3  =  -2.0 * s3 * (z3 - z1);
     m_tle.xl4  =  -2.0 * s3 * (-21.0 - 9.0 * emsq) * zel;
     m_tle.xgh2 =   2.0 * s4 * z32;
     m_tle.xgh3 =   2.0 * s4 * (z33 - z31);
     m_tle.xgh4 = -18.0 * s4 * zel;
     m_tle.xh2  =  -2.0 * s2 * z22;
     m_tle.xh3  =  -2.0 * s2 * (z23 - z21);

//#include "debug2.cpp"
}  // end dscom

/* -----------------------------------------------------------------------------
*
*                           procedure dpper
*
*  this procedure provides deep space long period periodic contributions
*    to the mean elements.  by design, these periodics are zero at epoch.
*    this used to be dscom which included initialization, but it's really a
*    recurring function.
*
*  author        : david vallado                  719-573-2600   28 jun 2005
*
*  inputs        :
*    e3          -
*    ee2         -
*    peo         -
*    pgho        -
*    pho         -
*    pinco       -
*    plo         -
*    se2 , se3 , sgh2, sgh3, sgh4, sh2, sh3, si2, si3, sl2, sl3, sl4 -
*    t           -
*    xh2, xh3, xi2, xi3, xl2, xl3, xl4 -
*    zmol        -
*    zmos        -
*    ep          - eccentricity                           0.0 - 1.0
*    inclo       - inclination - needed for lyddane modification
*    nodep       - right ascension of ascending node
*    argpp       - argument of perigee
*    mp          - mean anomaly
*
*  outputs       :
*    ep          - eccentricity                           0.0 - 1.0
*    inclp       - inclination
*    nodep        - right ascension of ascending node
*    argpp       - argument of perigee
*    mp          - mean anomaly
*
*  locals        :
*    alfdp       -
*    betdp       -
*    cosip  , sinip  , cosop  , sinop  ,
*    dalf        -
*    dbet        -
*    dls         -
*    f2, f3      -
*    pe          -
*    pgh         -
*    ph          -
*    pinc        -
*    pl          -
*    sel   , ses   , sghl  , sghs  , shl   , shs   , sil   , sinzf , sis   ,
*    sll   , sls
*    xls         -
*    xnoh        -
*    zf          -
*    zm          -
*
*  coupling      :
*    none.
*
*  references    :
*    hoots, roehrich, norad spacetrack report #3 1980
*    hoots, norad spacetrack report #6 1986
*    hoots, schumacher and glover 2004
*    vallado, crawford, hujsak, kelso  2006
  ----------------------------------------------------------------------------*/

void QSgp4::dpper
(
        //double e3,     double ee2,    double peo,     double pgho,   double pho,
        //double pinco,  double plo,    double se2,     double se3,    double sgh2,
        //double sgh3,   double sgh4,   double sh2,     double sh3,    double si2,
        //double si3,    double sl2,    double sl3,     double sl4,    double t,
        //double xgh2,   double xgh3,   double xgh4,    double xh2,    double xh3,
        //double xi2,    double xi3,    double xl2,     double xl3,    double xl4,
        //double zmol,   double zmos,   double inclo,
        char init
        //double& ep,    double& inclp, double& nodep,  double& argpp, double& mp
        //char opsmode
        )
{
    /* --------------------- local variables ------------------------ */
    double alfdp, betdp, cosip, cosop, dalf, dbet, dls,
            f2,    f3,    pe,    pgh,   ph,   pinc, pl ,
            sel,   ses,   sghl,  sghs,  shll, shs,  sil,
            sinip, sinop, sinzf, sis,   sll,  sls,  xls,
            xnoh,  zf,    zm,    zel,   zes,  znl,  zns;

    /* ---------------------- constants ----------------------------- */
    zns   = 1.19459e-5;
    zes   = 0.01675;
    znl   = 1.5835218e-4;
    zel   = 0.05490;

    /* --------------- calculate time varying periodics ----------- */
    zm    = m_tle.zmos + zns * m_tle.t;
    // be sure that the initial call has time set to zero
    if (init == 'y')
        zm = m_tle.zmos;
    zf    = zm + 2.0 * zes * sin(zm);
    sinzf = sin(zf);
    f2    =  0.5 * sinzf * sinzf - 0.25;
    f3    = -0.5 * sinzf * cos(zf);
    ses   = m_tle.se2* f2 + m_tle.se3 * f3;
    sis   = m_tle.si2 * f2 + m_tle.si3 * f3;
    sls   = m_tle.sl2 * f2 + m_tle.sl3 * f3 + m_tle.sl4 * sinzf;
    sghs  = m_tle.sgh2 * f2 + m_tle.sgh3 * f3 + m_tle.sgh4 * sinzf;
    shs   = m_tle.sh2 * f2 + m_tle.sh3 * f3;
    zm    = m_tle.zmol + znl * m_tle.t;
    if (init == 'y')
        zm = m_tle.zmol;
    zf    = zm + 2.0 * zel * sin(zm);
    sinzf = sin(zf);
    f2    =  0.5 * sinzf * sinzf - 0.25;
    f3    = -0.5 * sinzf * cos(zf);
    sel   = m_tle.ee2 * f2 + m_tle.e3 * f3;
    sil   = m_tle.xi2 * f2 + m_tle.xi3 * f3;
    sll   = m_tle.xl2 * f2 + m_tle.xl3 * f3 + m_tle.xl4 * sinzf;
    sghl  = m_tle.xgh2 * f2 + m_tle.xgh3 * f3 + m_tle.xgh4 * sinzf;
    shll  = m_tle.xh2 * f2 + m_tle.xh3 * f3;
    pe    = ses + sel;
    pinc  = sis + sil;
    pl    = sls + sll;
    pgh   = sghs + sghl;
    ph    = shs + shll;

    if (init == 'n')
    {
        pe    = pe - m_tle.peo;
        pinc  = pinc - m_tle.pinco;
        pl    = pl - m_tle.plo;
        pgh   = pgh - m_tle.pgho;
        ph    = ph - m_tle.pho;
        inclp = inclp + pinc;
        ep    = ep + pe;
        sinip = sin(inclp);
        cosip = cos(inclp);

        /* ----------------- apply periodics directly ------------ */
        //  sgp4fix for lyddane choice
        //  strn3 used original inclination - this is technically feasible
        //  gsfc used perturbed inclination - also technically feasible
        //  probably best to readjust the 0.2 limit value and limit discontinuity
        //  0.2 rad = 11.45916 deg
        //  use next line for original strn3 approach and original inclination
        //  if (inclo >= 0.2)
        //  use next line for gsfc version and perturbed inclination
        if (inclp >= 0.2)
        {
            ph     = ph / sinip;
            pgh    = pgh - cosip * ph;
            argpp  = argpp + pgh;
            nodep  = nodep + ph;
            mp     = mp + pl;
        }
        else
        {
            /* ---- apply periodics with lyddane modification ---- */
            sinop  = sin(nodep);
            cosop  = cos(nodep);
            alfdp  = sinip * sinop;
            betdp  = sinip * cosop;
            dalf   =  ph * cosop + pinc * cosip * sinop;
            dbet   = -ph * sinop + pinc * cosip * cosop;
            alfdp  = alfdp + dalf;
            betdp  = betdp + dbet;
            nodep  = fmod(nodep, TWOPI);
            //  sgp4fix for afspc written intrinsic functions
            // nodep used without a trigonometric function ahead
            if ((nodep < 0.0) && (m_tle.operationmode == QTle::opsmode_afspc_code))
                nodep = nodep + TWOPI;
            xls    = mp + argpp + cosip * nodep;
            dls    = pl + pgh - pinc * nodep * sinip;
            xls    = xls + dls;
            xnoh   = nodep;
            nodep  = atan2(alfdp, betdp);
            //  sgp4fix for afspc written intrinsic functions
            // nodep used without a trigonometric function ahead
            if ((nodep < 0.0) && (m_tle.operationmode == QTle::opsmode_afspc_code))
                nodep = nodep + TWOPI;
            if (fabs(xnoh - nodep) > PI)
            {
                if (nodep < xnoh)
                    nodep = nodep + TWOPI;
                else
                    nodep = nodep - TWOPI;
            }
            mp    = mp + pl;
            argpp = xls - mp - cosip * nodep;
        }
    }   // if init == 'n'

    //#include "debug1.cpp"
}  // end dpper


/*-----------------------------------------------------------------------------
*
*                           procedure dsinit
*
*  this procedure provides deep space contributions to mean motion dot due
*    to geopotential resonance with half day and one day orbits.
*
*  author        : david vallado                  719-573-2600   28 jun 2005
*
*  inputs        :
*    cosim, sinim-
*    emsq        - eccentricity squared
*    argpo       - argument of perigee
*    s1, s2, s3, s4, s5      -
*    ss1, ss2, ss3, ss4, ss5 -
*    sz1, sz3, sz11, sz13, sz21, sz23, sz31, sz33 -
*    t           - time
*    tc          -
*    gsto        - greenwich sidereal time                   rad
*    mo          - mean anomaly
*    mdot        - mean anomaly dot (rate)
*    no          - mean motion
*    nodeo       - right ascension of ascending node
*    nodedot     - right ascension of ascending node dot (rate)
*    xpidot      -
*    z1, z3, z11, z13, z21, z23, z31, z33 -
*    eccm        - eccentricity
*    argpm       - argument of perigee
*    inclm       - inclination
*    mm          - mean anomaly
*    xn          - mean motion
*    nodem       - right ascension of ascending node
*
*  outputs       :
*    em          - eccentricity
*    argpm       - argument of perigee
*    inclm       - inclination
*    mm          - mean anomaly
*    nm          - mean motion
*    nodem       - right ascension of ascending node
*    irez        - flag for resonance           0-none, 1-one day, 2-half day
*    atime       -
*    d2201, d2211, d3210, d3222, d4410, d4422, d5220, d5232, d5421, d5433    -
*    dedt        -
*    didt        -
*    dmdt        -
*    dndt        -
*    dnodt       -
*    domdt       -
*    del1, del2, del3        -
*    ses  , sghl , sghs , sgs  , shl  , shs  , sis  , sls
*    theta       -
*    xfact       -
*    xlamo       -
*    xli         -
*    xni
*
*  locals        :
*    ainv2       -
*    aonv        -
*    cosisq      -
*    eoc         -
*    f220, f221, f311, f321, f322, f330, f441, f442, f522, f523, f542, f543  -
*    g200, g201, g211, g300, g310, g322, g410, g422, g520, g521, g532, g533  -
*    sini2       -
*    temp        -
*    temp1       -
*    theta       -
*    xno2        -
*
*  coupling      :
*    getgravconst
*
*  references    :
*    hoots, roehrich, norad spacetrack report #3 1980
*    hoots, norad spacetrack report #6 1986
*    hoots, schumacher and glover 2004
*    vallado, crawford, hujsak, kelso  2006
  ----------------------------------------------------------------------------*/
void QSgp4::dsinit()
{
     // --------------------- local variables ------------------------

     double ainv2 , aonv=0.0, cosisq, eoc, f220 , f221  , f311  ,
          f321  , f322  , f330  , f441  , f442  , f522  , f523  ,
          f542  , f543  , g200  , g201  , g211  , g300  , g310  ,
          g322  , g410  , g422  , g520  , g521  , g532  , g533  ,
          ses   , sgs   , sghl  , sghs  , shs   , shll  , sis   ,
          sini2 , sls   , temp  , temp1 , theta , xno2  , q22   ,
          q31   , q33   , root22, root44, root54, rptim , root32,
          root52, x2o3  , znl   , emo   , zns   , emsqo;

     q22    = 1.7891679e-6;
     q31    = 2.1460748e-6;
     q33    = 2.2123015e-7;
     root22 = 1.7891679e-6;
     root44 = 7.3636953e-9;
     root54 = 2.1765803e-9;
     rptim  = 4.37526908801129966e-3; // this equates to 7.29211514668855e-5 rad/sec
     root32 = 3.7393792e-7;
     root52 = 1.1428639e-7;
     x2o3   = 2.0 / 3.0;
     znl    = 1.5835218e-4;
     zns    = 1.19459e-5;

     // -------------------- deep space initialization ------------
     m_tle.irez = 0;
     if ((nm < 0.0052359877) && (nm > 0.0034906585))
         m_tle.irez = 1;
     if ((nm >= 8.26e-3) && (nm <= 9.24e-3) && (em >= 0.5))
         m_tle.irez = 2;

     // ------------------------ do solar terms -------------------
     ses  =  ss1 * zns * ss5;
     sis  =  ss2 * zns * (sz11 + sz13);
     sls  = -zns * ss3 * (sz1 + sz3 - 14.0 - 6.0 * emsq);
     sghs =  ss4 * zns * (sz31 + sz33 - 6.0);
     shs  = -zns * ss2 * (sz21 + sz23);
     // sgp4fix for 180 deg incl
     if ((inclm < 5.2359877e-2) || (inclm > PI - 5.2359877e-2))
       shs = 0.0;
     if (sinim != 0.0)
       shs = shs / sinim;
     sgs  = sghs - cosim * shs;

     // ------------------------- do lunar terms ------------------
     m_tle.dedt = ses + s1 * znl * s5;
     m_tle.didt = sis + s2 * znl * (z11 + z13);
     m_tle.dmdt = sls - znl * s3 * (z1 + z3 - 14.0 - 6.0 * emsq);
     sghl = s4 * znl * (z31 + z33 - 6.0);
     shll = -znl * s2 * (z21 + z23);
     // sgp4fix for 180 deg incl
     if ((inclm < 5.2359877e-2) || (inclm > PI - 5.2359877e-2))
         shll = 0.0;
     m_tle.domdt = sgs + sghl;
     m_tle.dnodt = shs;
     if (sinim != 0.0)
       {
         m_tle.domdt = m_tle.domdt - cosim / sinim * shll;
         m_tle.dnodt = m_tle.dnodt + shll / sinim;
       }

     // ----------- calculate deep space resonance effects --------
     dndt   = 0.0;
     theta  = fmod(m_tle.gsto + tc * rptim, TWOPI);
     em     = em + m_tle.dedt * m_tle.t;
     inclm  = inclm + m_tle.didt * m_tle.t;
     argpm  = argpm + m_tle.domdt * m_tle.t;
     nodem  = nodem + m_tle.dnodt * m_tle.t;
     mm     = mm + m_tle.dmdt * m_tle.t;
     //   sgp4fix for negative inclinations
     //   the following if statement should be commented out
     //if (inclm < 0.0)
     //  {
     //    inclm  = -inclm;
     //    argpm  = argpm - pi;
     //    nodem = nodem + pi;
     //  }

     // -------------- initialize the resonance terms -------------
     if (m_tle.irez != 0)
       {
         aonv = pow(nm / m_tle.xke, x2o3);

         // ---------- geopotential resonance for 12 hour orbits ------
         if (m_tle.irez == 2)
           {
             cosisq = cosim * cosim;
             emo    = em;
             em     = m_tle.ecco;
             emsqo  = emsq;
             emsq   = eccsq;
             eoc    = em * emsq;
             g201   = -0.306 - (em - 0.64) * 0.440;

             if (em <= 0.65)
               {
                 g211 =    3.616  -  13.2470 * em +  16.2900 * emsq;
                 g310 =  -19.302  + 117.3900 * em - 228.4190 * emsq +  156.5910 * eoc;
                 g322 =  -18.9068 + 109.7927 * em - 214.6334 * emsq +  146.5816 * eoc;
                 g410 =  -41.122  + 242.6940 * em - 471.0940 * emsq +  313.9530 * eoc;
                 g422 = -146.407  + 841.8800 * em - 1629.014 * emsq + 1083.4350 * eoc;
                 g520 = -532.114  + 3017.977 * em - 5740.032 * emsq + 3708.2760 * eoc;
               }
               else
               {
                 g211 =   -72.099 +   331.819 * em -   508.738 * emsq +   266.724 * eoc;
                 g310 =  -346.844 +  1582.851 * em -  2415.925 * emsq +  1246.113 * eoc;
                 g322 =  -342.585 +  1554.908 * em -  2366.899 * emsq +  1215.972 * eoc;
                 g410 = -1052.797 +  4758.686 * em -  7193.992 * emsq +  3651.957 * eoc;
                 g422 = -3581.690 + 16178.110 * em - 24462.770 * emsq + 12422.520 * eoc;
                 if (em > 0.715)
                     g520 =-5149.66 + 29936.92 * em - 54087.36 * emsq + 31324.56 * eoc;
                   else
                     g520 = 1464.74 -  4664.75 * em +  3763.64 * emsq;
               }
             if (em < 0.7)
               {
                 g533 = -919.22770 + 4988.6100 * em - 9064.7700 * emsq + 5542.21  * eoc;
                 g521 = -822.71072 + 4568.6173 * em - 8491.4146 * emsq + 5337.524 * eoc;
                 g532 = -853.66600 + 4690.2500 * em - 8624.7700 * emsq + 5341.4  * eoc;
               }
               else
               {
                 g533 =-37995.780 + 161616.52 * em - 229838.20 * emsq + 109377.94 * eoc;
                 g521 =-51752.104 + 218913.95 * em - 309468.16 * emsq + 146349.42 * eoc;
                 g532 =-40023.880 + 170470.89 * em - 242699.48 * emsq + 115605.82 * eoc;
               }

             sini2=  sinim * sinim;
             f220 =  0.75 * (1.0 + 2.0 * cosim+cosisq);
             f221 =  1.5 * sini2;
             f321 =  1.875 * sinim  *  (1.0 - 2.0 * cosim - 3.0 * cosisq);
             f322 = -1.875 * sinim  *  (1.0 + 2.0 * cosim - 3.0 * cosisq);
             f441 = 35.0 * sini2 * f220;
             f442 = 39.3750 * sini2 * sini2;
             f522 =  9.84375 * sinim * (sini2 * (1.0 - 2.0 * cosim- 5.0 * cosisq) +
                     0.33333333 * (-2.0 + 4.0 * cosim + 6.0 * cosisq) );
             f523 = sinim * (4.92187512 * sini2 * (-2.0 - 4.0 * cosim +
                    10.0 * cosisq) + 6.56250012 * (1.0+2.0 * cosim - 3.0 * cosisq));
             f542 = 29.53125 * sinim * (2.0 - 8.0 * cosim+cosisq *
                    (-12.0 + 8.0 * cosim + 10.0 * cosisq));
             f543 = 29.53125 * sinim * (-2.0 - 8.0 * cosim+cosisq *
                    (12.0 + 8.0 * cosim - 10.0 * cosisq));
             xno2  =  nm * nm;
             ainv2 =  aonv * aonv;
             temp1 =  3.0 * xno2 * ainv2;
             temp  =  temp1 * root22;
             m_tle.d2201 =  temp * f220 * g201;
             m_tle.d2211 =  temp * f221 * g211;
             temp1 =  temp1 * aonv;
             temp  =  temp1 * root32;
             m_tle.d3210 =  temp * f321 * g310;
             m_tle.d3222 =  temp * f322 * g322;
             temp1 =  temp1 * aonv;
             temp  =  2.0 * temp1 * root44;
             m_tle.d4410 =  temp * f441 * g410;
             m_tle.d4422 =  temp * f442 * g422;
             temp1 =  temp1 * aonv;
             temp  =  temp1 * root52;
             m_tle.d5220 =  temp * f522 * g520;
             m_tle.d5232 =  temp * f523 * g532;
             temp  =  2.0 * temp1 * root54;
             m_tle.d5421 =  temp * f542 * g521;
             m_tle.d5433 =  temp * f543 * g533;
             m_tle.xlamo =  fmod(m_tle.mo + m_tle.nodeo + m_tle.nodeo-theta - theta, TWOPI);
             m_tle.xfact =  m_tle.mdot + m_tle.dmdt + 2.0 * (m_tle.nodedot + m_tle.dnodt - rptim) - m_tle.no;
             em    = emo;
             emsq  = emsqo;
           }

         // ---------------- synchronous resonance terms --------------
         if (m_tle.irez == 1)
           {
             g200  = 1.0 + emsq * (-2.5 + 0.8125 * emsq);
             g310  = 1.0 + 2.0 * emsq;
             g300  = 1.0 + emsq * (-6.0 + 6.60937 * emsq);
             f220  = 0.75 * (1.0 + cosim) * (1.0 + cosim);
             f311  = 0.9375 * sinim * sinim * (1.0 + 3.0 * cosim) - 0.75 * (1.0 + cosim);
             f330  = 1.0 + cosim;
             f330  = 1.875 * f330 * f330 * f330;
             m_tle.del1  = 3.0 * nm * nm * aonv * aonv;
             m_tle.del2  = 2.0 * m_tle.del1 * f220 * g200 * q22;
             m_tle.del3  = 3.0 * m_tle.del1 * f330 * g300 * q33 * aonv;
             m_tle.del1  = m_tle.del1 * f311 * g310 * q31 * aonv;
             m_tle.xlamo = fmod(m_tle.mo + m_tle.nodeo + m_tle.argpo - theta, TWOPI);
             m_tle.xfact = m_tle.mdot + xpidot - rptim + m_tle.dmdt + m_tle.domdt + m_tle.dnodt - m_tle.no;
           }

         // ------------ for sgp4, initialize the integrator ----------
         m_tle.xli   = m_tle.xlamo;
         m_tle.xni   = m_tle.no;
         m_tle.atime = 0.0;
         nm    = m_tle.no + dndt;
       }

//#include "debug3.cpp"
}  // end dsinit

/*-----------------------------------------------------------------------------
*
*                             procedure sgp4
*
*  this procedure is the sgp4 prediction model from space command. this is an
*    updated and combined version of sgp4 and sdp4, which were originally
*    published separately in spacetrack report #3. this version follows the
*    methodology from the aiaa paper (2006) describing the history and
*    development of the code.
*
*  author        : david vallado                  719-573-2600   28 jun 2005
*
*  inputs        :
*    satrec	 - initialised structure from sgp4init() call.
*    tsince	 - time eince epoch (minutes)
*
*  outputs       :
*    r           - position vector                     km
*    v           - velocity                            km/sec
*  return code - non-zero on error.
*                   1 - mean elements, ecc >= 1.0 or ecc < -0.001 or a < 0.95 er
*                   2 - mean motion less than 0.0
*                   3 - pert elements, ecc < 0.0  or  ecc > 1.0
*                   4 - semi-latus rectum < 0.0
*                   5 - epoch elements are sub-orbital
*                   6 - satellite has decayed
*
*  locals        :
*    am          -
*    axnl, aynl        -
*    betal       -
*    cosim   , sinim   , cosomm  , sinomm  , cnod    , snod    , cos2u   ,
*    sin2u   , coseo1  , sineo1  , cosi    , sini    , cosip   , sinip   ,
*    cosisq  , cossu   , sinsu   , cosu    , sinu
*    delm        -
*    delomg      -
*    dndt        -
*    eccm        -
*    emsq        -
*    ecose       -
*    el2         -
*    eo1         -
*    eccp        -
*    esine       -
*    argpm       -
*    argpp       -
*    omgadf      -
*    pl          -
*    r           -
*    rtemsq      -
*    rdotl       -
*    rl          -
*    rvdot       -
*    rvdotl      -
*    su          -
*    t2  , t3   , t4    , tc
*    tem5, temp , temp1 , temp2  , tempa  , tempe  , templ
*    u   , ux   , uy    , uz     , vx     , vy     , vz
*    inclm       - inclination
*    mm          - mean anomaly
*    nm          - mean motion
*    nodem       - right asc of ascending node
*    xinc        -
*    xincp       -
*    xl          -
*    xlm         -
*    mp          -
*    xmdf        -
*    xmx         -
*    xmy         -
*    nodedf      -
*    xnode       -
*    nodep       -
*    np          -
*
*  coupling      :
*    getgravconst-
*    dpper
*    dpspace
*
*  references    :
*    hoots, roehrich, norad spacetrack report #3 1980
*    hoots, norad spacetrack report #6 1986
*    hoots, schumacher and glover 2004
*    vallado, crawford, hujsak, kelso  2006
  ----------------------------------------------------------------------------*/

bool QSgp4::getPosition(double tsince, QEci &eci )
{
     double am   , axnl  , aynl , betal ,  cosim , cnod  ,
         cos2u, coseo1, cosi , cosip ,  cosisq, cossu , cosu,
         delm , delomg, em   , emsq  ,  ecose , el2   , eo1 ,
         ep   , esine , argpm, argpp ,  argpdf, pl,     mrt = 0.0,
         mvt  , rdotl , rl   , rvdot ,  rvdotl, sinim ,
         sin2u, sineo1, sini , sinip ,  sinsu , sinu  ,
         snod , su    , t2   , t3    ,  t4    , tem5  , temp,
         temp1, temp2 , tempa, tempe ,  templ , u     , ux  ,
         uy   , uz    , vx   , vy    ,  vz    , inclm , mm  ,
         nm   , nodem, xinc , xincp ,  xl    , xlm   , mp  ,
         xmdf , xmx   , xmy  , nodedf, xnode , nodep, tc  , dndt,
         x2o3  ,
         vkmpersec, delmtemp;
     double r[3], v[3];
     int ktr;

     /* ------------------ set mathematical constants --------------- */
     // sgp4fix divisor for divide by zero check on inclination
     // the old check used 1.0 + cos(pi-1.0e-9), but then compared it to
     // 1.5 e-12, so the threshold was changed to 1.5e-12 for consistency
     const double temp4 =   1.5e-12;
     x2o3  = 2.0 / 3.0;
     vkmpersec     = m_tle.radiusearthkm * m_tle.xke/60.0;

     /* --------------------- clear sgp4 error flag ----------------- */
     m_tle.t   = tsince;
     m_tle.error = 0;
     m_tle.errormsg = "";

     /* ------- update for secular gravity and atmospheric drag ----- */
     xmdf    = m_tle.mo + m_tle.mdot * m_tle.t;
     argpdf  = m_tle.argpo + m_tle.argpdot * m_tle.t;
     nodedf  = m_tle.nodeo + m_tle.nodedot * m_tle.t;
     argpm   = argpdf;
     mm      = xmdf;
     t2      = m_tle.t * m_tle.t;
     nodem   = nodedf + m_tle.nodecf * t2;
     tempa   = 1.0 - m_tle.cc1 * m_tle.t;
     tempe   = m_tle.bstar * m_tle.cc4 * m_tle.t;
     templ   = m_tle.t2cof * t2;

     if (m_tle.isimp != 1)
       {
         delomg = m_tle.omgcof * m_tle.t;
         // sgp4fix use mutliply for speed instead of pow
         delmtemp =  1.0 + m_tle.eta * cos(xmdf);
         delm   = m_tle.xmcof *
                  (delmtemp * delmtemp * delmtemp -
                  m_tle.delmo);
         temp   = delomg + delm;
         mm     = xmdf + temp;
         argpm  = argpdf - temp;
         t3     = t2 * m_tle.t;
         t4     = t3 * m_tle.t;
         tempa  = tempa - m_tle.d2 * t2 - m_tle.d3 * t3 -
                          m_tle.d4 * t4;
         tempe  = tempe + m_tle.bstar * m_tle.cc5 * (sin(mm) -
                          m_tle.sinmao);
         templ  = templ + m_tle.t3cof * t3 + t4 * (m_tle.t4cof +
                          m_tle.t * m_tle.t5cof);
       }

     nm    = m_tle.no;
     em    = m_tle.ecco;
     inclm = m_tle.inclo;
     if (m_tle.method == 'd')
       {
         tc = m_tle.t;
         dspace
             (
               m_tle.atime, em, argpm, inclm, m_tle.xli,
               mm, m_tle.xni, nodem, dndt, nm
             );
       } // if method = d

     if (nm <= 0.0)
       {
         m_tle.errormsg = QString("# error nm %1").arg(nm);
         m_tle.error = 2;
         // sgp4fix add return
         return false;
       }
     am = pow((m_tle.xke / nm),x2o3) * tempa * tempa;
     nm = m_tle.xke / pow(am, 1.5);
     em = em - tempe;

     // fix tolerance for error recognition
     // sgp4fix am is fixed from the previous nm check
     if ((em >= 1.0) || (em < -0.001)/* || (am < 0.95)*/ )
       {
         m_tle.errormsg = QString("# error em %1").arg(em);
         m_tle.error = 1;
         // sgp4fix to return if there is an error in eccentricity
         return false;
       }
     // sgp4fix fix tolerance to avoid a divide by zero
     if (em < 1.0e-6)
         em  = 1.0e-6;
     mm     = mm + m_tle.no * templ;
     xlm    = mm + argpm + nodem;
     emsq   = em * em;
     temp   = 1.0 - emsq;

     nodem  = fmod(nodem, TWOPI);
     argpm  = fmod(argpm, TWOPI);
     xlm    = fmod(xlm, TWOPI);
     mm     = fmod(xlm - argpm - nodem, TWOPI);

     /* ----------------- compute extra mean quantities ------------- */
     sinim = sin(inclm);
     cosim = cos(inclm);

     /* -------------------- add lunar-solar periodics -------------- */
     ep     = em;
     xincp  = inclm;
     argpp  = argpm;
     nodep  = nodem;
     mp     = mm;
     sinip  = sinim;
     cosip  = cosim;
     if (m_tle.method == 'd')
       {

         this->ep = ep;
         this->inclp = xincp;
         this->nodep = nodep;
         this->argpp = argpp;
         this->mp = mp;

         dpper( 'n');

         ep = this->ep;
         xincp = this->inclp;
         nodep = this->nodep;
         argpp = this->argpp;
         mp = this->mp;



         if (xincp < 0.0)
           {
             xincp  = -xincp;
             nodep = nodep + PI;
             argpp  = argpp - PI;
           }
         if ((ep < 0.0 ) || ( ep > 1.0))
           {
             m_tle.errormsg = QString("# error ep %1").arg(ep);
             m_tle.error = 3;
             // sgp4fix add return
             return false;
           }
       } // if method = d

     /* -------------------- long period periodics ------------------ */
     if (m_tle.method == 'd')
       {
         sinip =  sin(xincp);
         cosip =  cos(xincp);
         m_tle.aycof = -0.5*m_tle.j3oj2*sinip;
         // sgp4fix for divide by zero for xincp = 180 deg
         if (fabs(cosip+1.0) > 1.5e-12)
             m_tle.xlcof = -0.25 * m_tle.j3oj2 * sinip * (3.0 + 5.0 * cosip) / (1.0 + cosip);
           else
             m_tle.xlcof = -0.25 * m_tle.j3oj2 * sinip * (3.0 + 5.0 * cosip) / temp4;
       }
     axnl = ep * cos(argpp);
     temp = 1.0 / (am * (1.0 - ep * ep));
     aynl = ep* sin(argpp) + temp * m_tle.aycof;
     xl   = mp + argpp + nodep + temp * m_tle.xlcof * axnl;

     /* --------------------- solve kepler's equation --------------- */
     u    = fmod(xl - nodep, TWOPI);
     eo1  = u;
     tem5 = 9999.9;
     ktr = 1;
     //   sgp4fix for kepler iteration
     //   the following iteration needs better limits on corrections
     while (( fabs(tem5) >= 1.0e-12) && (ktr <= 10) )
       {
         sineo1 = sin(eo1);
         coseo1 = cos(eo1);
         tem5   = 1.0 - coseo1 * axnl - sineo1 * aynl;
         tem5   = (u - aynl * coseo1 + axnl * sineo1 - eo1) / tem5;
         if(fabs(tem5) >= 0.95)
             tem5 = tem5 > 0.0 ? 0.95 : -0.95;
         eo1    = eo1 + tem5;
         ktr = ktr + 1;
       }

     /* ------------- short period preliminary quantities ----------- */
     ecose = axnl*coseo1 + aynl*sineo1;
     esine = axnl*sineo1 - aynl*coseo1;
     el2   = axnl*axnl + aynl*aynl;
     pl    = am*(1.0-el2);
     if (pl < 0.0)
       {
         m_tle.errormsg = QString("# error pl %1").arg(pl);
         m_tle.error = 4;
         // sgp4fix add return
         return false;
       }
       else
       {
         rl     = am * (1.0 - ecose);
         rdotl  = sqrt(am) * esine/rl;
         rvdotl = sqrt(pl) / rl;
         betal  = sqrt(1.0 - el2);
         temp   = esine / (1.0 + betal);
         sinu   = am / rl * (sineo1 - aynl - axnl * temp);
         cosu   = am / rl * (coseo1 - axnl + aynl * temp);
         su     = atan2(sinu, cosu);
         sin2u  = (cosu + cosu) * sinu;
         cos2u  = 1.0 - 2.0 * sinu * sinu;
         temp   = 1.0 / pl;
         temp1  = 0.5 * m_tle.j2 * temp;
         temp2  = temp1 * temp;

         /* -------------- update for short period periodics ------------ */
         if (m_tle.method == 'd')
           {
             cosisq                 = cosip * cosip;
             m_tle.con41  = 3.0*cosisq - 1.0;
             m_tle.x1mth2 = 1.0 - cosisq;
             m_tle.x7thm1 = 7.0*cosisq - 1.0;
           }
         mrt   = rl * (1.0 - 1.5 * temp2 * betal * m_tle.con41) +
                 0.5 * temp1 * m_tle.x1mth2 * cos2u;
         su    = su - 0.25 * temp2 * m_tle.x7thm1 * sin2u;
         xnode = nodep + 1.5 * temp2 * cosip * sin2u;
         xinc  = xincp + 1.5 * temp2 * cosip * sinip * cos2u;
         mvt   = rdotl - nm * temp1 * m_tle.x1mth2 * sin2u / m_tle.xke;
         rvdot = rvdotl + nm * temp1 * (m_tle.x1mth2 * cos2u +
                 1.5 * m_tle.con41) / m_tle.xke;

         /* --------------------- orientation vectors ------------------- */
         sinsu =  sin(su);
         cossu =  cos(su);
         snod  =  sin(xnode);
         cnod  =  cos(xnode);
         sini  =  sin(xinc);
         cosi  =  cos(xinc);
         xmx   = -snod * cosi;
         xmy   =  cnod * cosi;
         ux    =  xmx * sinsu + cnod * cossu;
         uy    =  xmy * sinsu + snod * cossu;
         uz    =  sini * sinsu;
         vx    =  xmx * cossu - cnod * sinsu;
         vy    =  xmy * cossu - snod * sinsu;
         vz    =  sini * cossu;

         /* --------- position and velocity (in km and km/sec) ---------- */
         r[0] = (mrt * ux)* m_tle.radiusearthkm;
         r[1] = (mrt * uy)* m_tle.radiusearthkm;
         r[2] = (mrt * uz)* m_tle.radiusearthkm;
         v[0] = (mvt * ux + rvdot * vx) * vkmpersec;
         v[1] = (mvt * uy + rvdot * vy) * vkmpersec;
         v[2] = (mvt * uz + rvdot * vz) * vkmpersec;
       }  // if pl > 0

     // sgp4fix for decaying satellites
     if (mrt < 1.0)
       {
         m_tle.errormsg = QString("# decay condition %1").arg( mrt, 11, 'f', 6);
         m_tle.error = 6;
         return false;
       }

     Vector3 vecPos(r[0], r[1], r[2]);
     Vector3 vecVel(v[0], v[1], v[2]);

     QSgp4Date gmt;
     gmt.Set(m_tle.jdsatepoch + tsince/1440.0, false);

     eci = QEci(vecPos, vecVel, gmt);


//#include "debug7.cpp"
     return true;
}  // end sgp4

/*-----------------------------------------------------------------------------
*
*                           procedure dspace
*
*  this procedure provides deep space contributions to mean elements for
*    perturbing third body.  these effects have been averaged over one
*    revolution of the sun and moon.  for earth resonance effects, the
*    effects have been averaged over no revolutions of the satellite.
*    (mean motion)
*
*  author        : david vallado                  719-573-2600   28 jun 2005
*
*  inputs        :
*    d2201, d2211, d3210, d3222, d4410, d4422, d5220, d5232, d5421, d5433 -
*    dedt        -
*    del1, del2, del3  -
*    didt        -
*    dmdt        -
*    dnodt       -
*    domdt       -
*    irez        - flag for resonance           0-none, 1-one day, 2-half day
*    argpo       - argument of perigee
*    argpdot     - argument of perigee dot (rate)
*    t           - time
*    tc          -
*    gsto        - gst
*    xfact       -
*    xlamo       -
*    no          - mean motion
*    atime       -
*    em          - eccentricity
*    ft          -
*    argpm       - argument of perigee
*    inclm       - inclination
*    xli         -
*    mm          - mean anomaly
*    xni         - mean motion
*    nodem       - right ascension of ascending node
*
*  outputs       :
*    atime       -
*    em          - eccentricity
*    argpm       - argument of perigee
*    inclm       - inclination
*    xli         -
*    mm          - mean anomaly
*    xni         -
*    nodem       - right ascension of ascending node
*    dndt        -
*    nm          - mean motion
*
*  locals        :
*    delt        -
*    ft          -
*    theta       -
*    x2li        -
*    x2omi       -
*    xl          -
*    xldot       -
*    xnddt       -
*    xndt        -
*    xomi        -
*
*  coupling      :
*    none        -
*
*  references    :
*    hoots, roehrich, norad spacetrack report #3 1980
*    hoots, norad spacetrack report #6 1986
*    hoots, schumacher and glover 2004
*    vallado, crawford, hujsak, kelso  2006
  ----------------------------------------------------------------------------*/

void QSgp4::dspace
     (double& atime, double& em,    double& argpm,  double& inclm, double& xli,
       double& mm,    double& xni,   double& nodem,  double& dndt,  double& nm
     )
{
     int iretn , iret;
     double delt, ft, theta, x2li, x2omi, xl, xldot , xnddt, xndt, xomi, g22, g32,
          g44, g52, g54, fasx2, fasx4, fasx6, rptim , step2, stepn , stepp;

     fasx2 = 0.13130908;
     fasx4 = 2.8843198;
     fasx6 = 0.37448087;
     g22   = 5.7686396;
     g32   = 0.95240898;
     g44   = 1.8014998;
     g52   = 1.0508330;
     g54   = 4.4108898;
     rptim = 4.37526908801129966e-3; // this equates to 7.29211514668855e-5 rad/sec
     stepp =    720.0;
     stepn =   -720.0;
     step2 = 259200.0;

     /* ----------- calculate deep space resonance effects ----------- */
     dndt   = 0.0;
     theta  = fmod(m_tle.gsto + m_tle.t * rptim, TWOPI);
     em     = em + m_tle.dedt * m_tle.t;

     inclm  = inclm + m_tle.didt * m_tle.t;
     argpm  = argpm + m_tle.domdt * m_tle.t;
     nodem  = nodem + m_tle.dnodt * m_tle.t;
     mm     = mm + m_tle.dmdt * m_tle.t;

     //   sgp4fix for negative inclinations
     //   the following if statement should be commented out
     //  if (inclm < 0.0)
     // {
     //    inclm = -inclm;
     //    argpm = argpm - pi;
     //    nodem = nodem + pi;
     //  }

     /* - update resonances : numerical (euler-maclaurin) integration - */
     /* ------------------------- epoch restart ----------------------  */
     //   sgp4fix for propagator problems
     //   the following integration works for negative time steps and periods
     //   the specific changes are unknown because the original code was so convoluted

     // sgp4fix take out atime = 0.0 and fix for faster operation
     ft    = 0.0;
     if (m_tle.irez != 0)
       {
         // sgp4fix streamline check
         if ((atime == 0.0) || (m_tle.t * atime <= 0.0) || (fabs(m_tle.t) < fabs(atime)) )
           {
             atime  = 0.0;
             xni    = m_tle.no;
             xli    = m_tle.xlamo;
           }
           // sgp4fix move check outside loop
           if (m_tle.t > 0.0)
               delt = stepp;
             else
               delt = stepn;

         iretn = 381; // added for do loop
         iret  =   0; // added for loop
         while (iretn == 381)
           {
             /* ------------------- dot terms calculated ------------- */
             /* ----------- near - synchronous resonance terms ------- */
             if (m_tle.irez != 2)
               {
                 xndt  = m_tle.del1 * sin(xli - fasx2) + m_tle.del2 * sin(2.0 * (xli - fasx4)) +
                         m_tle.del3 * sin(3.0 * (xli - fasx6));
                 xldot = xni + m_tle.xfact;
                 xnddt = m_tle.del1 * cos(xli - fasx2) +
                         2.0 * m_tle.del2 * cos(2.0 * (xli - fasx4)) +
                         3.0 * m_tle.del3 * cos(3.0 * (xli - fasx6));
                 xnddt = xnddt * xldot;
               }
               else
               {
                 /* --------- near - half-day resonance terms -------- */
                 xomi  = m_tle.argpo + m_tle.argpdot * atime;
                 x2omi = xomi + xomi;
                 x2li  = xli + xli;
                 xndt  = m_tle.d2201 * sin(x2omi + xli - g22) + m_tle.d2211 * sin(xli - g22) +
                       m_tle.d3210 * sin(xomi + xli - g32)  + m_tle.d3222 * sin(-xomi + xli - g32)+
                       m_tle.d4410 * sin(x2omi + x2li - g44)+ m_tle.d4422 * sin(x2li - g44) +
                       m_tle.d5220 * sin(xomi + xli - g52)  + m_tle.d5232 * sin(-xomi + xli - g52)+
                       m_tle.d5421 * sin(xomi + x2li - g54) + m_tle.d5433 * sin(-xomi + x2li - g54);
                 xldot = xni + m_tle.xfact;
                 xnddt = m_tle.d2201 * cos(x2omi + xli - g22) + m_tle.d2211 * cos(xli - g22) +
                       m_tle.d3210 * cos(xomi + xli - g32) + m_tle.d3222 * cos(-xomi + xli - g32) +
                       m_tle.d5220 * cos(xomi + xli - g52) + m_tle.d5232 * cos(-xomi + xli - g52) +
                       2.0 * (m_tle.d4410 * cos(x2omi + x2li - g44) +
                       m_tle.d4422 * cos(x2li - g44) + m_tle.d5421 * cos(xomi + x2li - g54) +
                       m_tle.d5433 * cos(-xomi + x2li - g54));
                 xnddt = xnddt * xldot;
               }

             /* ----------------------- integrator ------------------- */
             // sgp4fix move end checks to end of routine
             if (fabs(m_tle.t - atime) >= stepp)
               {
                 iret  = 0;
                 iretn = 381;
               }
               else // exit here
               {
                 ft    = m_tle.t - atime;
                 iretn = 0;
               }

             if (iretn == 381)
               {
                 xli   = xli + xldot * delt + xndt * step2;
                 xni   = xni + xndt * delt + xnddt * step2;
                 atime = atime + delt;
               }
           }  // while iretn = 381

         nm = xni + xndt * ft + xnddt * ft * ft * 0.5;
         xl = xli + xldot * ft + xndt * ft * ft * 0.5;
         if (m_tle.irez != 1)
           {
             mm   = xl - 2.0 * nodem + 2.0 * theta;
             dndt = nm - m_tle.no;
           }
           else
           {
             mm   = xl - nodem - argpm + theta;
             dndt = nm - m_tle.no;
           }
         nm = m_tle.no + dndt;
       }

//#include "debug4.cpp"
}  // end dsspace

