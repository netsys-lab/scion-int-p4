#pragma once

#include "common.h"
#include "connection.h"
#include "controller.h"


/// \brief A simple controller for L2 MAC learning without aging or support for moving a MAC from
/// one port to another.
class MacLearningCtrl : public Controller
{
public:
    MacLearningCtrl(SwitchConnection& con, const p4::config::v1::P4Info &p4Info);

private:
    /// \name Initialization Functions
    ///@{
    bool createFloodMulticastGroup(SwitchConnection &con);
    bool installStaticTableEntries(SwitchConnection &con);
    bool configDigestMessages(SwitchConnection &con);
    ///@}

    /// \name Stream Message Handlers
    ///@{
    void handleArbitrationUpdate(
        SwitchConnection &con, const p4::v1::MasterArbitrationUpdate& arbUpdate) override;
    bool handleDigest(SwitchConnection &con, const p4::v1::DigestList& digestList) override;
    ///@}

private:
    uint32_t macLearnDigestId;
};
