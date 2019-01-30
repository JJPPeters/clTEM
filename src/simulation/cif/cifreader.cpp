//
// Created by Jon on 21/08/2015.
//

#include "cifreader.h"

#include <iostream>
namespace CIF {
    CIFReader::CIFReader(std::string filePath) {
        // http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
        std::ifstream filestream(filePath);
        std::string filecontents;

        filestream.seekg(0, std::ios::end);
        filecontents.reserve(filestream.tellg());
        filestream.seekg(0, std::ios::beg);

        filecontents.assign((std::istreambuf_iterator<char>(filestream)), std::istreambuf_iterator<char>());

        // read in the symmetry elements
        readSymmetryOperations(filecontents);

        // read in atom positions
        readAtomPositions(filecontents);

        // read in unit cell parameters
        readCellGeometry(filecontents);
    }

    void CIFReader::readSymmetryOperations(const std::string &input) {
        symmetrylist = std::vector<Symmetry>();

        // first we want to know how  columns we have and where the things we want are
        std::regex rgxheaders("(?:loop_\\n)((?:_symmetry_equiv_pos_\\w+?\\n)+)");
        std::smatch match;

        if (!std::regex_search(input, match, rgxheaders))
            throw std::runtime_error("Cannot find _symmetry_equiv_pos_ block.");

        std::vector<std::string> headerlines = Utilities::split(match[1], '\n');

        int xyzcol = Utilities::vectorSearch(headerlines, std::string("_symmetry_equiv_pos_as_xyz"));

        if (xyzcol >= headerlines.size())
            throw std::runtime_error("Could not find _symmetry_equiv_pos_as_xyz.");

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
                throw std::runtime_error("Problem parsing _symmetry_equiv_pos_ line.");

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

    void CIFReader::readAtomPositions(const std::string &input) {
        atomsites = std::vector<AtomSite>();

        // first we need to know where the positions we need are kept (column wise)
        // we do this through the header declaration just after the "loop_"
        std::regex rgxheaders("(?:loop_\\n)((?:_atom_site_\\w+?\\n)+)");
        std::smatch match;

        if (!std::regex_search(input, match, rgxheaders))
            throw std::runtime_error("Cannot find _atom_site_ block.");

        std::vector<std::string> headerlines = Utilities::split(match[1], '\n');

        // now we can find the values we want and this will be the columns numbers
        int xcol = Utilities::vectorSearch(headerlines, std::string("_atom_site_fract_x"));
        int ycol = Utilities::vectorSearch(headerlines, std::string("_atom_site_fract_y"));
        int zcol = Utilities::vectorSearch(headerlines, std::string("_atom_site_fract_z"));
        int symbolcol = Utilities::vectorSearch(headerlines, std::string("_atom_site_type_symbol"));
        // this is not absolutely needed (assume occupancy = 1 otherwise
        int occupancycol = Utilities::vectorSearch(headerlines, std::string("_atom_site_occupancy"));

        if (xcol >= headerlines.size())
            throw std::runtime_error("Could not find _atom_site_fract_x.");

        if (ycol >= headerlines.size())
            throw std::runtime_error("Could not find _atom_site_fract_y.");

        if (zcol >= headerlines.size())
            throw std::runtime_error("Could not find _atom_site_fract_z.");

        if (symbolcol >= headerlines.size())
            throw std::runtime_error("Could not find _atom_site_type_symbol.");


        //TODO: change this to read in line after headers, it can easily stack overflow for large number of atoms (not likely, but possible)

        // construct the regex dynamically to process each line (we don't know the number of columns beforehand)
        std::string positionpattern = "((?:\\s*";
        for (int i = 0; i < headerlines.size() - 1; ++i)
            positionpattern += "\\S+[ \\t]+";
        // the last column needs to be different to close off the regex
        positionpattern += "\\S+[ \\t]*)+)";

        std::regex rgxpositions(positionpattern);

        std::istringstream at_ss(match.suffix().str());
        std::string atom_line;
        bool isValid = true;

        // Now read through the following lines, check they match the operator regex and process each one
        while (std::getline(at_ss, atom_line)) {
            isValid = std::regex_match(atom_line, rgxpositions); //TODO: the regex for this is wrong...

            if (!isValid)
                break;

            // split each line by whitespace (use a regex for this?)
            std::regex rgxcolumns("([^\\s]+)");
            std::vector<std::string> columns;

            //TODO: is there a better way to separate this without regex?
            while (std::regex_search(atom_line, match, rgxcolumns)) {
                // extract column into list of vectors
                for (int j = 1; j < match.size(); ++j)
                    columns.push_back(match[j].str());

                atom_line = match.suffix().str();
            }

            // using the column indices from before, extract the required values
            // will need to remove brackets with errors in
            double x = std::stod(Utilities::split(columns[xcol], '(')[0]);
            double y = std::stod(Utilities::split(columns[ycol], '(')[0]);
            double z = std::stod(Utilities::split(columns[zcol], '(')[0]);
            // name has to be trimmed to remove oxidation state etc..
            std::string symbol = "";
            std::regex rgxname("([a-zA-Z]{1,2})");
            if (std::regex_search(columns[symbolcol], match, rgxname))
                symbol = match[1];
            // TODO: add handling for when occupancy column does not exist
            double occupancy = std::stod(Utilities::split(columns[occupancycol], '(')[0]);

            // here we are checking if the atom is on the same site
            std::vector<double> postemp({x, y, z});
            bool isNew = true;

            for (auto &site : atomsites) {
                auto positions = site.getPositions();
                int ind = Utilities::vectorSearch(positions, postemp);
                if (ind >= positions.size())
                    continue;
                else {
                    site.addAtom(symbol, occupancy);
                    isNew = false;
                    //TODO: think I can break the loop here
                }
            }

            if (isNew)
                atomsites.push_back(AtomSite(symmetrylist, symbol, x, y, z, occupancy));
        }
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
}