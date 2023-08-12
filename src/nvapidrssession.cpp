// class header include
#include "nvapidrssession.h"

// system includes
#include <iostream>
#include <stdexcept>

//--------------------------------------------------------------------------------------------------

NvApiDrsSession::NvApiDrsSession()
{
    auto status{m_nvapi.DRS_CreateSession(&m_handle)};
    if (status != NVAPI_OK)
    {
        const std::string error{"Failed to create session. Error: " + m_nvapi.getErrorMessage(status)};
        throw std::runtime_error(error);
    }
}

//--------------------------------------------------------------------------------------------------

NvApiDrsSession::~NvApiDrsSession()
{
    if (m_handle)
    {
        m_nvapi.DRS_DestroySession(m_handle);
    }
}

//--------------------------------------------------------------------------------------------------

NvApiDrsSession::operator NvDRSSessionHandle()
{
    return m_handle;
}
