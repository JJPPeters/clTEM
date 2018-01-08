#include <stdexcept>
//#include <QtCore/QFile>
//#include <QtCore/QTextStream>
#include "stringutils.h"

namespace Utils
{

    // Taken from http://stackoverflow.com/a/6089413
    std::istream& safeGetline(std::istream& is, std::string& t)
    {
        t.clear();

        // The characters in the stream are read one-by-one using a std::streambuf.
        // That is faster than reading them one-by-one using the std::istream.
        // Code that uses streambuf this way must be guarded by a sentry object.
        // The sentry object performs various tasks,
        // such as thread synchronization and updating the stream state.

        std::istream::sentry se(is, true);
        std::streambuf* sb = is.rdbuf();

        for(;;) {
            int c = sb->sbumpc();
            switch (c) {
            case '\n':
                return is;
            case '\r':
                if(sb->sgetc() == '\n')
                    sb->sbumpc();
                return is;
            case EOF:
                // Also handle the case when the last line has no line ending
                if(t.empty())
                    is.setstate(std::ios::eofbit);
                return is;
            default:
                t += (char)c;
            }
        }
    }

//    const char* resourceToChar(std::string resourcePath)
//    {
//        std::ifstream inStream("./kernels/" + resourcePath);
//
//        if (inStream.fail())
//            throw std::runtime_error("Error opening resource file: " + resourcePath);
//
//        std::string fileContents((std::istreambuf_iterator<char>(inStream)), (std::istreambuf_iterator<char>()));
//
//        inStream.close();
//
//        return fileContents.c_str();
//
////        // open qresource file
////        QFile resourceFile(QString::fromStdString(resourcePath));
////
////        if (!resourceFile.open(QIODevice::ReadOnly | QIODevice::Text))
////            throw std::runtime_error("Error opening parameters file.");
////
////        QTextStream inStream(&resourceFile);
////
////        std::string fileContents = inStream.readAll().toStdString();
////
////        return fileContents.c_str();
//    }

}
