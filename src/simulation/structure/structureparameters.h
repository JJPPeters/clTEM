#include <utility>

#include <utility>

#ifndef CLTEM_STRUCTUREPARAMETERS_H
#define CLTEM_STRUCTUREPARAMETERS_H

#include <vector>
#include <mutex>
#include <algorithm>

// This class is purely static, and is mainly used for reading in the required data and storing it.
// When it is actually needed, it is copied to the SimulationManager classes so they have their own copy
// and this can be edited whilst a simulation is running...

enum ParameterisationForm {
    Kirkland = 0,
    Peng = 1,
    Lobato = 2
};

struct Parameterisation {
//    Parameterisation() : Name(""), Parameters(), Max_Atomic_Number(0) {}
    Parameterisation(std::string _file, std::string _name, const std::string& _form, unsigned int _i_per, std::vector<double> _params) {
        file_name = std::move(_file);
        name = std::move(_name);
        form = stringToParamForm(_form);
        i_per_atom = _i_per;

        unsigned int params_per_atom = i_per_atom * paramsPerI(form);
        if (_params.size() % params_per_atom != 0) {
            throw std::runtime_error("Parameterisation " + _name + " does not contain correct parameters per atom");
        }

        max_atomic_number = (unsigned int) (_params.size() / params_per_atom);
        parameters = std::move(_params);
    }

    // this is mostly for debugging
    std::string file_name;

    std::string name;

    ParameterisationForm form;

    unsigned int i_per_atom;

    std::vector<double> parameters;

    unsigned int max_atomic_number;

    bool operator==(const std::string &_name) const {
        return _name == name;
    }

private:
    [[nodiscard]] ParameterisationForm stringToParamForm(const std::string& form_name) const {
        if (form_name == "kirkland")
            return ParameterisationForm::Kirkland;
        else if (form_name == "peng")
            return ParameterisationForm::Peng;
        else if (form_name == "lobato")
            return ParameterisationForm::Lobato;

        throw std::runtime_error("Unknown parameterisation form " + form_name + " in file " + file_name);
    }

    unsigned int paramsPerI(ParameterisationForm& f) {
        if (f == ParameterisationForm::Kirkland)
            return 4;
        else if (f == ParameterisationForm::Peng)
            return 2;
        else if (f == ParameterisationForm::Lobato)
            return 2;

        throw std::runtime_error("Unknown parameterisation form " + std::to_string(f));
    }
};

class StructureParameters
{
private:
    static std::vector<Parameterisation> params;

    static std::mutex mtx;

public:
    static void setParams(const std::string &file_path, const std::string &name, const std::string &form, const unsigned int& per, const std::vector<double> &p) {
        std::lock_guard<std::mutex> lck(mtx);
        // test if the name already exists
        if (std::find(params.begin(), params.end(), name) != params.end())
            throw std::runtime_error("Error adding duplicate parameter name");

        params.emplace_back(file_path, name, form, per, p);
    }

    static Parameterisation getParameters(const std::string &name) {
        std::lock_guard<std::mutex> lck(mtx);

        auto ind = std::find(params.begin(), params.end(), name);
        if (ind == params.end())
            throw std::runtime_error("Could not find loaded parameters with name " + name);

        int Current = (int) (ind - params.begin());

        return params[Current];
    }

    static std::vector<std::string> getNames() {
        std::lock_guard<std::mutex> lck(mtx);
        std::vector<std::string> names;

        for (const auto &p : params)
            names.push_back(p.name);
        return names;
    }

    static std::vector<double> getParametersData(const std::string &name) {
        // I could cal the getParameter function, but I will try to lock the mutex twice...
        std::lock_guard<std::mutex> lck(mtx);

        auto ind = std::find(params.begin(), params.end(), name);
        if (ind == params.end())
            throw std::runtime_error("Could not find parameterisation with name: " + name);

        int Current = (int) (ind - params.begin());

        return params[Current].parameters;
    }
};

#endif //CLTEM_STRUCTUREPARAMETERS_H
