
// vmath.cpp [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


#include "pengine.h"



template <class T>
frustum<T> &frustum<T>::construct(const mat44<T> &mat)
{
    side[SideRight] = plane<T>(
        mat.row[0][3] - mat.row[0][0],
        mat.row[1][3] - mat.row[1][0],
        mat.row[2][3] - mat.row[2][0],
        mat.row[3][3] - mat.row[3][0]);
    side[SideRight].normalize();

    side[SideLeft] = plane<T>(
        mat.row[0][3] + mat.row[0][0],
        mat.row[1][3] + mat.row[1][0],
        mat.row[2][3] + mat.row[2][0],
        mat.row[3][3] + mat.row[3][0]);
    side[SideLeft].normalize();

    side[SideTop] = plane<T>(
        mat.row[0][3] - mat.row[0][1],
        mat.row[1][3] - mat.row[1][1],
        mat.row[2][3] - mat.row[2][1],
        mat.row[3][3] - mat.row[3][1]);
    side[SideTop].normalize();

    side[SideBottom] = plane<T>(
        mat.row[0][3] + mat.row[0][1],
        mat.row[1][3] + mat.row[1][1],
        mat.row[2][3] + mat.row[2][1],
        mat.row[3][3] + mat.row[3][1]);
    side[SideBottom].normalize();

    side[SideNear] = plane<T>(
        mat.row[0][3] - mat.row[0][2],
        mat.row[1][3] - mat.row[1][2],
        mat.row[2][3] - mat.row[2][2],
        mat.row[3][3] - mat.row[3][2]);
    side[SideNear].normalize();

    side[SideFar] = plane<T>(
        mat.row[0][3] + mat.row[0][2],
        mat.row[1][3] + mat.row[1][2],
        mat.row[2][3] + mat.row[2][2],
        mat.row[3][3] + mat.row[3][2]);
    side[SideFar].normalize();

    return *this;
}
template frustum<float> &frustum<float>::construct(const mat44<float> &mat);
template frustum<double> &frustum<double>::construct(const mat44<double> &mat);

template <class T>
bool frustum<T>::isAABBOutside(const vec3<T> &mins, const vec3<T> &maxs) const
{
    vec3<T> pts[8] = {
        vec3<T>(mins.x, mins.y, mins.z),
        vec3<T>(maxs.x, mins.y, mins.z),
        vec3<T>(mins.x, maxs.y, mins.z),
        vec3<T>(maxs.x, maxs.y, mins.z),
        vec3<T>(mins.x, mins.y, maxs.z),
        vec3<T>(maxs.x, mins.y, maxs.z),
        vec3<T>(mins.x, maxs.y, maxs.z),
        vec3<T>(maxs.x, maxs.y, maxs.z)
    };
    for (int s = 0; s < 1; ++s) {
        bool outside = true;
        for (int p = 0; p < 8; ++p) {
            //if (side[s].normal * pts[p] + side[s].offset >= 0.0) {
            if (side[s].normal * pts[p] >= 0.0) {
                outside = false;
                break;
            }
        }
        if (outside) return true;
    }

    return false;
}
template bool frustum<float>::isAABBOutside(const vec3<float> &mins, const vec3<float> &maxs) const;
template bool frustum<double>::isAABBOutside(const vec3<double> &mins, const vec3<double> &maxs) const;




