/*
 * Copyright 2013 Daniel Warner <contact@danrw.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef QOBSERVER_H_
#define QOBSERVER_H_

#include "qgeodetic.h"
#include "qtopocentric.h"
#include "qsgp4date.h"
#include "qeci.h"

class QSgp4Date;
class QTopocentric;

class QObserver
{
public:

    QObserver(const double latitude,
            const double longitude,
            const double altitude)
        : m_geo(Util::DegreesToRadians(latitude), Util::DegreesToRadians(longitude), altitude),
        m_eci(m_geo, QSgp4Date())
    {
    }

    QObserver(const QGeodetic &geo)
        : m_geo(geo),
        m_eci(geo, QSgp4Date())
    {
    }

    virtual ~QObserver()
    {
    }

    void SetLocation(const QGeodetic& geo)
    {
        m_geo = geo;
        m_eci.Update(m_geo, m_eci.GetDate());
    }

    QGeodetic GetLocation() const
    {
        return m_geo;
    }

    QTopocentric GetLookAngle(QEci &eci);

private:
    void Update(const QSgp4Date &dt)
    {
        if (m_eci != dt)
        {
            m_eci.Update(m_geo, dt);
        }
    }

    QGeodetic m_geo;
    QEci m_eci;
};

#endif

