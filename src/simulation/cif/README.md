# CIF Reader

A library to open and interpret .cif files. The main aim is to be able to open a .cif file, and create a large supercell (i.e. a .xyz file).

**Warning**: The .cif file standard is barely a standard. I cannot test for every scenario and there is a good chance of being unable to read certain files. If you find a file you cannot open, please send it to me so I can try to add support for it.

## Usage

To open a .cif file, simply construct `CIFReader` with the path of the file you want to open.

```c++
auto cif = CIFReader(file_path);
```

Unit cell information can then be retrieved by first using the `getUnitCell` method

```c++
UnitCell cell = cif.getUnitCell();
```

From here, the geometry can be extracted

```c++
CellGeometry geom = cell.getGeometry();
// get the a, b and c vectors
std::vector<double> a = geom.getAVector();
std::vector<double> a = geom.getBVector();
std::vector<double> a = geom.getCVector();
```

as well as the list of atoms

```c++
std::vector<AtomSite> atoms = cell.getAtoms();

// Take one atom and get it's positions (after symmetry), occupancy and element
AtomSite a = atoms[0];

std::vector<double> occ = a.getOccupancies();
std::vector<std::vector<double>> pos = a.getPositions()
std::vector<std::string> element = a.getElements()

```