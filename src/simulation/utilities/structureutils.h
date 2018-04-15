#ifndef STRUCTUREUTILS_H
#define STRUCTUREUTILS_H

#include <string>
#include <unordered_map>

namespace Utils {

extern const std::unordered_map<std::string, int> MapSymbolToNumber;

// Getter for the map. Replicated convenience of [] that can't be used as the map
// is const (and [] will add elements if the key is not present)
int ElementSymbolToNumber(std::string sym);

}

#endif // STRUCTUREUTILS_H
