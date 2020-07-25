//
// Created by jon on 04/04/18.
//

#include <structure/structureparameters.h>
#include "jsonutils.h"

namespace JSONUtils {

    SimulationManager JsonToManager(json& j) {
        bool dummy;
        return JsonToManager(j, dummy);
    }

    SimulationManager JsonToManager(json& j, bool& area_set)
    {
        SimulationManager man;
        area_set = false;
        // not sure there is a particularly easy way to go about this. Just go through all the options...
        try { man.setDoublePrecisionEnabled( readJsonEntry<bool>(j, "double precision") );
        } catch (std::exception& e) {}

        try {
            auto mode = readJsonEntry<SimulationMode>(j, "mode", "id");
            if (mode == SimulationMode::None)
                mode = SimulationMode::CTEM;
            man.setMode( mode );
        } catch (std::exception& e) {}

        try { man.setResolution( readJsonEntry<unsigned int>(j, "resolution") );
        } catch (std::exception& e) {}

        try { man.simulationCell()->setSliceThickness( readJsonEntry<double>(j, "slice thickness", "val") );
        } catch (std::exception& e) {}

        try { man.simulationCell()->setSliceOffset( readJsonEntry<double>(j, "slice offset", "val") );
        } catch (std::exception& e) {}

        try { man.setIntermediateSlices( readJsonEntry<unsigned int>(j, "intermediate output", "slice interval") );
        } catch (std::exception& e) {}

        try { man.setIntermediateSlicesEnabled( readJsonEntry<bool>(j, "intermediate output", "enabled") );
        } catch (std::exception& e) {}

        try { man.setFull3dEnabled( readJsonEntry<bool>(j, "full 3d", "state") );
        } catch (std::exception& e) {}

        try { man.setFull3dIntegrals( readJsonEntry<unsigned int>(j, "full 3d", "integrals") );
        } catch (std::exception& e) {}

        try { man.setPrecalculateTransmission( readJsonEntry<bool>(j, "precalculate transmission") );
        } catch (std::exception& e) {}

        try { man.setMaintainAreas( readJsonEntry<bool>(j, "maintain areas") );
        } catch (std::exception& e) {}

        try { man.setStructureParameters( readJsonEntry<std::string>(j, "potentials") );
        } catch (std::exception& e) {}

        try {
            auto p_xy_val = readJsonEntry<double>(j, "default padding", "xy", "val");
            man.simulationCell()->setDefaultPaddingXY({-p_xy_val, p_xy_val});
        } catch (std::exception& e) {}

        try {
            auto p_z_val = readJsonEntry<double>(j, "default padding", "z", "val");
            man.simulationCell()->setDefaultPaddingZ({-p_z_val, p_z_val});
        } catch (std::exception& e) {}

        //
        // Do all the microscope parameters stuff...
        //

        auto mp = man.microscopeParams();

        try { mp->Voltage = readJsonEntry<double>(j, "microscope", "voltage", "val");
        } catch (std::exception& e) {}

        // apertures
        try { mp->CondenserAperture = readJsonEntry<double>(j, "microscope", "condenser aperture", "semi-angle");
        } catch (std::exception& e) {}

        try { mp->CondenserApertureSmoothing = readJsonEntry<double>(j, "microscope", "condenser aperture", "smoothing");
        } catch (std::exception& e) {}

        try { mp->ObjectiveAperture = readJsonEntry<double>(j, "microscope", "objective aperture", "semi-angle");
        } catch (std::exception& e) {}

        try { mp->ObjectiveApertureSmoothing = readJsonEntry<double>(j, "microscope", "objective aperture", "smoothing");
        } catch (std::exception& e) {}

        //
        try { mp->BeamTilt = readJsonEntry<double>(j, "microscope", "beam tilt", "inclination", "val");
        } catch (std::exception& e) {}

        try { mp->BeamAzimuth = (Constants::Pi / 180) * readJsonEntry<double>(j, "microscope", "beam tilt", "azimuth", "val");
        } catch (std::exception& e) {}

        try { mp->Alpha = readJsonEntry<double>(j, "microscope", "alpha", "val");
        } catch (std::exception& e) {}

        try { mp->Delta = 10 * readJsonEntry<double>(j, "microscope", "delta", "val");
        } catch (std::exception& e) {}

        try { mp->C10 = 10 * readJsonEntry<double>(j, "microscope", "aberrations", "C10", "val");
        } catch (std::exception& e) {}

        try {
            mp->C12.Mag = 10 * readJsonEntry<double>(j, "microscope", "aberrations", "C12", "mag");
            mp->C12.Ang = (Constants::Pi / 180) * readJsonEntry<double>(j, "microscope", "aberrations", "C12", "ang");
        } catch (std::exception& e) {}

        try {
            mp->C21.Mag = 10 * readJsonEntry<double>(j, "microscope", "aberrations", "C21", "mag");
            mp->C21.Ang = (Constants::Pi / 180) * readJsonEntry<double>(j, "microscope", "aberrations", "C21", "ang");
        } catch (std::exception& e) {}

        try {
            mp->C23.Mag = 10 * readJsonEntry<double>(j, "microscope", "aberrations", "C23", "mag");
            mp->C23.Ang = (Constants::Pi / 180) * readJsonEntry<double>(j, "microscope", "aberrations", "C23", "ang");
        } catch (std::exception& e) {}

        try { mp->C30 = 10000 * readJsonEntry<double>(j, "microscope", "aberrations", "C30", "val");
        } catch (std::exception& e) {}

        try {
            mp->C32.Mag = 10000 * readJsonEntry<double>(j, "microscope", "aberrations", "C32", "mag");
            mp->C32.Ang = (Constants::Pi / 180) * readJsonEntry<double>(j, "microscope", "aberrations", "C32", "ang");
        } catch (std::exception& e) {}

        try {
            mp->C34.Mag = 10000 * readJsonEntry<double>(j, "microscope", "aberrations", "C34", "mag");
            mp->C34.Ang = (Constants::Pi / 180) * readJsonEntry<double>(j, "microscope", "aberrations", "C34", "ang");
        } catch (std::exception& e) {}

        try {
            mp->C41.Mag = 10000 * readJsonEntry<double>(j, "microscope", "aberrations", "C41", "mag");
            mp->C41.Ang = (Constants::Pi / 180) * readJsonEntry<double>(j, "microscope", "aberrations", "C41", "ang");
        } catch (std::exception& e) {}

        try {
            mp->C43.Mag = 10000 * readJsonEntry<double>(j, "microscope", "aberrations", "C43", "mag");
            mp->C43.Ang = (Constants::Pi / 180) * readJsonEntry<double>(j, "microscope", "aberrations", "C43", "ang");
        } catch (std::exception& e) {}

        try {
            mp->C45.Mag = 10000 * readJsonEntry<double>(j, "microscope", "aberrations", "C45", "mag");
            mp->C45.Ang = (Constants::Pi / 180) * readJsonEntry<double>(j, "microscope", "aberrations", "C45", "ang");
        } catch (std::exception& e) {}

        try { mp->C50 = 10000 * readJsonEntry<double>(j, "microscope", "aberrations", "C50", "val");
        } catch (std::exception& e) {}

        try {
            mp->C52.Mag = 10000 * readJsonEntry<double>(j, "microscope", "aberrations", "C52", "mag");
            mp->C52.Ang = (Constants::Pi / 180) * readJsonEntry<double>(j, "microscope", "aberrations", "C52", "ang");
        } catch (std::exception& e) {}

        try {
            mp->C54.Mag = 10000 * readJsonEntry<double>(j, "microscope", "aberrations", "C54", "mag");
            mp->C54.Ang = (Constants::Pi / 180) * readJsonEntry<double>(j, "microscope", "aberrations", "C54", "ang");
        } catch (std::exception& e) {}

        try {
            mp->C56.Mag = 10000 * readJsonEntry<double>(j, "microscope", "aberrations", "C56", "mag");
            mp->C56.Ang = (Constants::Pi / 180) * readJsonEntry<double>(j, "microscope", "aberrations", "C56", "ang");
        } catch (std::exception& e) {}

        //
        // Phew, Ctem now
        //

        try { man.setCtemImageEnabled(readJsonEntry<bool>(j, "ctem", "simulate image"));
        } catch (std::exception& e) {}

        try { man.setCcdName(readJsonEntry<std::string>(j, "ctem", "ccd", "name"));
        } catch (std::exception& e) {}

        try { man.setCcdDose(readJsonEntry<double>(j, "ctem", "ccd", "dose", "val"));
        } catch (std::exception& e) {}

        try { man.setCcdBinning(readJsonEntry<int>(j, "ctem", "ccd", "binning"));
        } catch (std::exception& e) {}

        try {
            auto xs = readJsonEntry<double>(j, "ctem", "area", "x", "start");
            auto xf = readJsonEntry<double>(j, "ctem", "area", "x", "finish");

            auto ys = readJsonEntry<double>(j, "ctem", "area", "y", "start");
            auto yf = readJsonEntry<double>(j, "ctem", "area", "y", "finish");

            SimulationArea ar(xs, xf, ys, yf);
            auto area = man.simulationArea();
            *area = ar;
            if (man.mode() == SimulationMode::CTEM)
                area_set = true;
        } catch (std::exception& e) {}

        //
        // STEM
        //

        try {
            auto xs = readJsonEntry<double>(j, "stem", "area", "x", "start");
            auto xf = readJsonEntry<double>(j, "stem", "area", "x", "finish");
            auto xp = readJsonEntry<int>(j, "stem", "scan", "x", "pixels");

            auto ys = readJsonEntry<double>(j, "stem", "area", "y", "start");
            auto yf = readJsonEntry<double>(j, "stem", "area", "y", "finish");
            auto yp = readJsonEntry<int>(j, "stem", "scan", "y", "pixels");

            auto pad = readJsonEntry<double>(j, "stem", "area", "padding", "val");

            StemArea ar(xs, xf, ys, yf, xp, yp, pad);
            auto area = man.stemArea();
            *area = ar;
            if (man.mode() == SimulationMode::STEM)
                area_set = true;
        } catch (std::exception& e) {}

        try { man.setParallelPixels(readJsonEntry<unsigned int>(j, "stem", "static area", "concurrent pixels"));
        } catch (std::exception& e) {}

        try { man.setParallelStem(readJsonEntry<bool>(j, "stem", "static area", "enabled"));
        } catch (std::exception& e) {}

        // detectors...

        try {
            json det_section = readJsonEntry<json>(j, "stem", "detectors");
            for (json::iterator it = det_section.begin(); it != det_section.end(); ++it) {
                std::string entry = it.key();
                try{
                    auto inner = readJsonEntry<unsigned int>(det_section, entry, "radius", "inner");
                    auto outer = readJsonEntry<unsigned int>(det_section, entry, "radius", "outer");
                    auto xc = readJsonEntry<unsigned int>(det_section, entry, "centre", "x");
                    auto yc = readJsonEntry<unsigned int>(det_section, entry, "centre", "y");

                    StemDetector d(entry, inner, outer, xc, yc);

                    man.stemDetectors().push_back(d);

                } catch (std::exception& e) {}
            }

        } catch (std::exception& e) {}

        //
        // CBED
        //

        try {
            auto x = readJsonEntry<double>(j, "cbed", "position", "x");
            auto y = readJsonEntry<double>(j, "cbed", "position", "y");
            auto pad = readJsonEntry<double>(j, "cbed", "position", "padding");

            CbedPosition ar(x, y, pad);
            auto area = man.cbedPosition();
            *area = ar;
            if (man.mode() == SimulationMode::CBED)
                area_set = true; // I don't know if this one matters...
        } catch (std::exception& e) {}


        //
        // incoherence effects
        //

        try {
            man.incoherenceEffects()->setIterations(
                    readJsonEntry<unsigned int>(j, "incoherence", "iterations"));
        } catch (std::exception& e) {}

        // TEM effects are in the microscope part

        // chromatic

        try {
            man.incoherenceEffects()->chromatic()->setEnabled(
                    readJsonEntry<bool>(j, "incoherence", "probe", "chromatic", "enabled"));
        } catch (std::exception& e) {}

        try {
            man.incoherenceEffects()->chromatic()->setChromaticAberration(
                    readJsonEntry<double>(j, "incoherence", "probe", "chromatic", "Cc", "val"));
        } catch (std::exception& e) {}

        try {
            man.incoherenceEffects()->chromatic()->setHalfWidthHalfMaxPositive(
                    readJsonEntry<double>(j, "incoherence", "probe", "chromatic", "dE", "HWHM +"));
        } catch (std::exception& e) {}

        try {
            man.incoherenceEffects()->chromatic()->setHalfWidthHalfMaxNegative(
                    readJsonEntry<double>(j, "incoherence", "probe", "chromatic", "dE", "HWHM -"));
        } catch (std::exception& e) {}

        // source size

        try {
            man.incoherenceEffects()->source()->setEnabled(
                    readJsonEntry<bool>(j, "incoherence", "probe", "source size", "enabled"));
        } catch (std::exception& e) {}

        try {
            man.incoherenceEffects()->source()->setFullWidthHalfMax(
                    readJsonEntry<double>(j, "incoherence", "probe", "source size", "FWHM", "val"));
        } catch (std::exception& e) {}

        //
        // Inelastic scattering
        //


        // phonon

        *(man.incoherenceEffects()->phonons()) = JsonToThermalVibrations(j);

        try {
            man.setParallelPotentialsCount(
                    readJsonEntry<unsigned int>(j, "incoherence", "inelastic scattering", "phonon",
                                                "mixed static potentials", "count"));
        } catch (std::exception& e) {}

        try { man.setUseParallelPotentials(readJsonEntry<bool>(j, "incoherence", "inelastic scattering", "phonon", "mixed static potentials", "enabled"));
        } catch (std::exception& e) {}

        // plasmon

        try {
            man.incoherenceEffects()->plasmons()->setEnabled(readJsonEntry<bool>(j, "incoherence", "inelastic scattering", "plasmon", "enabled"));
        } catch (std::exception& e) {}

        try {
            std::string p_type = readJsonEntry<std::string>(j, "incoherence", "inelastic scattering", "plasmon", "type");
            if (p_type == "full")
                man.incoherenceEffects()->plasmons()->setSimType(PlasmonType::Full);
            else if (p_type == "individual")
                man.incoherenceEffects()->plasmons()->setSimType(PlasmonType::Individual);
        } catch (std::exception& e) {}

        try {
            man.incoherenceEffects()->plasmons()->setIndividualPlasmon(readJsonEntry<unsigned int>(j, "incoherence", "inelastic scattering", "plasmon", "individual"));
        } catch (std::exception& e) {}

        try {
            man.incoherenceEffects()->plasmons()->setMeanFreePath(readJsonEntry<double>(j, "incoherence", "inelastic scattering", "plasmon", "mean free path", "value") * 10); // convert nm to angstroms
        } catch (std::exception& e) {}

        try {
            man.incoherenceEffects()->plasmons()->setCharacteristicAngle(readJsonEntry<double>(j, "incoherence", "inelastic scattering", "plasmon", "characteristic angle", "value"));
        } catch (std::exception& e) {}

        try {
            man.incoherenceEffects()->plasmons()->setCriticalAngle(readJsonEntry<double>(j, "incoherence", "inelastic scattering", "plasmon", "critical angle", "value"));
        } catch (std::exception& e) {}

        //
        // All done!
        //

        return man;
    }

    PhononScattering JsonToThermalVibrations(json& j) {

        PhononScattering out_therms;


        bool frozen_phonon_enabled = false;
        bool force_default = false;
        bool override_file = false;
        double def = 0.0;

        std::vector<double> vibs;
        std::vector<int> els;

        try {
            frozen_phonon_enabled = readJsonEntry<bool>(j, "incoherence", "inelastic scattering", "phonon", "enabled");
        } catch (std::exception& e) {}

        try { force_default = readJsonEntry<bool>(j, "incoherence", "inelastic scattering", "phonon", "force default");
        } catch (std::exception& e) {}

        try { override_file = readJsonEntry<bool>(j, "incoherence", "inelastic scattering", "phonon", "override file");
        } catch (std::exception& e) {}

        try { def = readJsonEntry<double>(j, "incoherence", "inelastic scattering", "phonon", "default", "value");
        } catch (std::exception& e) {}

        try {
            json element_section = readJsonEntry<json>(j, "incoherence", "inelastic scattering", "phonon", "values");
            for (json::iterator it = element_section.begin(); it != element_section.end(); ++it) {
                std::string element = it.key();
                try{
                    auto v = readJsonEntry<double>(element_section, element);
                    els.emplace_back( Utils::ElementSymbolToNumber(element) );
                    vibs.emplace_back(v);
                } catch (std::exception& e) {}
            }

        } catch (std::exception& e) {}

        out_therms.setFrozenPhononEnabled(frozen_phonon_enabled);
        out_therms.setVibrations(def, els, vibs);
        out_therms.setForceDefined(override_file);
        out_therms.setForceDefault(force_default);

        return out_therms;
    }

    json FullManagerToJson(SimulationManager& man) {
        // this gets pretty much everything that is set
        json j = BasicManagerToJson(man, true, false);

        for (auto det : man.stemDetectors())
        {
            j["stem"]["detectors"][det.name] = JSONUtils::stemDetectorToJson(det);
        }

        return j;
    }

    json BasicManagerToJson(SimulationManager& man, bool force_all, bool sim_relevant) {

        json j;

        bool have_structure = man.simulationCell()->crystalStructure().get() == nullptr;

        // no file input here as it is not always needed

        j["double precision"] = man.doublePrecisionEnabled();

        auto mode = man.mode();
        j["mode"]["id"] = mode;
        j["mode"]["name"] = man.modeString();

        j["potentials"] = man.structureParameters().name;

        j["resolution"] = man.resolution();

        j["slice thickness"]["val"] = man.simulationCell()->sliceThickness();
        j["slice thickness"]["units"] = "Å";

        j["slice offset"]["val"] = man.simulationCell()->sliceOffset();
        j["slice offset"]["units"] = "Å";

        if (have_structure && sim_relevant)
            j["slice count"] = man.simulationCell()->sliceCount();

        j["intermediate output"]["slice interval"] = man.intermediateSliceStep();
        j["intermediate output"]["enabled"] = man.intermediateSlicesEnabled();

        j["maintain areas"] = man.maintainAreas();

        if (sim_relevant && have_structure) {
            auto xl = man.paddedSimLimitsX(0);
            auto yl = man.paddedSimLimitsY(0);
            auto zl = man.paddedSimLimitsZ(); // z never changes, so always is struct limits

            // padding is always plus/minus one value, with the second element ([1]) being positive
            auto xp = man.simulationCell()->paddingX()[1];
            auto yp = man.simulationCell()->paddingY()[1];
            auto zp = man.simulationCell()->paddingZ()[1];

            j["simulation area"]["x"]["start"] = xl[0];
            j["simulation area"]["x"]["finish"] = xl[1];
            j["simulation area"]["x"]["padding"] = xp;
            j["simulation area"]["x"]["units"] = "Å";
            j["simulation area"]["y"]["start"] = yl[0];
            j["simulation area"]["y"]["finish"] = yl[1];
            j["simulation area"]["y"]["padding"] = yp;
            j["simulation area"]["y"]["units"] = "Å";
            j["simulation area"]["z"]["start"] = zl[0];
            j["simulation area"]["z"]["finish"] = zl[1];
            j["simulation area"]["z"]["padding"] = zp;
            j["simulation area"]["z"]["units"] = "Å";
        }

        auto p_z_d_val = man.simulationCell()->defaultPaddingZ();
        auto p_xy_d_val = man.simulationCell()->defaultPaddingXY();

        j["default padding"]["z"]["val"] = p_z_d_val[1];
        j["default padding"]["z"]["units"] = "Å";
        j["default padding"]["xy"]["val"] = p_xy_d_val[1];
        j["default padding"]["xy"]["units"] = "Å";

        // TODO: could put some pixel scale things here for convenience? (would have to distinguish between diff maybe?)

        bool f3d = man.full3dEnabled();

        if(f3d || force_all) {
            j["full 3d"]["state"] = f3d;
            j["full 3d"]["integrals"] = man.full3dIntegrals();
        } else
            j["full 3d"]["state"] = f3d;

        j["precalculate transmission"] = man.precalculateTransmission();

        //
        //
        //

        auto mp = man.microscopeParams();

        j["microscope"]["voltage"]["val"] = mp->Voltage;
        j["microscope"]["voltage"]["units"] = "kV";

        // TODO: check which parameters are relevant to which modes
        // TODO: get whether CTEM is image, EW or diff...

        // ctem image stuffs
        if (mode == SimulationMode::CTEM || force_all) {
            j["microscope"]["objective aperture"]["semi-angle"] = mp->ObjectiveAperture;
            j["microscope"]["objective aperture"]["smoothing"] =  mp->ObjectiveApertureSmoothing;
            j["microscope"]["objective aperture"]["units"] = "mrad";

            j["microscope"]["alpha"]["val"] = mp->Alpha;
            j["microscope"]["alpha"]["units"] = "mrad";

            j["microscope"]["delta"]["val"] = mp->Delta / 10;
            j["microscope"]["delta"]["units"] = "nm";
        }

        if (mode != SimulationMode::CTEM || force_all) {
            j["microscope"]["condenser aperture"]["semi-angle"] = mp->CondenserAperture;
            j["microscope"]["condenser aperture"]["smoothing"] = mp->CondenserApertureSmoothing;
            j["microscope"]["condenser aperture"]["units"] = "mrad";
        }

        j["microscope"]["beam tilt"]["inclination"]["val"] = mp->BeamTilt;
        j["microscope"]["beam tilt"]["inclination"]["units"] = "mrad";

        j["microscope"]["beam tilt"]["azimuth"]["val"] = mp->BeamAzimuth * 180 / Constants::Pi;
        j["microscope"]["beam tilt"]["azimuth"]["units"] = "°";

        // aberration values
        j["microscope"]["aberrations"]["C10"]["val"] = mp->C10 / 10;
        j["microscope"]["aberrations"]["C10"]["units"] = "nm";
        j["microscope"]["aberrations"]["C12"]["mag"] = mp->C12.Mag / 10;
        j["microscope"]["aberrations"]["C12"]["ang"] = mp->C12.Ang * (180 / Constants::Pi);
        j["microscope"]["aberrations"]["C12"]["units"] = "nm, °";

        j["microscope"]["aberrations"]["C21"]["mag"] = mp->C21.Mag / 10;
        j["microscope"]["aberrations"]["C21"]["ang"] = mp->C21.Ang * (180 / Constants::Pi);
        j["microscope"]["aberrations"]["C21"]["units"] = "nm, °";
        j["microscope"]["aberrations"]["C23"]["mag"] = mp->C23.Mag / 10;
        j["microscope"]["aberrations"]["C23"]["ang"] = mp->C23.Ang * (180 / Constants::Pi);
        j["microscope"]["aberrations"]["C23"]["units"] = "nm, °";

        j["microscope"]["aberrations"]["C30"]["val"] = mp->C30 / 10000;
        j["microscope"]["aberrations"]["C30"]["units"] = "μm";
        j["microscope"]["aberrations"]["C32"]["mag"] = mp->C32.Mag / 10000;
        j["microscope"]["aberrations"]["C32"]["ang"] = mp->C32.Ang * (180 / Constants::Pi);
        j["microscope"]["aberrations"]["C32"]["units"] = "μm, °";
        j["microscope"]["aberrations"]["C34"]["mag"] = mp->C34.Mag / 10000;
        j["microscope"]["aberrations"]["C34"]["ang"] = mp->C34.Ang * (180 / Constants::Pi);
        j["microscope"]["aberrations"]["C34"]["units"] = "μm, °";

        j["microscope"]["aberrations"]["C41"]["mag"] = mp->C41.Mag / 10000;
        j["microscope"]["aberrations"]["C41"]["ang"] = mp->C41.Ang * (180 / Constants::Pi);
        j["microscope"]["aberrations"]["C41"]["units"] = "μm, °";
        j["microscope"]["aberrations"]["C43"]["mag"] = mp->C43.Mag / 10000;
        j["microscope"]["aberrations"]["C43"]["ang"] = mp->C43.Ang * (180 / Constants::Pi);
        j["microscope"]["aberrations"]["C43"]["units"] = "μm, °";
        j["microscope"]["aberrations"]["C45"]["mag"] = mp->C45.Mag / 10000;
        j["microscope"]["aberrations"]["C45"]["ang"] = mp->C45.Ang * (180 / Constants::Pi);
        j["microscope"]["aberrations"]["C45"]["units"] = "μm, °";

        j["microscope"]["aberrations"]["C50"]["val"] = mp->C50 / 10000;
        j["microscope"]["aberrations"]["C50"]["units"] = "μm";
        j["microscope"]["aberrations"]["C52"]["mag"] = mp->C52.Mag / 10000;
        j["microscope"]["aberrations"]["C52"]["ang"] = mp->C52.Ang * (180 / Constants::Pi);
        j["microscope"]["aberrations"]["C52"]["units"] = "μm, °";
        j["microscope"]["aberrations"]["C54"]["mag"] = mp->C54.Mag / 10000;
        j["microscope"]["aberrations"]["C54"]["ang"] = mp->C54.Ang * (180 / Constants::Pi);
        j["microscope"]["aberrations"]["C54"]["units"] = "μm, °";
        j["microscope"]["aberrations"]["C56"]["mag"] = mp->C56.Mag / 10000;
        j["microscope"]["aberrations"]["C56"]["ang"] = mp->C56.Ang * (180 / Constants::Pi);
        j["microscope"]["aberrations"]["C56"]["units"] = "μm, °";

        // If CTEM, get dose/CCD stuff
        if (mode == SimulationMode::CTEM || force_all)
        {
            // cropped padding entry will be added at the time of saving

            bool do_ctem_im = man.ctemImageEnabled();

            j["ctem"]["simulate image"] = do_ctem_im;

            if (do_ctem_im) {
                if (CCDParams::nameExists(man.ccdName()) || force_all) {
                    j["ctem"]["ccd"]["name"] = man.ccdName();
                    j["ctem"]["ccd"]["dose"]["val"] = man.ccdDose();
                    j["ctem"]["ccd"]["dose"]["units"] = "e- per square Å";
                    j["ctem"]["ccd"]["binning"] = man.ccdBinning();
                } else {
                    j["ctem"]["ccd"] = "Perfect";
                }
            }

            j["ctem"]["area"]["x"]["start"] = man.ctemArea().getRawLimitsX()[0];
            j["ctem"]["area"]["x"]["finish"] = man.ctemArea().getRawLimitsX()[1];

            j["ctem"]["area"]["y"]["start"] = man.ctemArea().getRawLimitsY()[0];
            j["ctem"]["area"]["y"]["finish"] = man.ctemArea().getRawLimitsY()[1];
        }

        // If STEM, get scan info, TDS info detector info will be added later if needed
        if (mode == SimulationMode::STEM || force_all)
        {
            auto sa = man.stemArea();
            j["stem"]["area"]["x"]["start"] = sa->getRawLimitsX()[0];
            j["stem"]["area"]["x"]["finish"] = sa->getRawLimitsX()[1];
            j["stem"]["scan"]["x"]["pixels"] = sa->getPixelsX();;
            j["stem"]["area"]["x"]["units"] = "Å";
            j["stem"]["area"]["y"]["start"] = sa->getRawLimitsY()[0];
            j["stem"]["area"]["y"]["finish"] = sa->getRawLimitsY()[1];
            j["stem"]["scan"]["y"]["pixels"] = sa->getPixelsY();
            j["stem"]["area"]["y"]["units"] = "Å";
            j["stem"]["area"]["padding"]["val"] = sa->getPadding();
            j["stem"]["area"]["padding"]["units"] = "Å";

            // stem detector bit...

            j["stem"]["static area"]["concurrent pixels"] = man.parallelPixels();
            j["stem"]["static area"]["enabled"] = man.parallelStem();
        }

        // If CBED, get position info
        if (mode == SimulationMode::CBED || force_all)
        {
            j["cbed"]["position"]["x"] = man.cbedPosition()->getXPos();
            j["cbed"]["position"]["y"] = man.cbedPosition()->getYPos();
            j["cbed"]["position"]["padding"] = man.cbedPosition()->getPadding();
            j["cbed"]["position"]["units"] = "Å";
        }

        //
        // Incoherent effects
        //

        auto inel = man.incoherenceEffects();
        bool inelastic_used = inel->enabled(man.mode());

        if (inelastic_used || force_all)
            j["incoherence"]["iterations"] = man.incoherenceEffects()->storedIterations();

        // probe only information

        if ((inelastic_used && man.isProbeSimulation()) || force_all) {
            // used stored interations in case of force all

            auto chrome = inel->chromatic();
            bool cc_used = chrome->enabled();
            j["incoherence"]["probe"]["chromatic"]["enabled"] = cc_used;

            if(cc_used || force_all) {
                j["incoherence"]["probe"]["chromatic"]["Cc"]["val"] = chrome->chromaticAberration();
                j["incoherence"]["probe"]["chromatic"]["Cc"]["units"] = "mm";

                j["incoherence"]["probe"]["chromatic"]["dE"]["HWHM +"] = chrome->halfWidthHalfMaxPositive();
                j["incoherence"]["probe"]["chromatic"]["dE"]["HWHM -"] = chrome->halfWidthHalfMaxNegative();
                j["incoherence"]["probe"]["chromatic"]["dE"]["units"] = "eV";
            }

            auto p_source = inel->source();
            bool ps_used = p_source->enabled();

            j["incoherence"]["probe"]["source size"]["enabled"] = ps_used;

            if(ps_used || force_all) {
                j["incoherence"]["probe"]["source size"]["FWHM"]["val"] = p_source->fullWidthHalfMax();
                j["incoherence"]["probe"]["source size"]["FWHM"]["units"] = "Å";
            }
        }

        //
        // Inelastic scattering
        //

        // phonon

        bool phonon_used = man.incoherenceEffects()->phonons()->getFrozenPhononEnabled();
        if (phonon_used || force_all) {

            // don't export if we have file defined vibrations and no override
            bool export_thermals = true;
            if (man.simulationCell()->crystalStructure()) // structure does not always exist
                export_thermals = !(man.simulationCell()->crystalStructure()->thermalFileDefined() &&
                                    !man.incoherenceEffects()->phonons()->forceDefault() &&
                                    !man.incoherenceEffects()->phonons()->forceDefined());

            if (export_thermals || force_all) {
                if (force_all)
                    j["incoherence"]["inelastic scattering"]["phonon"] = thermalVibrationsToJson(man);
            } else {
                j["incoherence"]["inelastic scattering"]["phonon"] = "input file defined";
            }

            if (man.useParallelPotentials() || force_all) {
                j["incoherence"]["inelastic scattering"]["phonon"]["mixed static potentials"]["count"] = man.storedParallelPotentialsCount();
                j["incoherence"]["inelastic scattering"]["phonon"]["mixed static potentials"]["enabled"] = man.storedUseParallelPotentials();
            } else if (man.useParallelPotentials()) {
                j["incoherence"]["inelastic scattering"]["phonon"]["mixed static potentials"]["count"] = man.parallelPotentialsCount();
                j["incoherence"]["inelastic scattering"]["phonon"]["mixed static potentials"]["enabled"] = man.useParallelPotentials();
            }


            try {
                man.setParallelPotentialsCount(
                        readJsonEntry<unsigned int>(j, "incoherence", "inelastic scattering", "phonon",
                                                    "mixed static potentials", "count"));
            } catch (std::exception& e) {}

            try { man.setUseParallelPotentials(readJsonEntry<bool>(j, "incoherence", "inelastic scattering", "phonon", "mixed static potentials", "enabled"));
            } catch (std::exception& e) {}
        }

        // plasmon

        auto plasmon = man.incoherenceEffects()->plasmons();
        bool plasmon_used = plasmon->enabled();
        if (plasmon_used || force_all) {
            j["incoherence"]["inelastic scattering"]["plasmon"]["enabled"] = plasmon_used;

            if (plasmon->simType() == PlasmonType::Full)
                j["incoherence"]["inelastic scattering"]["plasmon"]["type"] = "full";
            else if (man.incoherenceEffects()->plasmons()->simType() == PlasmonType::Individual)
                j["incoherence"]["inelastic scattering"]["plasmon"]["type"] = "individual";

            if (plasmon->simType() == PlasmonType::Individual || force_all)
                j["incoherence"]["inelastic scattering"]["plasmon"]["individual"] = plasmon->individualPlasmon();

            j["incoherence"]["inelastic scattering"]["plasmon"]["mean free path"]["value"] = plasmon->meanFreePath() / 10;
            j["incoherence"]["inelastic scattering"]["plasmon"]["mean free path"]["units"] = "nm";

            j["incoherence"]["inelastic scattering"]["plasmon"]["characteristic angle"]["value"] = plasmon->characteristicAngle();
            j["incoherence"]["inelastic scattering"]["plasmon"]["characteristic angle"]["units"] = "mrad";

            j["incoherence"]["inelastic scattering"]["plasmon"]["critical angle"]["value"] = plasmon->criticalAngle();
            j["incoherence"]["inelastic scattering"]["plasmon"]["critical angle"]["units"] = "mrad";
        }

        // this is only valid if we have actually saved a simulation, so we can't force it
        if (plasmon_used && plasmon->simType() == PlasmonType::Full) {

            auto pn = man.incoherenceEffects()->plasmons()->getPlasmonNumbers();

            // add leading zeros to make it be in correct order in json
            // (this is only for humands, it is never used by code)

            unsigned int max_n = 0;
            for (auto & i : pn)
                if (i[0] > max_n)
                    max_n = i[0];
            unsigned int str_len = std::to_string(max_n).size();


            for (auto & i : pn) {
                std::string str_n = std::to_string(i[0]);
                str_n = std::string(str_len - str_n.length(), '0') + str_n;

                j["incoherence"]["inelastic scattering"]["plasmon"]["full"]["plasmon numbers"][str_n] = i[1];
            }
        }

        //
        // All done!
        //

        return j;
    }

    json stemDetectorToJson(StemDetector d) {
        json j;

        j["radius"]["inner"] = d.inner;
        j["radius"]["outer"] = d.outer;
        j["radius"]["units"] = "mrad";
        j["centre"]["x"] = d.xcentre;
        j["centre"]["y"] = d.ycentre;
        j["centre"]["units"] = "mrad";

        return j;
    }

    json thermalVibrationsToJson(SimulationManager& man) {
        json j;

        j["enabled"] = man.incoherenceEffects()->phonons()->getFrozenPhononEnabled();

        j["force default"] = man.incoherenceEffects()->phonons()->forceDefault();
        j["override file"] = man.incoherenceEffects()->phonons()->forceDefined();

        j["default"]["value"] = man.incoherenceEffects()->phonons()->getDefault();
        j["default"]["units"] = "Å²";

        auto els = man.incoherenceEffects()->phonons()->getDefinedElements();
        auto vibs = man.incoherenceEffects()->phonons()->getDefinedVibrations();

        if (els.size() != vibs.size())
            throw std::runtime_error("cannot write thermal parameters to json file: element and displacement vectors have different size");

        for(int i = 0; i < els.size(); ++i) {
            j["values"][ Utils::NumberToElementSymbol(els[i]) ] = vibs[i];
            j["values"]["units"] = "Å²";
        }

        return j;
    }

}