//
// Created by Jon on 13/02/2020.
//

#ifndef CLTEM_RANGEVALIDATOR_H
#define CLTEM_RANGEVALIDATOR_H


#include <QValidator>

// https://stackoverflow.com/questions/7430876/qvalidator-in-qt
class RangeValidator : public QValidator
{
    Q_OBJECT
public:
//    explicit RangeValidator(QObject *parent = 0);

    virtual State validate ( QString & input, int & pos ) const
    {
        if (input.isEmpty())
            return Acceptable;

        // first test our input contains only valid characters
        std::string input_string = input.toStdString();
        if (input_string.find_first_not_of("0123456789,-") != std::string::npos)
            return Invalid;

        // now test for simple things
        // double comma
        if(input_string.find(",,") != std::string::npos)
            return Invalid;

        // double hyphen
        if(input_string.find("--") != std::string::npos)
            return Invalid;



        return Acceptable;
    }

};


#endif //CLTEM_RANGEVALIDATOR_H
