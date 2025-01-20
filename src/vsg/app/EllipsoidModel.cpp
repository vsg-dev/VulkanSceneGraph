/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/EllipsoidModel.h>
#include <vsg/maths/transform.h>

using namespace vsg;

EllipsoidModel::EllipsoidModel(double rEquator, double rPolar) :
    _radiusEquator(rEquator),
    _radiusPolar(rPolar)
{
    _computeEccentricitySquared();
}

void EllipsoidModel::_computeEccentricitySquared()
{
    double flattening = (_radiusEquator - _radiusPolar) / _radiusEquator;
    _eccentricitySquared = 2 * flattening - flattening * flattening;
}

void EllipsoidModel::read(Input& input)
{
    Object::read(input);

    input.read("radiusEquator", _radiusEquator);
    input.read("radiusPolar", _radiusPolar);

    _computeEccentricitySquared();
}

void EllipsoidModel::write(Output& output) const
{
    Object::write(output);

    output.write("radiusEquator", _radiusEquator);
    output.write("radiusPolar", _radiusPolar);
}

dvec3 EllipsoidModel::convertLatLongAltitudeToECEF(const dvec3& lla) const
{
    const double latitude = radians(lla[0]);
    const double longitude = radians(lla[1]);
    const double height = lla[2];

    // for details on maths see https://en.wikipedia.org/wiki/ECEF
    double sin_latitude = sin(latitude);
    double cos_latitude = cos(latitude);
    double N = _radiusEquator / sqrt(1.0 - _eccentricitySquared * sin_latitude * sin_latitude);
    return dvec3((N + height) * cos_latitude * cos(longitude),
                 (N + height) * cos_latitude * sin(longitude),
                 (N * (1 - _eccentricitySquared) + height) * sin_latitude);
}

dvec3 EllipsoidModel::convertECEFToLatLongAltitude(const dvec3& ecef) const
{
    double latitude, longitude, height;
    const double PI_2 = PI * 0.5;

    // handle polar and center-of-earth cases directly.
    if (ecef.x != 0.0)
        longitude = atan2(ecef.y, ecef.x);
    else
    {
        if (ecef.y > 0.0)
            longitude = PI_2;
        else if (ecef.y < 0.0)
            longitude = -PI_2;
        else
        {
            // at pole or at center of the earth
            longitude = 0.0;
            if (ecef.z > 0.0)
            { // north pole.
                latitude = PI_2;
                height = ecef.z - _radiusPolar;
            }
            else if (ecef.z < 0.0)
            { // south pole.
                latitude = -PI_2;
                height = -ecef.z - _radiusPolar;
            }
            else
            { // center of earth.
                latitude = PI_2;
                height = -_radiusPolar;
            }
            return dvec3(latitude, longitude, height);
        }
    }

    // http://www.colorado.edu/geography/gcraft/notes/datum/gif/xyzllh.gif
    double p = sqrt(ecef.x * ecef.x + ecef.y * ecef.y);
    double theta = atan2(ecef.z * _radiusEquator, (p * _radiusPolar));
    double eDashSquared = (_radiusEquator * _radiusEquator - _radiusPolar * _radiusPolar) /
                          (_radiusPolar * _radiusPolar);

    double sin_theta = sin(theta);
    double cos_theta = cos(theta);

    latitude = atan((ecef.z + eDashSquared * _radiusPolar * sin_theta * sin_theta * sin_theta) /
                    (p - _eccentricitySquared * _radiusEquator * cos_theta * cos_theta * cos_theta));

    double sin_latitude = sin(latitude);
    double N = _radiusEquator / sqrt(1.0 - _eccentricitySquared * sin_latitude * sin_latitude);

    height = p / cos(latitude) - N;
    return dvec3(degrees(latitude), degrees(longitude), height);
}

dmat4 EllipsoidModel::computeLocalToWorldTransform(const dvec3& lla) const
{
    dvec3 ecef = convertLatLongAltitudeToECEF(lla);

    const double latitude = radians(lla[0]);
    const double longitude = radians(lla[1]);

    // Compute up, east and north vector
    dvec3 up(cos(longitude) * cos(latitude), sin(longitude) * cos(latitude), sin(latitude));
    dvec3 east(-sin(longitude), cos(longitude), 0.0);
    dvec3 north = cross(up, east);

    dmat4 localToWorld = vsg::translate(ecef);

    // set matrix
    localToWorld(0, 0) = east[0];
    localToWorld(0, 1) = east[1];
    localToWorld(0, 2) = east[2];

    localToWorld(1, 0) = north[0];
    localToWorld(1, 1) = north[1];
    localToWorld(1, 2) = north[2];

    localToWorld(2, 0) = up[0];
    localToWorld(2, 1) = up[1];
    localToWorld(2, 2) = up[2];

    return localToWorld;
}

dmat4 EllipsoidModel::computeWorldToLocalTransform(const dvec3& lla) const
{
    return vsg::inverse(computeLocalToWorldTransform(lla));
}
