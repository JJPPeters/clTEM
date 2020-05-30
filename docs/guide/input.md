---
title: Input
---

# {{page.title}}

Currently, the only supported file format are .xyz files. However, some modification are supported so that occupancy and thermal parameters can be supported.

## Basic .xyz files

At it's simplest, the .xyz file has 3 main sections:

 - the number of atoms in the structure
 -  comment section
 - the atoms and their coordinates

The first line contains the number of atoms and the second line is reserved for comments. Every line after that represents an atom, in the form: `A x y z`, where `A` is the element symbol and `x`, `y`, `z` are the cartesian coordinates in Å. If you want to use the coordinates in nm, then include the term `nm` anywhere in the comments line (with whitespace either side). The use of nm then applied to all relevant units in the file (such as thermal vibrations).

## Extra information

To add any more information, clTEM will look at the comments section as a sort of header section for the columns. This has defaults and multiple options to try and easily support as much as possible. They keywords that are looked for are `A`, `x`, `y`, `z`, `occ`, `u`, `ux`, `uy` and `uz`. Any other terms will be ignored, so comments can be added (just make sure they don't contain any of these terms separated by spaces).

`A`, `x`, `y`, `z` do not need to be stated, if none of them are found, it is assumed that they form columns 1-4 in the order shown. Other terms are then treated as columns 5 onwards, in the order they are written in the comments section.

### Occupancy

Many structures require occupancies for the same sites. If the column `occ` is defined, clTEM will detect atoms that share the same site **if they are placed on consecutive lines** in the .xyz.

### Thermal vibrations

Thermal vibrations are defined by the headers `u`, `ux`, `uy` and `uz` that define the isotropic vibration and then the vibration specific to each cartesian axis. `u` is set first, then any of `ux`, `uy` or `uz` will override the relevant component of `u`. `u` is defined here as the mean square displacement of the atom, <u²>, and will have the units of Å² (or nm² if the `nm` tag has been set).

### Example file

Below is a small example file with occupancy and thermal parameters

```
6
occ u
Pb 0.0 0.0 0.0 1.0 0.0056
Pb 4.0 0.0 0.0 1.0 0.0056
Pb 4.0 4.0 0.0 1.0 0.0056
Pb 0.0 4.0 0.0 1.0 0.0056
Ti 2.0 2.0 0.0 0.8 0.0103
Zr 2.0 2.0 0.0 0.2 0.0048
```