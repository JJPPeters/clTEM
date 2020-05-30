//
// Created by jonat on 22/05/2020.
//

#include "simulationcell.h"

SimulationCell::SimulationCell()
        : default_xy_padding({-8.0, 8.0}), default_z_padding({-3.0, 3.0}),
        slice_thickness(1.0), slice_offset(0.0) {
    padding_x = default_xy_padding;
    padding_y = default_xy_padding;
    padding_z = default_z_padding;
}

SimulationCell::SimulationCell(const SimulationCell &sm) {
    default_xy_padding = sm.default_xy_padding;
    default_z_padding = sm.default_z_padding;
    slice_thickness = sm.slice_thickness;
    slice_offset = sm.slice_offset;
    padding_x = sm.padding_x;
    padding_y = sm.padding_y;
    padding_z = sm.padding_z;

    if (sm.crystal_structure)
        crystal_structure = std::make_shared<CrystalStructure>(*(sm.crystal_structure));
}

SimulationCell &SimulationCell::operator=(const SimulationCell &sm) {
    default_xy_padding = sm.default_xy_padding;
    default_z_padding = sm.default_z_padding;
    slice_thickness = sm.slice_thickness;
    slice_offset = sm.slice_offset;
    padding_x = sm.padding_x;
    padding_y = sm.padding_y;
    padding_z = sm.padding_z;

    if (sm.crystal_structure)
        crystal_structure = std::make_shared<CrystalStructure>(*(sm.crystal_structure));

    return *this;
}

void SimulationCell::roundPaddingZ() {
    // Do the Z padding so that our slice thickness/shift works (and produces AT LEAST the desired padding)
    auto p_z = default_z_padding;

    auto pad_slices_pre = (int) std::ceil((std::abs(p_z[1]) - slice_offset) / slice_thickness);
    double pre_pad = slice_offset + pad_slices_pre * slice_thickness;

    double zw = crystal_structure->limitsZ()[1] - crystal_structure->limitsZ()[0];
    auto struct_slices = (int) std::ceil((zw + slice_offset) / slice_thickness); // slice offset needed here?
    double struct_slice_thick = struct_slices * slice_thickness;

    double left_over = struct_slice_thick - slice_offset - zw;

    auto pad_slices_post = (int) std::ceil((std::abs(p_z[0]) - left_over) / slice_thickness);
    double post_pad = pad_slices_post * slice_thickness + left_over;

    // The simulation works from LARGEST z to SMALLEST. So the pre padding is actually on top of the z structure.
    padding_z = {-post_pad, pre_pad};
}

void SimulationCell::setDefaultPaddingXY(const std::valarray<double> &pd_xy) {
    if(pd_xy.size() != 2)
        throw std::runtime_error("XY padding value must be valarray with 2 values");
    default_xy_padding = pd_xy;
}

void SimulationCell::setDefaultPaddingZ(const std::valarray<double> &pd_z) {
    if(pd_z.size() != 2)
        throw std::runtime_error("Z padding value must be valarray with 2 values");
    default_z_padding = pd_z;
}

unsigned int SimulationCell::sliceCount() {
    auto z_lims = paddedStructLimitsZ();
    double z_range = z_lims[1] - z_lims[0];

    // the 0.000001 is for errors in the float
    auto n_slices = (unsigned int) std::ceil((z_range / slice_thickness) - 0.000001);
    n_slices += (n_slices == 0);
    return n_slices;
}

unsigned int SimulationCell::preSliceCount() {
    return static_cast<unsigned int>(padding_z[1] / slice_thickness);
}

void SimulationCell::setCrystalStructure(std::string &file_path, CIF::SuperCellInfo info, bool fix_cif) {
    crystal_structure.reset(new CrystalStructure(file_path, info, fix_cif));
}

void SimulationCell::setCrystalStructure(CIF::CIFReader cif, CIF::SuperCellInfo info) {
    crystal_structure.reset(new CrystalStructure(cif, info));
}
