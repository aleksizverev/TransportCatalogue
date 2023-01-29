#pragma once

#include "geo.h"

#include <string>
#include <vector>

struct Stop{
    std::string stopname;
    Coordinates coordinates;
};

struct Bus{
    std::string busname;
    std::vector<const Stop*> stops;
    bool is_roundtrip;

    bool operator<(const Bus& r) const{
        return std::lexicographical_compare(this->busname.begin(), this->busname.end(),
                                            r.busname.begin(), r.busname.end());
    };
};