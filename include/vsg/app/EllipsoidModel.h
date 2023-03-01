#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>

namespace vsg
{

    const double WGS_84_RADIUS_EQUATOR = 6378137.0;
    const double WGS_84_RADIUS_POLAR = 6356752.3142;

    /// EllipsoidModel provides a ellipsoid definition of celastral body and helper methods for computing positions/transforms on that ellipsoid.
    /// Defaults to WGS_84 ellipsoid model of Earth.
    class VSG_DECLSPEC EllipsoidModel : public Inherit<Object, EllipsoidModel>
    {
    public:
        EllipsoidModel(double rEquator = WGS_84_RADIUS_EQUATOR, double rPolar = WGS_84_RADIUS_POLAR);

        bool operator==(const EllipsoidModel& rhs) const
        {
            return _radiusEquator == rhs._radiusEquator && _radiusPolar == rhs._radiusPolar;
        }

        bool operator!=(const EllipsoidModel& rhs) const
        {
            return _radiusEquator != rhs._radiusEquator || _radiusPolar != rhs._radiusPolar;
        }

        double radiusEquator() const { return _radiusEquator; }
        double radiusPolar() const { return _radiusPolar; }

        void read(Input& input) override;
        void write(Output& output) const override;

        /// latitude and longitude in degrees, altitude in metres, ECEF coords in metres.
        dvec3 convertLatLongAltitudeToECEF(const dvec3& lla) const;

        /// latitude and longitude in degrees, altitude in metres, ECEF coords in metres.
        dvec3 convertECEFToLatLongAltitude(const dvec3& ecef) const;

        /// latitude and longitude in degrees, altitude in metres
        dmat4 computeLocalToWorldTransform(const dvec3& lla) const;

        /// latitude and longitude in degrees, altitude in metres
        dmat4 computeWorldToLocalTransform(const dvec3& lla) const;

    protected:
        void _computeEccentricitySquared();

        double _radiusEquator;
        double _radiusPolar;
        double _eccentricitySquared;
    };
    VSG_type_name(vsg::EllipsoidModel);

} // namespace vsg
