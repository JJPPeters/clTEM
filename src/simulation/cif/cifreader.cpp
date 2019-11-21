//
// Created by Jon on 21/08/2015.
//

#include "cifreader.h"

#include <iostream>
namespace CIF {
    CIFReader::CIFReader(std::string filePath, bool attempt_fixes) {
        file_path = filePath;

        fix = attempt_fixes;

        // http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
        std::ifstream filestream(filePath);
        std::string filecontents;

        filestream.seekg(0, std::ios::end);
        filecontents.reserve(filestream.tellg());
        filestream.seekg(0, std::ios::beg);

        filecontents.assign((std::istreambuf_iterator<char>(filestream)), std::istreambuf_iterator<char>());

        filecontents = Utilities::stripCommentsWhitespace(filecontents);

        std::string errors;

        // read in the symmetry elements
        try {
            readSymmetryOperations(filecontents);
        } catch (std::runtime_error& e) {
            errors += e.what();
        }

        // read in atom positions
        try {
            readAtoms(filecontents);
        } catch (std::runtime_error& e) {
            errors += e.what();
        }

        // read in unit cell parameters
        try {
            readCellGeometry(filecontents);
        } catch (std::runtime_error& e) {
            errors += e.what();
        }

        if (!errors.empty())
            throw std::runtime_error(errors);
    }

    void CIFReader::readSymmetryOperations(const std::string &input) {
        symmetrylist = std::vector<Symmetry>();

        // first we want to know how  columns we have and where the things we want are
        std::regex rgxheaders("(?:loop_\\n)((?:_symmetry_equiv_pos_\\w+?\\n)+)");
        std::smatch match;

        std::string match_string = "";

        if (!std::regex_search(input, match, rgxheaders)) {
            if (fix) {
                // this simple fix assumes we have a primitive cell if the symmetry is not defined
                // could try and interpret the space group line, but that is a while other can of worms
                auto temp_symmetry = Symmetry();
                std::vector<std::string> temp_terms = {"x", "y", "z"};
                temp_symmetry.setOperation(0, temp_terms);
                symmetrylist = {temp_symmetry};
                return;
            } else
                throw std::runtime_error("Cannot find _symmetry_equiv_pos_ block\n");
        }

        std::vector<std::string> headerlines = Utilities::split(match[1], '\n');

        int xyzcol = Utilities::vectorSearch(headerlines, std::string("_symmetry_equiv_pos_as_xyz"));

        if (xyzcol >= headerlines.size())
            throw std::runtime_error("Could not find _symmetry_equiv_pos_as_xyz\n");

        std::string symmetrypattern = "";

        // this is a bit more tricky as all the columns don't have the same regex
        for (int i = 0; i < headerlines.size(); ++i) {
            if (i == xyzcol)
                symmetrypattern += "(?:['\"]?.+?,.+?,.+?['\"]?)";
            else
                symmetrypattern += "\\S+";
            if (i != headerlines.size() - 1)
                symmetrypattern += "[ \\t]+";
        }

        std::regex rgxsymmetry(symmetrypattern);

        std::istringstream op_ss(match.suffix().str());
        std::string symmetry_line;
        bool isValid = true;

        // Now read through the following lines, check they match the operator regex and process each one
        while (std::getline(op_ss, symmetry_line)) {
            isValid = std::regex_match(symmetry_line, rgxsymmetry);

            if (!isValid)
                break;

            std::regex rgxsymmetry_element("(['\"].+?['\"]|[\\S]+)");

            std::vector<std::string> column;

            while (std::regex_search(symmetry_line, match, rgxsymmetry_element)) {
                column.push_back(match[1].str());
                symmetry_line = match.suffix().str();
            }

            std::string symmetryxyz = column[xyzcol];

            // this first regex splits the lines into the individual operations
            std::regex rgxxyz("['\"]*\\s*([^\\s'\"]+)\\s*,\\s*([^\\s'\"]+)\\s*,\\s*([^\\s'\"]+)\\s*['\"]*");

            if (!std::regex_search(symmetryxyz, match, rgxxyz))
                throw std::runtime_error("Problem parsing _symmetry_equiv_pos_ line\n");

            auto matched = match;
            Symmetry ops;

            // loop through each operation ( we know there will be 3)
            for (int i = 1; i < 4; ++i) {
                // the operation string
                std::string op = matched[i];

                std::regex rgxoperation("([+-]?[^+-]+)");

                // TODO: maybe check that we have one match first so we can throw an error?
                // TODO: check we are reading in 3 operations for each line?

                // TODO: preallocate this vector

                // match all individual terms in the one operation
                // use a while loop now as we expect more than one match per string
                std::vector<std::string> termstrings;

                while (std::regex_search(op, match, rgxoperation)) {

                    for (int i = 1; i < match.size(); ++i)
                        termstrings.push_back(match[i].str());

                    // trim the string to make sure we don't find the same match
                    op = match.suffix().str();
                }

                if (termstrings.size() < 1)
                    throw std::runtime_error("Problem parsing symmetry operation.");

                if (termstrings.size() < 1)
                    throw std::runtime_error("Problem parsing symmetry operation.");

                ops.setOperation(i - 1, termstrings);
            }

            // append ops to vector to return
            symmetrylist.push_back(ops);
        }

        if (symmetrylist.size() < 1)
            throw std::runtime_error("Problem parsing _symmetry_equiv_pos_ block.");

    }

    void CIFReader::readAtoms(const std::string &input) {
        atomsites = std::vector<AtomSite>();

        // first we need to know where the positions we need are kept (column wise)
        // we do this through the header declaration just after the "loop_"
        std::regex rgxheaders(R"((?:\nloop_\n)((?:_atom_site_\w+?\n)+))");
        std::smatch match;

        auto start = input;
        bool found = false;

        // these are needed so we can parse this again for the aniso part
        std::vector<std::vector<std::string>> header_strings;
        std::vector<std::vector<std::string>> data_strings;

        while(std::regex_search(start, match, rgxheaders)) {
            found = true;
            // headers
            std::vector<std::string> headerlines = Utilities::split(match[1], '\n');
            header_strings.push_back(headerlines);

            std::string positionpattern = "((?:\\s*";
            for (int i = 0; i < headerlines.size() - 1; ++i)
                positionpattern += "\\S+[ \\t]+";
            // the last column needs to be different to close off the regex
            positionpattern += "\\S+[ \\t]*)+)";

            std::regex rgxpositions(positionpattern);
            std::istringstream at_ss(match.suffix().str());

            std::string atom_line;
            std::vector<std::string> atomlines;

            while (std::getline(at_ss, atom_line)) {
                if(!std::regex_match(atom_line, rgxpositions) || atom_line == "loop_") // could also test if line starts with _ ?
                    break;

                atomlines.push_back(atom_line);
            }

            data_strings.push_back(atomlines);

            start = match.suffix().str();
        }

        if (!found)
            throw std::runtime_error("Could not find _atom_site_ block(s)\n");

        // now process the atomc positions
        std::vector<std::string> errors;
        for (int i = 0; i < header_strings.size(); ++i) {
            try {
                readAtomPositions(header_strings[i], data_strings[i]);
                errors.emplace_back("");
            } catch (std::runtime_error& e) {
                errors.emplace_back(e.what());
            }
        }

        // we just need one block to not have errors for this to have 'passed'
        found = errors.empty();
        std::string error_out;
        for (auto &er : errors) {
            if (er.empty()) {
                found = true;
                break;
            } else {
                error_out += er;
            }
        }

        if (!found)
            throw std::runtime_error(error_out);

        // Now the displacement stuff
        for (int i = 0; i < header_strings.size(); ++i)
            readThermalParameters(header_strings[i], data_strings[i]);

        if (atomsites.empty())
            throw std::runtime_error("Could not parse atom information");
    }

    void CIFReader::readCellGeometry(const std::string &input) {
        double a = Utilities::regexFindDoubleTag(input, "_cell_length_a\\s+([\\d.]+)");
        double b = Utilities::regexFindDoubleTag(input, "_cell_length_b\\s+([\\d.]+)");
        double c = Utilities::regexFindDoubleTag(input, "_cell_length_c\\s+([\\d.]+)");

        double alpha = Utilities::regexFindDoubleTag(input, "_cell_angle_alpha\\s+([\\d.]+)");
        double beta = Utilities::regexFindDoubleTag(input, "_cell_angle_beta\\s+([\\d.]+)");
        double gamma = Utilities::regexFindDoubleTag(input, "_cell_angle_gamma\\s+([\\d.]+)");

        cell = CellGeometry(a, b, c, alpha, beta, gamma);
    }

    void CIFReader::readAtomPositions(const std::vector<std::string> &headers, const std::vector<std::string> &entries) {
        // these are required!
        int labelcol = Utilities::vectorSearch(headers, std::string("_atom_site_label"));
        bool foundlabel = labelcol != headers.size();
        int xcol = Utilities::vectorSearch(headers, std::string("_atom_site_fract_x"));
        bool foundx = xcol != headers.size();
        int ycol = Utilities::vectorSearch(headers, std::string("_atom_site_fract_y"));
        bool foundy = ycol != headers.size();
        int zcol = Utilities::vectorSearch(headers, std::string("_atom_site_fract_z"));
        bool foundz = zcol != headers.size();
        int symbolcol = Utilities::vectorSearch(headers, std::string("_atom_site_type_symbol"));
        bool foundsymbol = symbolcol != headers.size();

        // this is not absolutely needed (assume occupancy = 1 otherwise
        int occupancycol = Utilities::vectorSearch(headers, std::string("_atom_site_occupancy"));
        bool foundoccupancy = occupancycol != headers.size();

        // this fix will try to get a atom symbol from the atom label
        // they are often just a type symbol with a number
        if (fix && !foundsymbol && foundlabel) {
            symbolcol = labelcol;
            foundsymbol = true;
        }

        std::string errors = "";

        if(!foundlabel)
            errors += "Could not find _atom_site_label\n";
        if(!foundsymbol)
            errors += "Could not find _atom_site_type_symbol\n";
        if(!foundx)
            errors += "Could not find _atom_site_fract_x\n";
        if(!foundy)
            errors += "Could not find _atom_site_fract_y\n";
        if(!foundz)
            errors += "Could not find _atom_site_fract_z\n";

        if (!errors.empty())
            throw std::runtime_error(errors);

        std::smatch match;
        std::regex rgxcolumns("([^\\s]+)");

        for (auto &row : entries) {
            std::vector<std::string> columns;
            std::string line = row;

            // split our line by columns
            while (std::regex_search(line, match, rgxcolumns)) {
                // extract column into list of vectors
                for (int j = 1; j < match.size(); ++j)
                    columns.push_back(match[j].str());

                line = match.suffix().str();
            }

            // get the label (This can be anything, it is not the atom type)
            // this is the only simple one...
            std::string label = columns[labelcol];

            // type symbol can have valence etc attached to it
            // There is probably a safe, non-regex way to do this
            std::string symbol = "";
            std::regex rgxname("([a-zA-Z]{1,2})");
            if (std::regex_search(columns[symbolcol], match, rgxname))
                symbol = match[1];
            else
                errors += "Could not find acceptable format _atom_site_type_symbol (need 1-2 letters) in: " + columns[symbolcol] + "\n";

            if (fix) {
                // this fix deals with the case of the symbol string
                symbol[0] = std::toupper(symbol[0]);
                if (symbol.length() == 2)
                    symbol[1] = std::tolower(symbol[1]);
            }

            if (!CIF::Utilities::isAcceptedAtom(symbol))
                errors += "Could not detect valid atom from: " + columns[symbolcol] + " (sub string: " + symbol + ")\n";

            // get the x, y, z positions of our atom site
            // the split here is to get rid of uncertainties in brackets
            double x, y, z;
            try {
                x = Utilities::stod(Utilities::split(columns[xcol], '(')[0]);
            } catch (std::invalid_argument& e) {
                errors += "Could not get number from position values (not a number): " + columns[xcol] + "\n";
            } catch (std::out_of_range& e) {
                errors += "Could not get number from position values (value too large): " + columns[xcol] + "\n";
            }

            try {
                y = Utilities::stod(Utilities::split(columns[ycol], '(')[0]);
            } catch (std::invalid_argument& e) {
                errors += "Could not get number from position values (not a number): " + columns[ycol] + "\n";
            } catch (std::out_of_range& e) {
                errors += "Could not get number from position values (value too large): " + columns[ycol] + "\n";
            }

            try {
                z = Utilities::stod(Utilities::split(columns[zcol], '(')[0]);
            } catch (std::invalid_argument& e) {
                errors += "Could not get number from position values (not a number): " + columns[zcol] + "\n";
            } catch (std::out_of_range& e) {
                errors += "Could not get number from position values (value too large): " + columns[zcol] + "\n";
            }


            // default to 1.0
            double occupancy = 1.0;
            if (foundoccupancy)
                try {
                    occupancy = Utilities::stod(Utilities::split(columns[occupancycol], '(')[0]);
                } catch (std::invalid_argument& e) {
                    errors += "Could not get number from occupancy value (not a number): " + columns[occupancycol] + "\n";
                } catch (std::out_of_range& e) {
                    errors += "Could not get number from occupancy (value too large): " + columns[occupancycol] + "\n";
                }

            // if we have errrors, now is when we quit!
            if (!errors.empty())
                continue;

            // here we are checking if the atom is on the same site
            std::vector<double> postemp({x, y, z});
            bool isNew = true;

            for (auto &site : atomsites) {
                auto positions = site.getPositions();
                int ind = Utilities::vectorSearch(positions, postemp);
                if (ind >= positions.size())
                    continue;
                else {
                    site.addAtom(symbol, label, occupancy);
                    isNew = false;
                    break;
                }
            }


            if (isNew)
                atomsites.emplace_back(symmetrylist, symbol, label, x, y, z, occupancy);
        }

        if (!errors.empty())
            throw std::runtime_error(errors);
    }

    void CIFReader::readThermalParameters(const std::vector<std::string> &headers, const std::vector<std::string> &entries){

        // find label, normal one takes precedence
        int labelcol = Utilities::vectorSearch(headers, std::string("_atom_site_label"));
        bool foundlabel = labelcol < headers.size();

        // then look for our aniso label
        if (!foundlabel) {
            labelcol = Utilities::vectorSearch(headers, std::string("_atom_site_aniso_label"));
            foundlabel = labelcol < headers.size();
        }

        if (!foundlabel)
            return;

        // could be u iso?
        int isocol = Utilities::vectorSearch(headers, std::string("_atom_site_B_iso_or_equiv"));
        bool foundiso = isocol < headers.size();

        // could be B?
        int uxcol = Utilities::vectorSearch(headers, std::string("_atom_site_aniso_U_11"));
        bool foundux = uxcol < headers.size();
        int uycol = Utilities::vectorSearch(headers, std::string("_atom_site_aniso_U_22"));
        bool founduy = uycol < headers.size();
        int uzcol = Utilities::vectorSearch(headers, std::string("_atom_site_aniso_U_33"));
        bool founduz = uzcol < headers.size();

        std::smatch match;
        std::regex rgxcolumns("([^\\s]+)");

        for (auto &row : entries) {
            std::vector<std::string> columns;
            std::string line = row;

            // split our line by columns
            while (std::regex_search(line, match, rgxcolumns)) {
                // extract column into list of vectors
                for (int j = 1; j < match.size(); ++j)
                    columns.push_back(match[j].str());

                line = match.suffix().str();
            }

            std::string lbl = columns[labelcol];

            double u_iso = 0.0;
            if (foundiso)
                u_iso = Utilities::stod(Utilities::split(columns[isocol], '(')[0]);

            double u_x = 0.0;
            if(foundux)
                u_x = Utilities::stod(Utilities::split(columns[uxcol], '(')[0]);

            double u_y = 0.0;
            if(foundux)
                u_y = Utilities::stod(Utilities::split(columns[uycol], '(')[0]);

            double u_z = 0.0;
            if(foundux)
                u_z = Utilities::stod(Utilities::split(columns[uzcol], '(')[0]);

            for(auto &a : atomsites) {
                if (foundiso)
                    a.setIsoU(lbl, u_iso);

                if (foundux)
                    a.setU(lbl, u_x, 0);

                if (founduy)
                    a.setU(lbl, u_y, 1);

                if (founduz)
                    a.setU(lbl, u_z, 2);
            }
        }
    }
}