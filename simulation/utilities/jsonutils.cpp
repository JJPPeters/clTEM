//
// Created by jon on 04/04/18.
//

#include "jsonutils.h"

namespace JSONUtils {

    SimulationManager JsonToManager(json& j)
    {
        SimulationManager man;
        // not sure there is a particularly easy way to go about this. Just go through all the options...

        try { man.setMode( readJsonEntry<SimulationMode>(j, "mode", "test") );
        } catch (json::out_of_range& e) {}

        try { man.setResolution( readJsonEntry<unsigned int>(j, "resolution") );
        } catch (json::out_of_range& e) {}

        try { man.setSliceThickness( readJsonEntry<float>(j, "slice thickness", "val") );
        } catch (json::out_of_range& e) {}

        try { man.setFull3d( readJsonEntry<bool>(j, "full 3d", "state") );
        } catch (json::out_of_range& e) {}

        try { man.setFull3dInts( readJsonEntry<unsigned int>(j, "full 3d", "integrals") );
        } catch (json::out_of_range& e) {}

        //
        // Do all the microscope parameters stuff...
        //

        auto mp = man.getMicroscopeParams();

        try { mp->Voltage = readJsonEntry<float>(j, "microscope", "voltage", "val");
        } catch (json::out_of_range& e) {}

        try { mp->Aperture = readJsonEntry<float>(j, "microscope", "aperture", "val");
        } catch (json::out_of_range& e) {}

        try { mp->Alpha = readJsonEntry<float>(j, "microscope", "alpha", "val");
        } catch (json::out_of_range& e) {}

        try { mp->Delta = readJsonEntry<float>(j, "microscope", "delta", "val");
        } catch (json::out_of_range& e) {}

        try { mp->C10 = readJsonEntry<float>(j, "microscope", "C10", "val");
        } catch (json::out_of_range& e) {}

        try {
            mp->C12.Mag = readJsonEntry<float>(j, "microscope", "C12", "mag");
            mp->C12.Ang = readJsonEntry<float>(j, "microscope", "C12", "ang");
        } catch (json::out_of_range& e) {}

        try {
            mp->C21.Mag = readJsonEntry<float>(j, "microscope", "C21", "mag");
            mp->C21.Ang = readJsonEntry<float>(j, "microscope", "C21", "ang");
        } catch (json::out_of_range& e) {}

        try {
            mp->C23.Mag = readJsonEntry<float>(j, "microscope", "C23", "mag");
            mp->C23.Ang = readJsonEntry<float>(j, "microscope", "C23", "ang");
        } catch (json::out_of_range& e) {}

        try { mp->C30 = readJsonEntry<float>(j, "microscope", "C30", "val");
        } catch (json::out_of_range& e) {}

        try {
            mp->C32.Mag = readJsonEntry<float>(j, "microscope", "C32", "mag");
            mp->C32.Ang = readJsonEntry<float>(j, "microscope", "C32", "ang");
        } catch (json::out_of_range& e) {}

        try {
            mp->C34.Mag = readJsonEntry<float>(j, "microscope", "C34", "mag");
            mp->C34.Ang = readJsonEntry<float>(j, "microscope", "C34", "ang");
        } catch (json::out_of_range& e) {}

        try {
            mp->C41.Mag = readJsonEntry<float>(j, "microscope", "C41", "mag");
            mp->C41.Ang = readJsonEntry<float>(j, "microscope", "C41", "ang");
        } catch (json::out_of_range& e) {}

        try {
            mp->C43.Mag = readJsonEntry<float>(j, "microscope", "C43", "mag");
            mp->C43.Ang = readJsonEntry<float>(j, "microscope", "C43", "ang");
        } catch (json::out_of_range& e) {}

        try {
            mp->C45.Mag = readJsonEntry<float>(j, "microscope", "C45", "mag");
            mp->C45.Ang = readJsonEntry<float>(j, "microscope", "C45", "ang");
        } catch (json::out_of_range& e) {}

        try { mp->C50 = readJsonEntry<float>(j, "microscope", "C50", "val");
        } catch (json::out_of_range& e) {}

        try {
            mp->C52.Mag = readJsonEntry<float>(j, "microscope", "C52", "mag");
            mp->C52.Ang = readJsonEntry<float>(j, "microscope", "C52", "ang");
        } catch (json::out_of_range& e) {}

        try {
            mp->C54.Mag = readJsonEntry<float>(j, "microscope", "C54", "mag");
            mp->C54.Ang = readJsonEntry<float>(j, "microscope", "C54", "ang");
        } catch (json::out_of_range& e) {}

        try {
            mp->C56.Mag = readJsonEntry<float>(j, "microscope", "C56", "mag");
            mp->C56.Ang = readJsonEntry<float>(j, "microscope", "C56", "ang");
        } catch (json::out_of_range& e) {}

        //
        // Phew, Ctem now
        //

        try { man.setCcdName(readJsonEntry<std::string>(j, "ctem", "ccd", "name"));
        } catch (json::out_of_range& e) {}

        try { man.setCcdDose(readJsonEntry<float>(j, "ctem", "ccd", "dose", "val"));
        } catch (json::out_of_range& e) {}

        try { man.setCcdBinning(readJsonEntry<int>(j, "ctem", "ccd", "binning"));
        } catch (json::out_of_range& e) {}

        try {
            auto xs = readJsonEntry<float>(j, "ctem", "area", "x", "start");
            auto xf = readJsonEntry<float>(j, "ctem", "area", "x", "finish");

            auto ys = readJsonEntry<float>(j, "ctem", "area", "y", "start");
            auto yf = readJsonEntry<float>(j, "ctem", "area", "y", "finish");

            SimulationArea ar(xs, xf, ys, yf);
            auto area = man.getSimulationArea();
            *area = ar;
        } catch (json::out_of_range& e) {}

        //
        // STEM
        //

        try {
            auto xs = readJsonEntry<float>(j, "stem", "area", "x", "start");
            auto xf = readJsonEntry<float>(j, "stem", "area", "x", "finish");
            auto xp = readJsonEntry<int>(j, "stem", "scan", "x", "pixels");

            auto ys = readJsonEntry<float>(j, "stem", "area", "y", "start");
            auto yf = readJsonEntry<float>(j, "stem", "area", "y", "finish");
            auto yp = readJsonEntry<int>(j, "stem", "scan", "y", "pixels");

            auto pad = readJsonEntry<float>(j, "stem", "area", "padding", "val");

            StemArea ar(xs, xf, ys, yf, xp, yp, pad);
            auto area = man.getStemArea();
            *area = ar;
        } catch (json::out_of_range& e) {}

        try { man.setTdsRunsStem(readJsonEntry<unsigned int>(j, "stem", "tds configurations"));
        } catch (json::out_of_range& e) {}

        try { man.setParallelPixels(readJsonEntry<unsigned int>(j, "stem", "concurrent pixels"));
        } catch (json::out_of_range& e) {}

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

                } catch (json::out_of_range& e) {}
            }

        } catch (json::out_of_range& e) {}

        //
        // CBED
        //

        try {
            auto x = readJsonEntry<float>(j, "cbed", "position", "x");
            auto y = readJsonEntry<float>(j, "cbed", "position", "y");
            auto pad = readJsonEntry<float>(j, "cbed", "position", "padding");

            CbedPosition ar(x, y, pad);
            auto area = man.getCBedPosition();
            *area = ar;
        } catch (json::out_of_range& e) {}

        try { man.setTdsRunsCbed(readJsonEntry<unsigned int>(j, "cbed", "tds configurations"));
        } catch (json::out_of_range& e) {}

        return man;
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

        auto mode = man.getMode();
        j["mode"]["id"] = mode;
        j["mode"]["name"] = man.getModeString();

        j["resolution"] = man.getResolution();

        j["slice thickness"]["val"] = man.getSliceThickness();
        j["slice thickness"]["units"] = "Angstrom";

        auto xl = man.getPaddedSimLimitsX();
        auto yl = man.getPaddedSimLimitsY();

        j["simulation area"]["x"]["start"] = xl[0];
        j["simulation area"]["x"]["finish"] = xl[1];
        j["simulation area"]["x"]["units"] = "Angstrom";
        j["simulation area"]["y"]["start"] = yl[0];
        j["simulation area"]["y"]["finish"] = yl[1];
        j["simulation area"]["y"]["units"] = "Angstrom";

        // TODO: could put some pixel scale things here for convenience? (would have to distinguish between diff maybe?)

        bool f3d = man.isFull3d();
        bool fd = man.isFiniteDifference();

        if(f3d || force_all) {
            j["full 3d"]["state"] = f3d;
            j["full 3d"]["integrals"] = man.getFull3dInts();
        } else
            j["full 3d"]["state"] = f3d;

//        j["finite difference"]["state"] = fd;

        j["microscope"]["voltage"]["val"] = man.getVoltage();
        j["microscope"]["voltage"]["units"] = "?";

        auto mp = man.getMicroscopeParams();

        // TODO: check which parameters are relevant to which modes
        // TODO: get whether CTEM is image, EW or diff...

        // apert
        j["microscope"]["aperture"]["val"] = mp->Aperture;
        j["microscope"]["aperture"]["units"] = "?";

        if (mode == SimulationMode::CTEM || force_all) {
            // alpha
            j["microscope"]["alpha"]["val"] = mp->Alpha;
            j["microscope"]["alpha"]["units"] = "?";

            // delta
            j["microscope"]["delta"]["val"] = mp->Delta;
            j["microscope"]["delta"]["units"] = "?";
        }

        // aberration values
        j["microscope"]["aberrations"]["C10"]["val"] = mp->C10;
        j["microscope"]["aberrations"]["C10"]["units"] = "?";
        j["microscope"]["aberrations"]["C12"]["mag"] = mp->C12.Mag;
        j["microscope"]["aberrations"]["C12"]["ang"] = mp->C12.Ang;
        j["microscope"]["aberrations"]["C12"]["units"] = "?";

        j["microscope"]["aberrations"]["C21"]["mag"] = mp->C21.Mag;
        j["microscope"]["aberrations"]["C21"]["ang"] = mp->C21.Ang;
        j["microscope"]["aberrations"]["C21"]["units"] = "?";
        j["microscope"]["aberrations"]["C23"]["mag"] = mp->C23.Mag;
        j["microscope"]["aberrations"]["C23"]["ang"] = mp->C23.Ang;
        j["microscope"]["aberrations"]["C23"]["units"] = "?";

        j["microscope"]["aberrations"]["C30"]["val"] = mp->C30;
        j["microscope"]["aberrations"]["C30"]["units"] = "?";
        j["microscope"]["aberrations"]["C32"]["mag"] = mp->C32.Mag;
        j["microscope"]["aberrations"]["C32"]["ang"] = mp->C32.Ang;
        j["microscope"]["aberrations"]["C32"]["units"] = "?";
        j["microscope"]["aberrations"]["C34"]["mag"] = mp->C34.Mag;
        j["microscope"]["aberrations"]["C34"]["ang"] = mp->C34.Ang;
        j["microscope"]["aberrations"]["C34"]["units"] = "?";

        j["microscope"]["aberrations"]["C41"]["mag"] = mp->C41.Mag;
        j["microscope"]["aberrations"]["C41"]["ang"] = mp->C41.Ang;
        j["microscope"]["aberrations"]["C41"]["units"] = "?";
        j["microscope"]["aberrations"]["C43"]["mag"] = mp->C43.Mag;
        j["microscope"]["aberrations"]["C43"]["ang"] = mp->C43.Ang;
        j["microscope"]["aberrations"]["C43"]["units"] = "?";
        j["microscope"]["aberrations"]["C45"]["mag"] = mp->C45.Mag;
        j["microscope"]["aberrations"]["C45"]["ang"] = mp->C45.Ang;
        j["microscope"]["aberrations"]["C45"]["units"] = "?";

        j["microscope"]["aberrations"]["C50"]["val"] = mp->C50;
        j["microscope"]["aberrations"]["C50"]["units"] = "?";
        j["microscope"]["aberrations"]["C52"]["mag"] = mp->C52.Mag;
        j["microscope"]["aberrations"]["C52"]["ang"] = mp->C52.Ang;
        j["microscope"]["aberrations"]["C52"]["units"] = "?";
        j["microscope"]["aberrations"]["C54"]["mag"] = mp->C54.Mag;
        j["microscope"]["aberrations"]["C54"]["ang"] = mp->C54.Ang;
        j["microscope"]["aberrations"]["C54"]["units"] = "?";
        j["microscope"]["aberrations"]["C56"]["mag"] = mp->C56.Mag;
        j["microscope"]["aberrations"]["C56"]["ang"] = mp->C56.Ang;
        j["microscope"]["aberrations"]["C56"]["units"] = "?";

        // If CTEM, get dose/CCD stuff
        if (mode == SimulationMode::CTEM || force_all)
        {
            // cropped padding entry will be added at the time of saving

            if (CCDParams::nameExists(man.getCcdName()) || force_all) {
                j["ctem"]["ccd"]["name"] = man.getCcdName();
                j["ctem"]["ccd"]["dose"]["val"] = man.getCcdDose();
                j["ctem"]["ccd"]["dose"]["units"] = "?";
                j["ctem"]["ccd"]["binning"] = man.getCcdBinning();
            } else {
                j["ctem"]["ccd"] = "Perfect";
            }

            j["ctem"]["area"]["x"]["start"] = man.getCtemArea().getLimitsX()[0];
            j["ctem"]["area"]["x"]["finish"] = man.getCtemArea().getLimitsX()[1];

            j["ctem"]["area"]["y"]["start"] = man.getCtemArea().getLimitsY()[0];
            j["ctem"]["area"]["y"]["finish"] = man.getCtemArea().getLimitsY()[1];
        }

        // If STEM, get scan info, TDS info detector info will be added later if needed
        if (mode == SimulationMode::STEM || force_all)
        {
            auto sa = man.getStemArea();
            j["stem"]["area"]["x"]["start"] = sa->getLimitsX()[0];
            j["stem"]["area"]["x"]["finish"] = sa->getLimitsX()[0];
            j["stem"]["scan"]["x"]["pixels"] = sa->getPixelsX();;
            j["stem"]["area"]["x"]["units"] = "Angstrom";
            j["stem"]["area"]["y"]["start"] = sa->getLimitsY()[0];
            j["stem"]["area"]["y"]["finish"] = sa->getLimitsY()[1];
            j["stem"]["scan"]["y"]["pixels"] = sa->getPixelsY();
            j["stem"]["area"]["y"]["units"] = "Angstrom";
            j["stem"]["area"]["padding"]["val"] = sa->getPadding();
            j["stem"]["area"]["padding"]["units"] = "Angstrom";

            // stem detector bit...

            j["stem"]["tds configurations"] = man.getTdsRunsStem();
            j["stem"]["concurrent pixels"] = man.getParallelPixels();
        }

        // If CBED, get position info and TDS info
        if (mode == SimulationMode::CBED || force_all)
        {
            j["cbed"]["position"]["x"] = man.getCBedPosition()->getXPos();
            j["cbed"]["position"]["y"] = man.getCBedPosition()->getYPos();
            j["cbed"]["position"]["padding"] = man.getCBedPosition()->getPadding();
            j["cbed"]["position"]["units"] = "Angstrom";
            j["cbed"]["tds configurations"] = man.getTdsRunsCbed();
        }

        return j;
    }

    json stemDetectorToJson(StemDetector d) {
        json j;
//        j["stem"]["detector"]["name"] = d.name;
        j["radius"]["inner"] = d.inner;
        j["radius"]["outer"] = d.outer;
        j["radius"]["units"] = "mrad";
        j["centre"]["x"] = d.xcentre;
        j["centre"]["y"] = d.ycentre;
        j["centre"]["units"] = "mrad";

        return j;
    }

}