//
// Created by jon on 04/04/18.
//

#include "jsonutils.h"

namespace JSONUtils {

    json simManagerToJson(SimulationManager man) {

        json j;

        j["filename"] = man.getStructure()->getFileName();

        auto mode = man.getMode();
        j["mode"]["id"] = mode;
        j["mode"]["name"] = man.getModeString();

        j["resolution"] = man.getResolution();

        j["slice thickness"] = man.getSliceThickness();

        auto xl = man.getPaddedSimLimitsX();
        auto yl = man.getPaddedSimLimitsY();

        j["simulation area"]["x"]["start"] = xl[0];
        j["simulation area"]["x"]["finish"] = xl[1];
        j["simulation area"]["y"]["start"] = yl[0];
        j["simulation area"]["y"]["finish"] = yl[1];

        // TODO: could put some pixel scale things here for convenience? (would have to distinguish between diff maybe?)

        bool f3d = man.isFull3d();
        bool fd = man.isFiniteDifference();

        if(f3d) {
            j["full 3d"]["state"] = f3d;
            j["full 3d"]["num integrals"] = man.getFull3dInts();
        } else
            j["full 3d"] = f3d;

        if(fd) {
            j["finite difference"]["state"] = fd;
//            j["finite difference"]["num integrals"] = man.getFull3dInts();
        } else
            j["finite difference"] = fd;

        j["microscope"]["voltage"]["val"] = man.getVoltage();
        j["microscope"]["voltage"]["units"] = "?";

        auto mp = man.getMicroscopeParams();

        // TODO: check which parameters are relevant to which modes
        // TODO: get whether CTEM is image, EW or diff...

        // apert
        j["microscope"]["aperture"]["val"] = mp->Aperture;
        j["microscope"]["aperture"]["units"] = "?";

        if (mode == SimulationMode::CTEM) {
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
        if (mode == SimulationMode::CTEM)
        {
            // cropped padding entry will be added at the time of saving

            if (CCDParams::nameExists(man.getCcdName())) {
                j["ctem"]["ccd"]["name"] = man.getCcdName();
                j["ctem"]["ccd"]["dose"] = man.getCcdDose();
                j["ctem"]["ccd"]["binning"] = man.getCcdBinning();
            } else {
                j["ctem"]["ccd"] = "Perfect";
            }
        }

        // If STEM, get scan info, TDS info and detector info
        if (mode == SimulationMode::STEM)
        {
            auto sa = man.getStemArea();
            j["stem"]["scan"]["x"]["start"] = sa->getLimitsX()[0];
            j["stem"]["scan"]["x"]["finish"] = sa->getLimitsX()[0];
            j["stem"]["scan"]["x"]["pixels"] = sa->getPixelsX();;
            j["stem"]["scan"]["y"]["start"] = sa->getLimitsY()[0];
            j["stem"]["scan"]["y"]["finish"] = sa->getLimitsY()[1];
            j["stem"]["scan"]["y"]["pixels"] = sa->getPixelsY();

            // stem detector bit...

            j["stem"]["tds configurations"] = man.getTdsRuns();
            j["stem"]["concurrent pixels"] = man.getParallelPixels();
        }

        // If CBED, get position info and TDS info
        if (mode == SimulationMode::CBED)
        {
            j["cbed"]["position"]["x"] = man.getCBedPosition()->getXPos();
            j["cbed"]["position"]["y"] = man.getCBedPosition()->getYPos();
            j["cbed"]["tds configurations"] = man.getTdsRuns();
        }

        return j;
    }

    json stemDetectorToJson(StemDetector d) {
        json j;
        j["stem"]["detector"]["name"] = d.name;
        j["stem"]["detector"]["radius"]["innder"] = d.inner;
        j["stem"]["detector"]["radius"]["outer"] = d.outer;
        j["stem"]["detector"]["radius"]["units"] = "mrad";
        j["stem"]["detector"]["centre"]["x"] = d.xcentre;
        j["stem"]["detector"]["centre"]["y"] = d.ycentre;
        j["stem"]["detector"]["centre"]["units"] = "mrad";

        return j;
    }

}