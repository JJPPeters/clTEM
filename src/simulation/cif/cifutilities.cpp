
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

        double stod(std::string& s) {
            if (s == ".")
                return 0.0;

            return std::stod(s);
        }

        double regexFindDoubleTag(std::string input, std::string pattern) {
            std::regex rgx(pattern);
            std::smatch match;

            if (!std::regex_search(input, match, rgx))
                return 0.0; // TODO: throw error

            return std::stod(std::string(match[1].str()));
        }

        std::string stripComments(const std::string& input) {

            std::string output = "";
            std::istringstream iss(input);
            for (std::string line; std::getline(iss, line); ) {
                size_t comm = line.rfind('#');
                std::string temp = line.substr(0, comm);
                temp.erase(temp.find_last_not_of(R"( \n\r\t)")+1); // I don't really care about whitespace, but I don't want blank lines (i.e. only whitespace)

                if (!temp.empty())
                    output += line.substr(0, comm) + "\n";
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