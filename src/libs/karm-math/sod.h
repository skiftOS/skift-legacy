#pragma once

#include <karm-base/clamp.h>
#include <karm-base/opt.h>

#include "const.h"

namespace Karm::Math {

template <typename T>
struct Sod {
    // Based on code from https://youtu.be/KPoeNZZ6H4s?t=856

    bool _init = false;
    T _xp = {};
    T _y = {};
    T _yd = {};

    double _w;
    double _z;
    double _d;
    double _k1;
    double _k2;
    double _k3;

    Sod(double f, double z, double r) {
        _w = 2 * PI * f;
        _z = z;
        _d = _w * sqrt(abs(z * z - 1));
        _k1 = z / (PI * f);
        _k2 = 1 / (_w * _w);
        _k3 = r * z / _w;
    }

    void reset() {
        _init = false;
    }

    T update(T x, double t) {
        if (!_init) {
            _xp = x;
            _y = x;
            _yd = 0;
        }

        // Estimate velocity
        auto xd = (x - _xp) / t;
        _xp = x;

        // Clamp k1 and k2 to guarantee stability without jitter
        double k1Stable;
        double k2Stable;

        if (_w * t < _z) {
            k1Stable = _k1;
            k2Stable = max(_k2, t * t / 2 + t * _k1 / 2, t * _k1);
        } else {
            // Use pole matching when the system is very fast
            double t1 = exp(-_z * _w * t);
            double alpha = 2 * t1 * (_z <= 1 ? cos(t * _d) : cosh(t * _d));
            double beta = t1 * t1;
            double t2 = t / (1 + beta - alpha);
            k1Stable = (1 - beta) * t2;
            k2Stable = t * t2;
        }

        // Integrate position by velocity
        _y = _y + t * _yd;

        // Interfeare velocity by acceleration
        _yd = _yd + t * (x + _k3 * xd - _y - k1Stable * _yd) / k2Stable;

        return _y;
    }
};

} // namespace Karm::Math
