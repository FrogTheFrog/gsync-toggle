#pragma once

// system includes

// local includes
#include "nvapiwrapper.h"

//--------------------------------------------------------------------------------------------------

class NvApiDrsSession final
{
public:
    explicit NvApiDrsSession();
    ~NvApiDrsSession();

    operator NvDRSSessionHandle();

private:
    NvApiWrapper       m_nvapi;
    NvDRSSessionHandle m_handle{nullptr};
};
