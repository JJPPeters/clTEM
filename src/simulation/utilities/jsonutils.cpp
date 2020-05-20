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
        try { man.setDoDoublePrecision( readJsonEntry<bool>(j, "double precision") );
        } catch (std::exception& e) {}

        try {
            auto mode = readJsonEntry<SimulationMode>(j, "mode", "id");
            if (mode == SimulationMode::None)
                mode = SimulationMode::CTEM;
            man.setMode( mode );
        } catch (std::exception& e) {}

        try { man.setResolution( readJsonEntry<unsigned int>(j, "resolution") );
        } catch (std::exception& e) {}

        try { man.setSliceThickness( readJsonEntry<double>(j, "slice thickness", "val") );
        } catch (std::exception& e) {}

        try { man.setSliceOffset( readJsonEntry<double>(j, "slice offset", "val") );
        } catch (std::exception& e) {}

        try { man.setIntermediateSlices( readJsonEntry<unsigned int>(j, "intermediate output", "slice interval") );
        } catch (std::exception& e) {}

        try { man.setIntermediateSlicesEnabled( readJsonEntry<bool>(j, "intermediate output", "enabled") );
        } catch (std::exception& e) {}

        try { man.setFull3d( readJsonEntry<bool>(j, "full 3d", "state") );
        } catch (std::exception& e) {}

        try { man.setFull3dInts( readJsonEntry<unsigned int>(j, "full 3d", "integrals") );
        } catch (std::exception& e) {}

        try { man.setMaintainAreas( readJsonEntry<bool>(j, "maintain areas") );
        } catch (std::exception& e) {}

        try { man.setStructureParameters( readJsonEntry<std::string>(j, "potentials") );
        } catch (std::exception& e) {}

        try {
            auto p_xy_val = readJsonEntry<double>(j, "default padding", "xy", "val");
            man.setDefaultPaddingXY({-p_xy_val, p_xy_val});
        } catch (std::exception& e) {}

        try {
            auto p_z_val = readJsonEntry<double>(j, "default padding", "z", "val");
            man.setDefaultPaddingZ({-p_z_val, p_z_val});
        } catch (std::exception& e) {}

        //
        // Do all the microscope parameters stuff...
        //

        auto mp = man.getMicroscopeParams();

        try { mp->Voltage = readJsonEntry<double>(j, "microscope", "voltage", "val");
        } catch (std::exception& e) {}

        try { mp->Aperture = readJsonEntry<double>(j, "microscope", "aperture", "val");
        } catch (std::exception& e) {}

        try { mp->ApertureSmoothing = readJsonEntry<double>(j, "microscope", "aperture smooth radius", "val");
        } catch (std::exception& e) {}

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

        try { man.setSimulateCtemImage(readJsonEntry<bool>(j, "ctem", "simulate image"));
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
            auto area = man.getSimulationArea();
            *area = ar;
            if (man.getMode() == SimulationMode::CTEM)
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
            auto area = man.getStemArea();
            *area = ar;
            if (man.getMode() == SimulationMode::STEM)
                area_set = true;
        } catch (std::exception& e) {}

        try { man.setParallelPixels(readJsonEntry<unsigned int>(j, "stem", "concurrent pixels"));
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

                    man.getDetectors().push_back(d);

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
            auto area = man.getCBedPosition();
            *area = ar;
            if (man.getMode() == SimulationMode::CBED)
                area_set = true; // I don't know if this one matters...
        } catch (std::exception& e) {}


        //
        // Inelastic scattering
        //

        try { man.getInelasticScattering()->setInelasticIterations(readJsonEntry<unsigned int>(j, "inelastic scattering", "iterations"));
        } catch (std::exception& e) {}

        // phonon

        try { man.getInelasticScattering()->getPhonons()->setFrozenPhononEnabled(readJsonEntry<bool>(j, "inelastic scattering", "phonon", "frozen phonon", "enabled"));
        } catch (std::exception& e) {}

        *(man.getInelasticScattering()->getPhonons()) = JsonToThermalVibrations(j);

        // plasmon

        try {
            man.getInelasticScattering()->getPlasmons()->setCombinedEnabled(
                    readJsonEntry<bool>(j, "inelastic scattering", "plasmon", "full", "enabled"));
        } catch (std::exception& e) {}

        try {
            man.getInelasticScattering()->getPlasmons()->setIndividualEnabled(
                    readJsonEntry<bool>(j, "inelastic scattering", "plasmon", "individual", "enabled"));
        } catch (std::exception& e) {}

        try { man.getInelasticScattering()->getPlasmons()->setIndividualPlasmon(readJsonEntry<unsigned int>(j, "inelastic scattering", "plasmon", "individual", "number"));
        } catch (std::exception& e) {}

        try { man.getInelasticScattering()->getPlasmons()->setMeanFreePath(readJsonEntry<double>(j, "inelastic scattering", "plasmon", "mean free path", "value") * 10); // convert nm to angstroms
        } catch (std::exception& e) {}

        try { man.getInelasticScattering()->getPlasmons()->setCharacteristicAngle(readJsonEntry<double>(j, "inelastic scattering", "plasmon", "characteristic angle", "value"));
        } catch (std::exception& e) {}

        try { man.getInelasticScattering()->getPlasmons()->setCriticalAngle(readJsonEntry<double>(j, "inelastic scattering", "plasmon", "critical angle", "value"));
        } catch (std::exception& e) {}

        //
        // All done!
        //

        return man;
    }

    PhononScattering JsonToThermalVibrations(json& j) {

        PhononScattering out_therms;

        bool force_default = false;
        bool override_file = false;
        double def = 0.0;

        std::vector<double> vibs;
        std::vector<int> els;

        try { force_default = readJsonEntry<bool>(j, "inelastic scattering", "phonon","thermal parameters", "force default");
        } catch (std::exception& e) {}

        try { override_file = readJsonEntry<bool>(j, "inelastic scattering", "phonon", "thermal parameters", "override file");
        } catch (std::exception& e) {}

        try { def = readJsonEntry<double>(j, "inelastic scattering", "phonon", "thermal parameters", "default");
        } catch (std::exception& e) {}

        try {
            json element_section = readJsonEntry<json>(j, "inelastic scattering", "phonon", "thermal parameters", "values");
            for (json::iterator it = element_section.begin(); it != element_section.end(); ++it) {
                std::string element = it.key();
                try{
                    auto v = readJsonEntry<double>(element_section, element);
                    els.emplace_back( Utils::ElementSymbolToNumber(element) );
                    vibs.emplace_back(v);
                } catch (std::exception& e) {}
            }

        } catch (std::exception& e) {}

        out_therms.setVibrations(def, els, vibs);
        out_therms.force_defined = override_file;
        out_therms.force_default = force_default;

        return out_therms;
    }

    json FullManagerToJson(SimulationManager& man) {
        // this gets pretty much everything that is set
        json j = BasicManagerToJson(man, true);

        for (auto det : man.getDetectors())
        {
            j["stem"]["detectors"][det.name] = JSONUtils::stemDetectorToJson(det);
        }

        return j;
    }

    json BasicManagerToJson(SimulationManager& man, bool force_all) {

        json j;

        // no file input here as it is not always needed

        j["double precision"] = man.getDoDoublePrecision();

        auto mode = man.getMode();
        j["mode"]["id"] = mode;
        j["mode"]["name"] = man.getModeString();

        j["potentials"] = man.getStructureParametersName();

        j["resolution"] = man.getResolution();

        j["slice thickness"]["val"] = man.getSliceThickness();
        j["slice thickness"]["units"] = "Å";

        j["slice offset"]["val"] = man.getSliceOffset();
        j["slice offset"]["units"] = "Å";

        j["slice count"] = man.getNumberofSlices();

        j["intermediate output"]["slice interval"] = man.getIntermediateSlices();
        j["intermediate output"]["enabled"] = man.getIntermediateSlicesEnabled();

        j["maintain areas"] = man.getMaintainAreas();

        auto xl = man.getPaddedSimLimitsX(0);
        auto yl = man.getPaddedSimLimitsY(0);
        auto zl = man.getPaddedStructLimitsZ(); // z never changes, so always is struct limits

        // padding is always plus/minus one value, with the second element ([1]) being positive
        auto xp = man.getPaddingX()[1];
        auto yp = man.getPaddingY()[1];
        auto zp = man.getPaddingZ()[1];

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


        auto p_z_d_val = man.getDefaultPaddingZ();
        auto p_xy_d_val = man.getDefaultPaddingXY();

        j["default padding"]["z"]["val"] = p_z_d_val[1];
        j["default padding"]["z"]["units"] = "Å";
        j["default padding"]["xy"]["val"] = p_xy_d_val[1];
        j["default padding"]["xy"]["units"] = "Å";

        // TODO: could put some pixel scale things here for convenience? (would have to distinguish between diff maybe?)

        bool f3d = man.isFull3d();

        if(f3d || force_all) {
            j["full 3d"]["state"] = f3d;
            j["full 3d"]["integrals"] = man.getFull3dInts();
        } else
            j["full 3d"]["state"] = f3d;

        auto mp = man.getMicroscopeParams();

        j["microscope"]["voltage"]["val"] = mp->Voltage;
        j["microscope"]["voltage"]["units"] = "kV";

        // TODO: check which parameters are relevant to which modes
        // TODO: get whether CTEM is image, EW or diff...

        // apert
        j["microscope"]["aperture"]["val"] = mp->Aperture;
        j["microscope"]["aperture"]["units"] = "mrad";

        if (mode != SimulationMode::CTEM || force_all) {
            // alpha
            j["microscope"]["aperture smooth radius"]["val"] = mp->ApertureSmoothing;
            j["microscope"]["aperture smooth radius"]["units"] = "mrad";
        }

        j["microscope"]["beam tilt"]["inclination"]["val"] = mp->BeamTilt;
        j["microscope"]["beam tilt"]["inclination"]["units"] = "mrad";

        j["microscope"]["beam tilt"]["azimuth"]["val"] = mp->BeamAzimuth * 180 / Constants::Pi;
        j["microscope"]["beam tilt"]["azimuth"]["units"] = "°";


        if (mode == SimulationMode::CTEM || force_all) {
            // alpha
            j["microscope"]["alpha"]["val"] = mp->Alpha;
            j["microscope"]["alpha"]["units"] = "mrad";

            // delta
            j["microscope"]["delta"]["val"] = mp->Delta / 10;
            j["microscope"]["delta"]["units"] = "nm";
        }

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

            bool do_ctem_im = man.getSimulateCtemImage();

            j["ctem"]["simulate image"] = do_ctem_im;

            if (do_ctem_im) {
                if (CCDParams::nameExists(man.getCcdName()) || force_all) {
                    j["ctem"]["ccd"]["name"] = man.getCcdName();
                    j["ctem"]["ccd"]["dose"]["val"] = man.getCcdDose();
                    j["ctem"]["ccd"]["dose"]["units"] = "e- per square Å";
                    j["ctem"]["ccd"]["binning"] = man.getCcdBinning();
                } else {
                    j["ctem"]["ccd"] = "Perfect";
                }
            }

            j["ctem"]["area"]["x"]["start"] = man.getCtemArea().getRawLimitsX()[0];
            j["ctem"]["area"]["x"]["finish"] = man.getCtemArea().getRawLimitsX()[1];

            j["ctem"]["area"]["y"]["start"] = man.getCtemArea().getRawLimitsY()[0];
            j["ctem"]["area"]["y"]["finish"] = man.getCtemArea().getRawLimitsY()[1];
        }

        // If STEM, get scan info, TDS info detector info will be added later if needed
        if (mode == SimulationMode::STEM || force_all)
        {
            auto sa = man.getStemArea();
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

            j["stem"]["concurrent pixels"] = man.getParallelPixels();
        }

        // If CBED, get position info and TDS info
        if (mode == SimulationMode::CBED || force_all)
        {
            j["cbed"]["position"]["x"] = man.getCBedPosition()->getXPos();
            j["cbed"]["position"]["y"] = man.getCBedPosition()->getYPos();
            j["cbed"]["position"]["padding"] = man.getCBedPosition()->getPadding();
            j["cbed"]["position"]["units"] = "Å";
        }

        //
        // Inelastic scattering
        //

        bool inelastic_used = (mode == SimulationMode::CBED || mode == SimulationMode::STEM) && man.getInelasticScattering()->getInelasticEnabled();
        if (inelastic_used || force_all)
            j["inelastic scattering"]["iterations"] = man.getInelasticScattering()->getStoredInelasticIterations();

        // phonon

        bool phonon_used = (mode == SimulationMode::CBED || mode == SimulationMode::STEM) && man.getInelasticScattering()->getPhonons()->getFrozenPhononEnabled();
        if (phonon_used || force_all) {

            j["inelastic scattering"]["phonon"]["frozen phonon"]["enabled"] = man.getInelasticScattering()->getPhonons()->getFrozenPhononEnabled();

            // don't export if we have file defined vibrations and no override
            bool export_thermals = true;
            if (man.getStructure()) // structure does not always exist
                export_thermals = !(man.getStructure()->isThermalFileDefined() &&
                                    !man.getInelasticScattering()->getPhonons()->force_default &&
                                    !man.getInelasticScattering()->getPhonons()->force_defined);

            if (export_thermals || force_all) {
                if (mode == SimulationMode::CBED || mode == SimulationMode::STEM || force_all)
                    j["inelastic scattering"]["thermal parameters"]["phonon"] = thermalVibrationsToJson(man);
            } else {
                j["inelastic scattering"]["thermal parameters"]["phonon"] = "input file defined";
            }
        }
        // plasmon

        bool plasmon_used = (mode == SimulationMode::CBED || mode == SimulationMode::STEM) && man.getInelasticScattering()->getPlasmons()->getPlasmonEnabled();
        if (plasmon_used || force_all) {
            j["inelastic scattering"]["plasmon"]["full"]["enabled"] = man.getInelasticScattering()->getPlasmons()->getCombinedEnabled();

            j["inelastic scattering"]["plasmon"]["individual"]["enabled"] = man.getInelasticScattering()->getPlasmons()->getIndividualEnabled();
            j["inelastic scattering"]["plasmon"]["individual"]["number"] = man.getInelasticScattering()->getPlasmons()->getIndividualPlasmon();

            j["inelastic scattering"]["plasmon"]["mean free path"]["value"] = man.getInelasticScattering()->getPlasmons()->getMeanFreePath() / 10;
            j["inelastic scattering"]["plasmon"]["mean free path"]["unit"] = "nm";

            j["inelastic scattering"]["plasmon"]["characteristic angle"]["value"] = man.getInelasticScattering()->getPlasmons()->getCharacteristicAngle();
            j["inelastic scattering"]["plasmon"]["characteristic angle"]["unit"] = "mrad";

            j["inelastic scattering"]["plasmon"]["critical angle"]["value"] = man.getInelasticScattering()->getPlasmons()->getCriticalAngle();
            j["inelastic scattering"]["plasmon"]["critical angle"]["unit"] = "mrad";
        }

        // this is only valid if we have actually saved a simulation, so we can't force it
        if (plasmon_used && man.getInelasticScattering()->getPlasmons()->getCombinedEnabled()) {

            auto pn = man.getInelasticScattering()->getPlasmons()->getPlasmonNumbers();

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

                j["inelastic scattering"]["plasmon"]["full"]["plasmon numbers"][str_n] = i[1];
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

        j["force default"] = man.getInelasticScattering()->getPhonons()->force_default;
        j["override file"] = man.getInelasticScattering()->getPhonons()->force_defined;

        j["default"] = man.getInelasticScattering()->getPhonons()->getDefault();
        j["units"] = "Å²";

        auto els = man.getInelasticScattering()->getPhonons()->getDefinedElements();
        auto vibs = man.getInelasticScattering()->getPhonons()->getDefinedVibrations();

        if (els.size() != vibs.size())
            throw std::runtime_error("cannot write thermal parameters to json file: element and displacement vectors have different size");

        for(int i = 0; i < els.size(); ++i) {
            j["values"][ Utils::NumberToElementSymbol(els[i]) ] = vibs[i];
        }

        return j;
    }

}