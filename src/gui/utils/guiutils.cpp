//
// Created by jon on 21/04/18.
//

#include "guiutils.h"

namespace GuiUtils
{

    const unsigned int edit_precision = 5;

    std::string ElementNumberToColour(int A) {
        auto got = MapNumberToColour.find(A);

        if ( got == MapNumberToColour.end() )
            throw std::runtime_error("Unidentified atomic number: " + std::to_string(A));

        return got->second;
    }

    QColor ElementNumberToQColour(int A) {
        std::string hx = ElementNumberToColour(A);

        // convert hex string to RGB
        // NOTE: no # or 0x, otherwise errors!

        // https://stackoverflow.com/questions/10156409/convert-hex-string-char-to-int
        long hex_col = std::strtol(hx.c_str(), nullptr, 16);

        int r = (hex_col >> 16) & 0xFF;
        int g = (hex_col >> 8) & 0xFF;
        int b = hex_col & 0xFF;

        return QColor(r, g, b);
    }

    int ConstructElementColourMap() {

        for(auto& el : VectorNumberToColour) {
            MapNumberToColour.insert(el);
        }
        return 0;
    }


    std::unordered_map<int, std::string> MapNumberToColour;

    // I have used to JMOL colour scheme
    // http://jmol.sourceforge.net/jscolors/
    const std::vector<std::pair<int, std::string>> VectorNumberToColour = {
            {1, "FFFFFF"},
            {2, "D9FFFF"},
            {3, "CC80FF"},
            {4, "C2FF00"},
            {5, "FFB5B5"},
            {6, "909090"},
            {7, "3050F8"},
            {8, "FF0D0D"},
            {9, "90E050"},
            {10, "B3E3F5"},
            {11, "AB5CF2"},
            {12, "8AFF00"},
            {13, "BFA6A6"},
            {14, "F0C8A0"},
            {15, "FF8000"},
            {16, "FFFF30"},
            {17, "1FF01F"},
            {18, "80D1E3"},
            {19, "8F40D4"},
            {20, "3DFF00"},
            {21, "E6E6E6"},
            {22, "BFC2C7"},
            {23, "A6A6AB"},
            {24, "8A99C7"},
            {25, "9C7AC7"},
            {26, "E06633"},
            {27, "F090A0"},
            {28, "50D050"},
            {29, "C88033"},
            {30, "7D80B0"},
            {31, "C28F8F"},
            {32, "668F8F"},
            {33, "BD80E3"},
            {34, "FFA100"},
            {35, "A62929"},
            {36, "5CB8D1"},
            {37, "702EB0"},
            {38, "00FF00"},
            {39, "94FFFF"},
            {40, "94E0E0"},
            {41, "73C2C9"},
            {42, "54B5B5"},
            {43, "3B9E9E"},
            {44, "248F8F"},
            {45, "0A7D8C"},
            {46, "006985"},
            {47, "C0C0C0"},
            {48, "FFD98F"},
            {49, "A67573"},
            {50, "668080"},
            {51, "9E63B5"},
            {52, "D47A00"},
            {53, "940094"},
            {54, "429EB0"},
            {55, "57178F"},
            {56, "00C900"},
            {57, "70D4FF"},
            {58, "FFFFC7"},
            {59, "D9FFC7"},
            {60, "C7FFC7"},
            {61, "A3FFC7"},
            {62, "8FFFC7"},
            {63, "61FFC7"},
            {64, "45FFC7"},
            {65, "30FFC7"},
            {66, "1FFFC7"},
            {67, "00FF9C"},
            {68, "00E675"},
            {69, "00D452"},
            {70, "00BF38"},
            {71, "00AB24"},
            {72, "4DC2FF"},
            {73, "4DA6FF"},
            {74, "2194D6"},
            {75, "267DAB"},
            {76, "266696"},
            {77, "175487"},
            {78, "D0D0E0"},
            {79, "FFD123"},
            {80, "B8B8D0"},
            {81, "A6544D"},
            {82, "575961"},
            {83, "9E4FB5"},
            {84, "AB5C00"},
            {85, "754F45"},
            {86, "428296"},
            {87, "420066"},
            {88, "007D00"},
            {89, "70ABFA"},
            {90, "00BAFF"},
            {91, "00A1FF"},
            {92, "008FFF"},
            {93, "0080FF"},
            {94, "006BFF"},
            {95, "545CF2"},
            {96, "785CE3"},
            {97, "8A4FE3"},
            {98, "A136D4"},
            {99, "B31FD4"},
            {100, "B31FBA"},
            {101, "B30DA6"},
            {102, "BD0D87"},
            {103, "C70066"}
    };

    int dummy_construct_map = ConstructElementColourMap();
}