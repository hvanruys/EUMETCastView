/*
Module : AADATE.H
Purpose: Implementation for the algorithms which convert between the Gregorian and Julian calendars and the Julian Day
Created: PJN / 29-12-2003

Copyright (c) 2003 - 2013 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


/////////////////////// Macros / Defines //////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __SGP4DATE_H__
#define __SGP4DATE_H__

#include "qsgp4globals.h"
#include <QString>

/////////////////////// Classes ///////////////////////////////////////////////

class SGP4CalendarDate
{
public:
//Constructors / Destructors
  SGP4CalendarDate() : Year(0),
                      Month(0), 
                      Day(0) 
  {
  }

//Member variables
  long Year;
  long Month;
  long Day;
};


class QSgp4Date
{
public:
//Enums
  enum DAY_OF_WEEK
  {	
    SUNDAY	  = 0,
    MONDAY	  = 1,
    TUESDAY	  = 2,
    WEDNESDAY	= 3,
    THURSDAY	= 4,
    FRIDAY	  = 5,
    SATURDAY	= 6
  };

//Constructors / Destructors
  QSgp4Date();
  QSgp4Date(long Year, long Month, double Day, bool bGregorianCalendar);
  QSgp4Date(long Year, long Month, double Day, double Hour, double Minute, double Second, bool bGregorianCalendar = true);
  QSgp4Date(double JD, bool bGregorianCalendar);

//Static Methods
  static double          DateToJD(long Year, long Month, double Day, bool bGregorianCalendar);
  static bool            IsLeap(long Year, bool bGregorianCalendar);
  static void            DayOfYearToDayAndMonth(long DayOfYear, bool bLeap, long& DayOfMonth, long& Month);
  static SGP4CalendarDate JulianToGregorian(long Year, long Month, long Day);
  static SGP4CalendarDate GregorianToJulian(long Year, long Month, long Day);
  static long            INT(double value);
  static bool            AfterPapalReform(long Year, long Month, double Day);
  static bool            AfterPapalReform(double JD);
  static double          DayOfYear(double JD, long Year, bool bGregorianCalendar);
  static long            DaysInMonth(long Month, bool bLeap);


//Non Static methods
  double      Julian() const { return m_dblJulian; }
  operator    double() const { return m_dblJulian; }
  long        Day() const;
  long        Month() const;
  long        Year() const;
  long        Hour() const;
  long        Minute() const;
  double      Second() const;
  void        Set(long Year, long Month, double Day, double Hour, double Minute, double Second, bool bGregorianCalendar);
  void        Set(double JD, bool bGregorianCalendar);
  void        SetInGregorianCalendar(bool bGregorianCalendar);
  void        Get(long& Year, long& Month, long& Day, long& Hour, long& Minute, double& Second) const;
  DAY_OF_WEEK DayOfWeek() const;
  double      DayOfYear() const;
  long        DaysInMonth() const;
  long        DaysInYear() const;
  bool        Leap() const;
  bool        InGregorianCalendar() const { return m_bGregorianCalendar; }
  double      FractionalYear() const;
  double      FromJan1_00h_1900() const { return m_dblJulian - EPOCH_JAN1_00H_1900; }
  double      FromJan1_12h_1900() const { return m_dblJulian - EPOCH_JAN1_12H_1900; }
  double      FromJan1_12h_2000() const { return m_dblJulian - EPOCH_JAN1_12H_2000; }
  double      ToGreenwichSiderealTime() const;
  double      ToLocalMeanSiderealTime(double lon) const;
  QString     toString();
  static QSgp4Date   NowUTC();

  double spanDay (const QSgp4Date& b) const { return m_dblJulian - b.Julian();        }
  double spanHour(const QSgp4Date& b) const { return spanDay(b) * HOURS_PER_DAY;  }
  double spanMin (const QSgp4Date& b) const { return spanDay(b) * MINUTES_PER_DAY; }
  double spanSec (const QSgp4Date& b) const { return spanDay(b) * SECONDS_PER_DAY; }

  void AddDay (double day) { m_dblJulian += day;                 }
  void AddHour(double hr ) { m_dblJulian += (hr  / HOURS_PER_DAY ); }
  void AddMin (double min) { m_dblJulian += (min / MINUTES_PER_DAY); }
  void AddSec (double sec) { m_dblJulian += (sec / SECONDS_PER_DAY); }


protected:
//Member variables
  double m_dblJulian;  //Julian Day number for this date
  bool   m_bGregorianCalendar; //Is this date in the Gregorian calendar
};

#endif //__AADATE_H__
