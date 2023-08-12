#pragma once

// system includes
#include <string>

// local includes
// clang-format off
#include "nvapi-open-source-sdk/nvapi.h"
#include "nvapi-open-source-sdk/NvApiDriverSettings.h"
// clang-format on

//--------------------------------------------------------------------------------------------------

class NvApiWrapper final
{
public:
    explicit NvApiWrapper();
    ~NvApiWrapper();

    std::string getErrorMessage(NvAPI_Status status);

    NvAPI_Status DRS_CreateSession(NvDRSSessionHandle* handle);
    NvAPI_Status DRS_DestroySession(NvDRSSessionHandle handle);

    NvAPI_Status DRS_LoadSettings(NvDRSSessionHandle handle);
    NvAPI_Status DRS_SaveSettings(NvDRSSessionHandle handle);

    NvAPI_Status DRS_GetBaseProfile(NvDRSSessionHandle session_handle, NvDRSProfileHandle* profile_handle);
    NvAPI_Status DRS_SetSetting(NvDRSSessionHandle session_handle, NvDRSProfileHandle profile_handle,
                                NVDRS_SETTING* setting);
    NvAPI_Status DRS_GetSetting(NvDRSSessionHandle session_handle, NvDRSProfileHandle profile_handle, NvU32 setting_id,
                                NVDRS_SETTING* setting);
};
