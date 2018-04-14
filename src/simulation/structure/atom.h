#ifndef ATOM_H
#define ATOM_H


struct Atom
{
    int A;
    float x, y, z;

    Atom() : x(0.0f), y(0.0f), z(0.0f) {}

    Atom(int _A, float _x, float _y, float _z) : A(_A), x(_x), y(_y), z(_z) {}

    // this is only for position testing
    bool operator==(const Atom &RHS)
    {
        return x == RHS.x && y == RHS.y && z == RHS.z;
    }

    Atom operator* (float f)
    {
        return Atom(A, f*x, f*y, f*z);
    }
};

struct AtomOcc : public Atom
{
    float occ;
    AtomOcc() : Atom(), occ(0.0f) {}
    AtomOcc(int _A, float _x, float _y, float _z, float _occ) : Atom(_A, _x, _y, _z), occ(_occ) {}

    Atom operator* (float f)
    {
        return AtomOcc(A, f*x, f*y, f*z, occ);
    }
};

#endif // ATOM_H
