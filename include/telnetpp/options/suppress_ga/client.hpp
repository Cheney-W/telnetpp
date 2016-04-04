#pragma once

#include "telnetpp/options/subnegotiationless_client.hpp"
#include "telnetpp/options/suppress_ga.hpp"

namespace telnetpp { namespace options { namespace suppress_ga {

using client = telnetpp::options::subnegotiationless_client<
    telnetpp::options::suppress_ga::option
>;

}}}
