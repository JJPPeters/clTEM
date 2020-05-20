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
    double ux, uy, uz;
    AtomSite() : Atom(), occ(1.0), defined_u(false), ux(0.0), uy(0.0), uz(0.0) {}
    AtomSite(unsigned int _a, double _x, double _y, double _z) : Atom(_a, _x, _y, _z), occ(1.0), defined_u(false), ux(0.0), uy(0.0), uz(0.0) {}
    AtomSite(unsigned int _a, double _x, double _y, double _z, double _occ) : Atom(_a, _x, _y, _z), occ(_occ), defined_u(false), ux(0.0), uy(0.0), uz(0.0) {}
    AtomSite(unsigned int _a, double _x, double _y, double _z, double _occ, double _ux, double _uy, double _uz) : Atom(_a, _x, _y, _z), occ(_occ), defined_u(true), ux(_ux), uy(_uy), uz(_uz) {}
    AtomSite(unsigned int _a, double _x, double _y, double _z, double _occ, bool _defined_u, double _ux, double _uy, double _uz) : Atom(_a, _x, _y, _z), occ(_occ), defined_u(_defined_u), ux(_ux), uy(_uy), uz(_uz) {}

    AtomSite operator* (double f) {
        // u is in Angstrom squared
        return AtomSite(A, f*x, f*y, f*z, occ, defined_u, f*f*ux, f*f*uy, f*f*uz);
    }
//
//    void setThermal(double u) {
//        ux = u;
//        uy = u;
//        uz = u;
//    }
};

// use this to sort atoms by z https://stackoverflow.com/questions/1380463/sorting-a-vector-of-custom-objects
struct AtomSite_z_less_Sort {
    inline bool operator() (const Atom& a1, const Atom& a2) {
        return a1.z < a2.z;
    }
};

#endif // ATOM_H
