#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/viewer/ViewMatrix.h>

namespace vsg
{

    // Base class for projection matrices
    class ProjectionMatrix : public Inherit<Object, ProjectionMatrix>
    {
    public:
        virtual void get(mat4& matrix) const = 0;
        virtual void get(dmat4& matrix) const = 0;

        virtual void get_inverse(mat4& matrix) const
        {
            get(matrix);
            matrix = inverse(matrix);
        }
        virtual void get_inverse(dmat4& matrix) const
        {
            get(matrix);
            matrix = inverse(matrix);
        }
    };

    class Perspective : public Inherit<ProjectionMatrix, Perspective>
    {
    public:
        Perspective() :
            fieldOfViewY(60.0),
            aspectRatio(1.0),
            nearDistance(1.0),
            farDistance(10000.0)
        {
        }

        Perspective(double fov, double ar, double nd, double fd) :
            fieldOfViewY(fov),
            aspectRatio(ar),
            nearDistance(nd),
            farDistance(fd)
        {
        }

        void get(mat4& matrix) const override { matrix = perspective(radians(fieldOfViewY), aspectRatio, nearDistance, farDistance); }
        void get(dmat4& matrix) const override { matrix = perspective(radians(fieldOfViewY), aspectRatio, nearDistance, farDistance); }

        double fieldOfViewY;
        double aspectRatio;
        double nearDistance;
        double farDistance;
    };

    class Orthographic : public Inherit<ProjectionMatrix, Orthographic>
    {
    public:
        Orthographic() :
            left(-1.0),
            right(1.0),
            bottom(-1.0),
            top(1.0),
            nearDistance(1.0),
            farDistance(10000.0)
        {
        }

        Orthographic(double l, double r, double b, double t, double nd, double fd) :
            left(l),
            right(r),
            bottom(b),
            top(t),
            nearDistance(nd),
            farDistance(fd)
        {
        }

        void get(mat4& matrix) const override { matrix = orthographic(left, right, bottom, top, nearDistance, farDistance); }
        void get(dmat4& matrix) const override { matrix = orthographic(left, right, bottom, top, nearDistance, farDistance); }

        double left;
        double right;
        double bottom;
        double top;
        double nearDistance;
        double farDistance;
    };

    const double WGS_84_RADIUS_EQUATOR = 6378137.0;
    const double WGS_84_RADIUS_POLAR = 6356752.3142;

    class EllipsoidModel : public Inherit<Object, EllipsoidModel>
    {
    public:
        EllipsoidModel(double rEquator = WGS_84_RADIUS_EQUATOR, double rPolar = WGS_84_RADIUS_POLAR) :
            _radiusEquator(rEquator),
            _radiusPolar(rPolar)
        {
            double flattening = (_radiusEquator - _radiusPolar) / _radiusEquator;
            _eccentricitySquared = 2 * flattening - flattening * flattening;
        }

        double radiusEquator() const { return _radiusEquator; }
        double radiusPolar() const { return _radiusPolar; }

        // latitude and longitude in radians
        dvec3 convertLatLongAltitudeToECEF(const dvec3& lla)
        {
            const double latitude = lla[0];
            const double longitude = lla[1];
            const double height = lla[2];

            // for details on maths see http://www.colorado.edu/geography/gcraft/notes/datum/gif/llhxyz.gif
            double sin_latitude = sin(latitude);
            double cos_latitude = cos(latitude);
            double N = _radiusEquator / sqrt(1.0 - _eccentricitySquared * sin_latitude * sin_latitude);
            return dvec3((N + height) * cos_latitude * cos(longitude),
                         (N + height) * cos_latitude * sin(longitude),
                         (N * (1 - _eccentricitySquared) + height) * sin_latitude);
        }

        // latitude and longitude in radians
        dvec3 convertECEVToLatLongAltitude(const dvec3& ecef)
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
            return dvec3(latitude, longitude, height);
        }

    protected:
        double _radiusEquator;
        double _radiusPolar;
        double _eccentricitySquared;
    };

    class EllipsoidPerspective : public Inherit<ProjectionMatrix, EllipsoidPerspective>
    {
    public:
        EllipsoidPerspective(ref_ptr<LookAt> la, ref_ptr<EllipsoidModel> em) :
            lookAt(la),
            ellipsoidModel(em),
            fieldOfViewY(60.0),
            aspectRatio(1.0),
            nearFarRatio(0.0001),
            horizonMountainHeight(1000.0)
        {
        }

        EllipsoidPerspective(ref_ptr<LookAt> la, ref_ptr<EllipsoidModel> em, double fov, double ar, double nfr, double hmh) :
            lookAt(la),
            ellipsoidModel(em),
            fieldOfViewY(fov),
            aspectRatio(ar),
            nearFarRatio(nfr),
            horizonMountainHeight(hmh)
        {
        }

        void get(mat4& matrix) const override
        {
            dmat4 dm;
            get(dm);
            matrix = dm;
        }
        void get(dmat4& matrix) const override
        {
            // std::cout<<"camera eye : "<<lookAt->eye<<", "<<ellipsoidModel->convertECEVToLatLongAltitude(lookAt->eye)<<std::endl;
            vsg::dvec3 v = lookAt->eye;
            vsg::dvec3 lv = vsg::normalize(lookAt->center - lookAt->eye);
            double R = ellipsoidModel->radiusEquator();
            double H = ellipsoidModel->convertECEVToLatLongAltitude(v).z;
            double D = R + H;
            double alpha = (D > R) ? std::acos(R / D) : 0.0;
            double beta = std::acos(R / (R + horizonMountainHeight));
            double theta = std::acos(-vsg::dot(lv, v) / (vsg::length(lv) * vsg::length(v)));

            double l = R * (std::tan(alpha) + std::tan(beta));

            double farDistance = std::cos(theta + alpha - vsg::PI * 0.5) * l;
            double nearDistance = farDistance * nearFarRatio;
            //std::cout<<"H = "<<H<<", l = "<<l<<", theta = "<<vsg::degrees(theta)<<", fd = "<<farDistance<<std::endl;

            matrix = perspective(radians(fieldOfViewY), aspectRatio, nearDistance, farDistance);
        }

        ref_ptr<LookAt> lookAt;
        ref_ptr<EllipsoidModel> ellipsoidModel;

        double fieldOfViewY;
        double aspectRatio;
        double nearFarRatio;
        double horizonMountainHeight;
    };
} // namespace vsg
