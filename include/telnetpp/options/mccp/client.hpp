#pragma once

#include "telnetpp/client_option.hpp"

namespace telnetpp { namespace options { namespace mccp {

class codec;

//* =========================================================================
/// \brief A server option responsible for negotiating the client part of the
/// MCCP protocol.
//* =========================================================================
class TELNETPP_EXPORT client : public telnetpp::client_option
{
public:
    //* =====================================================================
    /// \brief Constructor
    //* =====================================================================
    explicit client(codec &cdc);

private:
    //* =====================================================================
    /// \brief Called when a subnegotiation is received while the option is
    /// active.  Override for option-specific functionality.
    //* =====================================================================
    void handle_subnegotiation(
        telnetpp::bytes data,
        continuation const &cont) override;
        
    codec &codec_;
};

}}}
