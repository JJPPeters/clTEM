//
// Created by jonat on 22/05/2020.
//

#ifndef CLTEM_SIMULATIONCELL_H
#define CLTEM_SIMULATIONCELL_H

#include "structure/crystalstructure.h"

class SimulationCell {

private:
    // padding added to the outside of the crystal structure
    // this is largely to avoid FFT wrap around artefacts
    // these are the values actually used by the simulations
    std::valarray<double> padding_x, padding_y, padding_z;

    // these paddings are the user defined values
    std::valarray<double> default_xy_padding, default_z_padding;

    // these calculate the padding from the defaults and structure
    // Nothing happens with the xy padding now, but z needs to be rounded to get the correct slice offset
    void roundPaddingX() {padding_x = default_xy_padding;}
    void roundPaddingY() {padding_y = default_xy_padding;}
    void roundPaddingZ();

    // thickness of each slice
    double slice_thickness;
    // offset of each slice, so we can try and centre the atoms in the slice
    double slice_offset;

    std::shared_ptr<CrystalStructure> crystal_structure;

public:

    explicit SimulationCell();

    SimulationCell(const SimulationCell& sm);

    SimulationCell& operator=(const SimulationCell& sm);

    // getters
    [[nodiscard]] double sliceThickness() const {return slice_thickness;}
    [[nodiscard]] double sliceOffset() const {return slice_offset;}

    unsigned int sliceCount();
    unsigned int preSliceCount();

    void setCrystalStructure(std::string &file_path, CIF::SuperCellInfo info = CIF::SuperCellInfo(), bool fix_cif=false);
    void setCrystalStructure(CIF::CIFReader cif, CIF::SuperCellInfo info);
    std::shared_ptr<CrystalStructure> crystalStructure() {return crystal_structure;}

    std::valarray<double> defaultPaddingXY() {return default_xy_padding;}
    std::valarray<double> defaultPaddingZ() {return default_z_padding;}

    std::valarray<double> paddingX() {roundPaddingX(); return padding_x;}
    std::valarray<double> paddingY() {roundPaddingY(); return padding_y;}
    std::valarray<double> paddingZ() {roundPaddingZ(); return padding_z;}

    std::valarray<double> paddedStructLimitsX() { return crystal_structure->limitsX() + paddingX(); }
    std::valarray<double> paddedStructLimitsY() { return crystal_structure->limitsY() + paddingY(); }
    std::valarray<double> paddedStructLimitsZ() { return crystal_structure->limitsZ() + paddingZ(); }

    // setters
    void setSliceThickness(double thickness) {slice_thickness = thickness;}
    void setSliceOffset(double offset) {slice_offset = offset;}

    void setDefaultPaddingXY(const std::valarray<double> &pd_xy);
    void setDefaultPaddingZ(const std::valarray<double> &pd_z);

};


#endif //CLTEM_SIMULATIONCELL_H
