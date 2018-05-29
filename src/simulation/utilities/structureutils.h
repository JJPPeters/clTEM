#ifndef STRUCTUREUTILS_H
#define STRUCTUREUTILS_H

#include <string>
#include <unordered_map>
#include <vector>

namespace Utils {

extern const std::vector<std::pair<std::string, int>> VectorSymbolToNumber;

extern std::unordered_map<std::string, int> MapSymbolToNumber;

// Getter for the map. Replicated convenience of [] that can't be used as the map
// is const (and [] will add elements if the key is not present)
int ElementSymbolToNumber(std::string sym);

int ConstructElementMap();

extern int dummy_construct_map;

}

#endif // STRUCTUREUTILS_H
