#ifndef ATOM_H
#define ATOM_H


struct Atom
{
    unsigned int A;
    float x, y, z;

    Atom() : A(0), x(0.0f), y(0.0f), z(0.0f) {}

    Atom(unsigned int _A, float _x, float _y, float _z) : A(_A), x(_x), y(_y), z(_z) {}

    // this is only for position testing (Not element testing)
    bool operator==(const Atom &RHS) {
        return x == RHS.x && y == RHS.y && z == RHS.z;
    }

    Atom operator* (float f) {
        return {A, f*x, f*y, f*z};
    }
};

struct AtomSite : public Atom
{
    float occ;
    float ux, uy, uz;
    AtomSite() : Atom(), occ(1.0f), ux(0.0f), uy(0.0f), uz(0.0f) {}
    AtomSite(unsigned int _A, float _x, float _y, float _z, float _occ, float _ux, float _uy, float _uz) : Atom(_A, _x, _y, _z), occ(_occ), ux(_ux), uy(_uy), uz(_uz) {}

    AtomSite operator* (float f) {
        // u is in Angstrom squared
        return AtomSite(A, f*x, f*y, f*z, occ, f*f*ux, f*f*uy, f*f*uz);
    }

    void setThermal(float u) {
        ux = u;
        uy = u;
        uz = u;
    }
};

// use this to sort atoms by z https://stackoverflow.com/questions/1380463/sorting-a-vector-of-custom-objects
struct AtomSite_z_less_Sort {
    inline bool operator() (const Atom& a1, const Atom& a2) {
        return a1.z < a2.z;
    }
};

#endif // ATOM_H
