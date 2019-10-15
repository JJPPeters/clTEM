
#include "cifutilities.h"

namespace CIF::Utilities {

        bool isAcceptedAtom(const std::string& symbol) {
            return !(std::find(AcceptedAtoms.begin(), AcceptedAtoms.end(), symbol) == AcceptedAtoms.end());
        }

        std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
            std::stringstream ss(s);
            std::string item;
            while (std::getline(ss, item, delim)) {
                elems.push_back(item);
            }
            return elems;
        }

        std::vector<std::string> split(const std::string &s, char delim) {
            std::vector<std::string> elems;
            split(s, delim, elems);
            return elems;
        }

        std::vector<std::string> split(const std::string &s, std::string delims) {
            size_t beg, pos = 0;
            std::vector<std::string> elems;
            while ((beg = s.find_first_not_of(delims, pos)) != std::string::npos)
            {
                pos = s.find_first_of(delims, beg + 1);
                elems.push_back(s.substr(beg, pos - beg));
            }
        }

        double stod(std::string& s) {
            if (s == ".")
                return 0.0;

            return std::stod(s);
        }

        bool string_ends_with(std::string fullString, std::string ending) {
            if (fullString.length() >= ending.length()) {
                return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
            } else {
                return false;
            }
        }

        double regexFindDoubleTag(std::string input, std::string pattern) {
            std::regex rgx(pattern);
            std::smatch match;

            if (!std::regex_search(input, match, rgx))
                return 0.0; // TODO: throw error

            return std::stod(std::string(match[1].str()));
        }

        std::string stripCommentsWhitespace(const std::string& input) {

            std::string output = "";
            std::istringstream iss(input);
            std::string whitespace = R"( \n\r\t)";

            for (std::string line; std::getline(iss, line); ) {
                // start by removing whitespace
                const auto start = line.find_first_not_of(whitespace);

                if (start == std::string::npos)
                    continue; // nothing to do on this line

                const auto range = start - line.find_last_not_of(R"( \n\r\t)") + 1;
                std::string temp = line.substr(start, range);

                // now look for comments
                size_t comm = temp.rfind('#');
                temp = temp.substr(0, comm);

                if (!temp.empty())
                    output += temp + "\n";
            }

            return output;
        }
    }


const std::vector<std::string> CIF::Utilities::AcceptedAtoms = {"H",
"He",
"Li",
"Be",
"B",
"C",
"N",
"O",
"F",
"Ne",
"Na",
"Mg",
"Al",
"Si",
"P",
"S",
"Cl",
"Ar",
"K",
"Ca",
"Sc",
"Ti",
"V",
"Cr",
"Mn",
"Fe",
"Co",
"Ni",
"Cu",
"Zn",
"Ga",
"Ge",
"As",
"Se",
"Br",
"Kr",
"Rb",
"Sr",
"Y",
"Zr",
"Nb",
"Mo",
"Tc",
"Ru",
"Rh",
"Pd",
"Ag",
"Cd",
"In",
"Sn",
"Sb",
"Te",
"I",
"Xe",
"Cs",
"Ba",
"La",
"Ce",
"Pr",
"Nd",
"Pm",
"Sm",
"Eu",
"Gd",
"Tb",
"Dy",
"Ho",
"Er",
"Tm",
"Yb",
"Lu",
"Hf",
"Ta",
"W",
"Re",
"Os",
"Ir",
"Pt",
"Au",
"Hg",
"Tl",
"Pb",
"Bi",
"Po",
"At",
"Rn",
"Fr",
"Ra",
"Ac",
"Th",
"Pa",
"U",
"Np",
"Pu",
"Am",
"Cm",
"Bk",
"Cf",
"Es",
"Fm",
"Md",
"No",
"Lr"};