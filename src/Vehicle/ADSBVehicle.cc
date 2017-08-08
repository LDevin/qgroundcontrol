/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "ADSBVehicle.h"

#include <QDebug>
#include <QtMath>

ADSBVehicle::ADSBVehicle(mavlink_adsb_vehicle_t& adsbVehicle, QObject* parent)
    : QObject       (parent)
    , _icaoAddress  (adsbVehicle.ICAO_address)
    , _callsign     (adsbVehicle.callsign)
    , _altitude     (NAN)
    , _heading      (NAN)
{
    if (!(adsbVehicle.flags | ADSB_FLAGS_VALID_COORDS)) {
        qWarning() << "At least coords must be valid";
        return;
    }

    update(adsbVehicle);
}

ADSBVehicle::ADSBVehicle(const QGeoCoordinate& location, float heading, QObject* parent)
    : QObject(parent), _icaoAddress(0)
{
    update(location, heading);
}

void ADSBVehicle::update(const QGeoCoordinate& location, float heading)
{
    _coordinate = location;
    _altitude = location.altitude();
    _heading = heading;
    emit coordinateChanged(_coordinate);
    emit altitudeChanged(_altitude);
    emit headingChanged(_heading);
}

void ADSBVehicle::update(mavlink_adsb_vehicle_t& adsbVehicle)
{
    if (_icaoAddress != adsbVehicle.ICAO_address) {
        qWarning() << "ICAO address mismatch expected:actual" << _icaoAddress << adsbVehicle.ICAO_address;
        return;
    }

    if (!(adsbVehicle.flags | ADSB_FLAGS_VALID_COORDS)) {
        return;
    }

    QString currCallsign(adsbVehicle.callsign);

    if (currCallsign != _callsign) {
        _callsign = currCallsign;
        emit callsignChanged(_callsign);
    }

    QGeoCoordinate newCoordinate(adsbVehicle.lat / 1e7, adsbVehicle.lon / 1e7);
    if (newCoordinate != _coordinate) {
        _coordinate = newCoordinate;
        emit coordinateChanged(_coordinate);
    }

    double newAltitude = NAN;
    if (adsbVehicle.flags | ADSB_FLAGS_VALID_ALTITUDE) {
        newAltitude = (double)adsbVehicle.altitude / 1e3;
    }
    if (!(qIsNaN(newAltitude) && qIsNaN(_altitude)) && !qFuzzyCompare(newAltitude, _altitude)) {
        _altitude = newAltitude;
        emit altitudeChanged(_altitude);
    }

    double newHeading = NAN;
    if (adsbVehicle.flags | ADSB_FLAGS_VALID_HEADING) {
        newHeading = (double)adsbVehicle.heading / 100.0;
    }
    if (!(qIsNaN(newHeading) && qIsNaN(_heading)) && !qFuzzyCompare(newHeading, _heading)) {
        _heading = newHeading;
        emit headingChanged(_heading);
    }
}
