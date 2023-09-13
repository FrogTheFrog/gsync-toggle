// system includes
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>

// local includes
#include "nvapidrssession.h"

//--------------------------------------------------------------------------------------------------

void assertSuccess(NvAPI_Status status, std::string_view error_prefix)
{
    if (status != NVAPI_OK)
    {
        NvApiWrapper nvapi;

        std::string error{error_prefix};
        error += " Error: " + nvapi.getErrorMessage(status);

        throw std::runtime_error(error);
    }
}

//--------------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    try
    {
        const std::set<std::string>    possible_values{"status", "0", "1", "2"};
        const std::vector<std::string> args(argv, argv + argc);

        if (args.size() != 2 || !possible_values.contains(args[1]))
        {
            // clang-format off
            std::cout << std::endl;
            std::cout << "  Usage:" << std::endl;
            std::cout << "    gsynctoggle status    prints the current G-Sync status. See below for possible status values." << std::endl;
            std::cout << "    gsynctoggle 0         turns the G-Sync off." << std::endl;
            std::cout << "    gsynctoggle 1         turns the G-Sync on for fullscreen mode only." << std::endl;
            std::cout << "    gsynctoggle 2         turns the G-Sync on for fullscreen and windowed modes." << std::endl;
            std::cout << std::endl;
            // clang-format on
            return EXIT_SUCCESS;
        }

        NvApiWrapper       nvapi;
        NvApiDrsSession    drs_session;
        NvDRSProfileHandle drs_profile{nullptr};
        NVDRS_SETTING      drs_setting{};

        drs_setting.version = NVDRS_SETTING_VER;

        assertSuccess(nvapi.DRS_LoadSettings(drs_session), "Failed to load session settings!");
        assertSuccess(nvapi.DRS_GetBaseProfile(drs_session, &drs_profile), "Failed to get base profile!");
        assertSuccess(nvapi.DRS_GetSetting(drs_session, drs_profile, VRR_MODE_ID, &drs_setting),
                      "Failed to get VRR setting!");

        if (args[1] == "status")
        {
            std::cout << drs_setting.u32CurrentValue << std::endl;
            return EXIT_SUCCESS;
        }

        auto requested_setting{std::stoul(args[1])};
        if (requested_setting == drs_setting.u32CurrentValue)
        {
            return EXIT_SUCCESS;
        }

        if (requested_setting >= VRR_MODE_NUM_VALUES)
        {
            throw std::runtime_error("Unsupported VRR mode specified!");
        }

        drs_setting.u32CurrentValue = requested_setting;

        assertSuccess(nvapi.DRS_SetSetting(drs_session, drs_profile, &drs_setting), "Failed to set VRR setting!");
        assertSuccess(nvapi.DRS_SaveSettings(drs_session), "Failed to save session settings!");
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}