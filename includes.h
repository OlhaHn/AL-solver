#ifndef INCLUDES_H
#define INCLUDES_H

#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <sstream>
#include <stack>
#include <boost/functional/hash.hpp>

using PairsSet = std::unordered_set<std::pair<int, int>, boost::hash< std::pair<int, int> > >;

#endif
