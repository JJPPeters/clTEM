#ifndef ATOM_H
#define ATOM_H


struct Atom
{
    unsigned int A;
    double x, y, z;

    Atom() : A(0), x(0.0), y(0.0), z(0.0) {}

    Atom(unsigned int _A, double _x, double _y, double _z) : A(_A), x(_x), y(_y), z(_z) {}

    // this is only for position testing (Not element testing)
    bool operator==(const Atom &RHS) {
        return x == RHS.x && y == RHS.y && z == RHS.z;
    }

    Atom operator* (double f) {
        return {A, f*x, f*y, f*z};
    }
};

// Stores the atom data
// TODO: set if thermal vibrations are defined properly
struct AtomSite : public Atom
{
    double occ;
    bool defined_u;
    double u1, u2, u3;
    AtomSite() : Atom(), occ(1.0), defined_u(false), u1(0.0), u2(0.0), u3(0.0) {}
    AtomSite(unsigned int _a, double _x, double _y, double _z) : Atom(_a, _x, _y, _z), occ(1.0), defined_u(false), u1(0.0), u2(0.0), u3(0.0) {}
    AtomSite(unsigned int _a, double _x, double _y, double _z, double _occ) : Atom(_a, _x, _y, _z), occ(_occ), defined_u(false), u1(0.0), u2(0.0), u3(0.0) {}
    AtomSite(unsigned int _a, double _x, double _y, double _z, double _occ, double _ux, double _uy, double _uz) : Atom(_a, _x, _y, _z), occ(_occ), defined_u(true), u1(_ux), u2(_uy), u3(_uz) {}
    AtomSite(unsigned int _a, double _x, double _y, double _z, double _occ, bool _defined_u, double _ux, double _uy, double _uz) : Atom(_a, _x, _y, _z), occ(_occ), defined_u(_defined_u), u1(_ux), u2(_uy), u3(_uz) {}

    AtomSite operator* (double f) {
        // u is in Angstrom squared
        return AtomSite(A, f*x, f*y, f*z, occ, defined_u, f * f * u1, f * f * u2, f * f * u3);
    }
//
//    void setThermal(double u) {
//        u1 = u;
//        u2 = u;
//        u3 = u;
//    }
};

// use this to sort atoms by z https://stackoverflow.com/questions/1380463/sorting-a-vector-of-custom-objects
struct AtomSite_z_less_Sort {
    inline bool operator() (const Atom& a1, const Atom& a2) {
        return a1.z < a2.z;
    }
};

#endif // ATOM_H
