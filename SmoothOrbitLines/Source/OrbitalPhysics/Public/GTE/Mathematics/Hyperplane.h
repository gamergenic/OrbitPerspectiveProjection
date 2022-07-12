// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.07.29

#pragma once

#include <Mathematics/Matrix.h>
#include <Mathematics/SingularValueDecomposition.h>
#include <Mathematics/Vector3.h>

// The hyperplane is represented as Dot(U, X - P) = 0 where U is a unit-length
// normal vector, P is the hyperplane origin, and X is any point on the
// hyperplane. The user must ensure that the normal vector is unit length. The
// hyperplane constant is c = Dot(U, P) so that Dot(U, X) = c. If P is not
// specified when constructing a hyperplane, it is chosen to be the point on
// the plane closest to the origin, P = c * U.

namespace gte
{
    template <int N, typename T>
    class Hyperplane
    {
    public:
        // Construction and destruction.  The default constructor sets the
        // normal to (0,...,0,1), the origin to (0,...,0) and the constant to
        // zero.
        Hyperplane()
            :
            normal{},
            origin(Vector<N, T>::Zero()),
            constant(static_cast<T>(0))
        {
            normal.MakeUnit(N - 1);
        }

        Hyperplane(Vector<N, T> const& inNormal, T const& inConstant)
            :
            normal(inNormal),
            origin(inConstant * inNormal),
            constant(inConstant)
        {
        }

        Hyperplane(Vector<N, T> const& inNormal, Vector<N, T> const& inOrigin)
            :
            normal(inNormal),
            origin(inOrigin),
            constant(Dot(inNormal, inOrigin))
        {
        }

        // U is a unit-length vector in the orthogonal complement of the set
        // {p[1]-p[0],...,p[n-1]-p[0]} and c = Dot(U,p[0]), where the p[i] are
        // pointson the hyperplane.
        Hyperplane(std::array<Vector<N, T>, N> const& p)
        {
            ComputeFromPoints<N>(p);
        }

        // Public member access.
        Vector<N, T> normal;
        Vector<N, T> origin;
        T constant;

    public:
        // Comparisons to support sorted containers.
        bool operator==(Hyperplane const& hyperplane) const
        {
            return normal == hyperplane.normal
                && origin == hyperplane.origin
                && constant == hyperplane.constant;
        }

        bool operator!=(Hyperplane const& hyperplane) const
        {
            return !operator==(hyperplane);
        }

        bool operator< (Hyperplane const& hyperplane) const
        {
            if (normal < hyperplane.normal)
            {
                return true;
            }

            if (origin > hyperplane.origin)
            {
                return false;
            }

            if (origin < hyperplane.origin)
            {
                return true;
            }

            if (origin > hyperplane.origin)
            {
                return false;
            }

            return constant < hyperplane.constant;
        }

        bool operator<=(Hyperplane const& hyperplane) const
        {
            return !hyperplane.operator<(*this);
        }

        bool operator> (Hyperplane const& hyperplane) const
        {
            return hyperplane.operator<(*this);
        }

        bool operator>=(Hyperplane const& hyperplane) const
        {
            return !operator<(hyperplane);
        }

    private:
        // For use in the Hyperplane(std::array<*>) constructor when N > 3.
        template <int Dimension = N>
        typename std::enable_if<Dimension != 3, void>::type
        ComputeFromPoints(std::array<Vector<Dimension, T>, Dimension> const& p)
        {
            Matrix<Dimension, Dimension - 1, T> edge{};
            for (int i0 = 0, i1 = 1; i1 < Dimension; i0 = i1++)
            {
                edge.SetCol(i0, p[i1] - p[0]);
            }

            // Compute the 1-dimensional orthogonal complement of the edges of
            // the simplex formed by the points p[].
            SingularValueDecomposition<T> svd(Dimension, Dimension - 1, 32);
            svd.Solve(&edge[0], -1);
            svd.GetUColumn(Dimension - 1, &normal[0]);

            constant = Dot(normal, p[0]);
            origin = constant * normal;
        }

        // For use in the Hyperplane(std::array<*>) constructor when N == 3.
        template <int Dimension = N>
        typename std::enable_if<Dimension == 3, void>::type
        ComputeFromPoints(std::array<Vector<Dimension, T>, Dimension> const& p)
        {
            Vector<Dimension, T> edge0 = p[1] - p[0];
            Vector<Dimension, T> edge1 = p[2] - p[0];
            normal = UnitCross(edge0, edge1);
            constant = Dot(normal, p[0]);
            origin = constant * normal;
        }
    };

    // Template alias for convenience.
    template <typename T>
    using Plane3 = Hyperplane<3, T>;
}
