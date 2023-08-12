// class header include
#include "nvapiwrapper.h"

// special place for Windows' include
#include <windows.h>

// system includes
#include <iostream>
#include <map>
#include <mutex>
#include <stdexcept>
#include <system_error>

// local includes
#include "nvapi-open-source-sdk/nvapi_interface.h"

//--------------------------------------------------------------------------------------------------

namespace
{
#ifdef _WIN64
const auto DLL_NAME{"nvapi64.dll"};
#else
const auto DLL_NAME{"nvapi.dll"};
#endif

//--------------------------------------------------------------------------------------------------

std::string getErrorString(auto&& error)
{
    return std::system_category().message(static_cast<int>(error));
}

//--------------------------------------------------------------------------------------------------

class NvApiInterface
{
public:
    static NvApiInterface& get();

    std::string load();
    void        unload();

    template<class Func, class... Args>
    NvAPI_Status call(const char* name, Args&&... args);

private:
    explicit NvApiInterface() = default;

    void freeDll();

    std::mutex                   m_dll_mutex;
    std::size_t                  m_dll_use_counter{0};
    HMODULE                      m_dll_handle{nullptr};
    std::map<const char*, void*> m_dll_interfaces;
};

//--------------------------------------------------------------------------------------------------

NvApiInterface& NvApiInterface::get()
{
    static NvApiInterface instance;
    return instance;
}

//--------------------------------------------------------------------------------------------------

std::string NvApiInterface::load()
{
    std::lock_guard lock{m_dll_mutex};

    if (m_dll_use_counter == 0)
    {
        m_dll_handle = LoadLibraryExA(DLL_NAME, NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (!m_dll_handle)
        {
            return "Failed to load \"" + std::string{DLL_NAME} + "\". Error: " + getErrorString(GetLastError());
        }

        const auto load_interfaces{
            [this]()
            {
                using QueryInterface = void*(WINAPIV*)(NvU32 id);
                auto query_interface{reinterpret_cast<QueryInterface>(
                    reinterpret_cast<void*>(GetProcAddress(m_dll_handle, "nvapi_QueryInterface")))};
                if (!query_interface)
                {
                    return "Failed to load \"nvapi_QueryInterface\". Error: " + getErrorString(GetLastError());
                }

                for (const auto& item : nvapi_interface_table)
                {
                    void* iface{query_interface(item.id)};
                    if (!iface)
                    {
                        return "Failed to load \"" + std::string{item.func} + "\" interface!";
                    }

                    m_dll_interfaces[item.func] = iface;
                }

                return std::string{};
            }};

        auto error_str{load_interfaces()};
        if (!error_str.empty())
        {
            freeDll();
            return error_str;
        }
    }

    m_dll_use_counter++;
    return std::string{};
}

//--------------------------------------------------------------------------------------------------

void NvApiInterface::unload()
{
    std::lock_guard lock{m_dll_mutex};

    if (m_dll_use_counter > 0)
    {
        m_dll_use_counter--;
        if (m_dll_use_counter == 0)
        {
            freeDll();
        }
    }
}

//--------------------------------------------------------------------------------------------------

void NvApiInterface::freeDll()
{
    if (m_dll_handle)
    {
        m_dll_interfaces.clear();
        FreeLibrary(m_dll_handle);
        m_dll_handle = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------

template<class Func, class... Args>
NvAPI_Status NvApiInterface::call(const char* name, Args&&... args)
{
    auto func{reinterpret_cast<Func*>(m_dll_interfaces[name])};
    if (!func)
    {
        return m_dll_interfaces.empty() ? NVAPI_API_NOT_INITIALIZED : NVAPI_NOT_SUPPORTED;
    }

    return func(std::forward<Args>(args)...);
}
}  // namespace

//--------------------------------------------------------------------------------------------------

NvApiWrapper::NvApiWrapper()
{
    auto error_str{NvApiInterface::get().load()};
    if (!error_str.empty())
    {
        throw std::runtime_error(error_str);
    }
}

//--------------------------------------------------------------------------------------------------

NvApiWrapper::~NvApiWrapper()
{
    NvApiInterface::get().unload();
}

//--------------------------------------------------------------------------------------------------

std::string NvApiWrapper::getErrorMessage(NvAPI_Status status)
{
    NvAPI_ShortString szDesc = {0};
    NvApiInterface::get().call<decltype(NvAPI_GetErrorMessage)>("NvAPI_GetErrorMessage", status, szDesc);
    return std::string{szDesc};
}

//--------------------------------------------------------------------------------------------------

NvAPI_Status NvApiWrapper::DRS_CreateSession(NvDRSSessionHandle* handle)
{
    return NvApiInterface::get().call<decltype(NvAPI_DRS_CreateSession)>("NvAPI_DRS_CreateSession", handle);
}

//--------------------------------------------------------------------------------------------------

NvAPI_Status NvApiWrapper::DRS_DestroySession(NvDRSSessionHandle handle)
{
    return NvApiInterface::get().call<decltype(NvAPI_DRS_DestroySession)>("NvAPI_DRS_DestroySession", handle);
}

//--------------------------------------------------------------------------------------------------

NvAPI_Status NvApiWrapper::DRS_LoadSettings(NvDRSSessionHandle handle)
{
    return NvApiInterface::get().call<decltype(NvAPI_DRS_LoadSettings)>("NvAPI_DRS_LoadSettings", handle);
}

//--------------------------------------------------------------------------------------------------

NvAPI_Status NvApiWrapper::DRS_SaveSettings(NvDRSSessionHandle handle)
{
    return NvApiInterface::get().call<decltype(NvAPI_DRS_SaveSettings)>("NvAPI_DRS_SaveSettings", handle);
}

//--------------------------------------------------------------------------------------------------

NvAPI_Status NvApiWrapper::DRS_GetBaseProfile(NvDRSSessionHandle session_handle, NvDRSProfileHandle* profile_handle)
{
    return NvApiInterface::get().call<decltype(NvAPI_DRS_GetBaseProfile)>("NvAPI_DRS_GetBaseProfile", session_handle,
                                                                          profile_handle);
}

//--------------------------------------------------------------------------------------------------

NvAPI_Status NvApiWrapper::DRS_SetSetting(NvDRSSessionHandle session_handle, NvDRSProfileHandle profile_handle,
                                          NVDRS_SETTING* setting)
{
    return NvApiInterface::get().call<decltype(NvAPI_DRS_SetSetting)>("NvAPI_DRS_SetSetting", session_handle,
                                                                      profile_handle, setting);
}

//--------------------------------------------------------------------------------------------------

NvAPI_Status NvApiWrapper::DRS_GetSetting(NvDRSSessionHandle session_handle, NvDRSProfileHandle profile_handle,
                                          NvU32 setting_id, NVDRS_SETTING* setting)
{
    return NvApiInterface::get().call<decltype(NvAPI_DRS_GetSetting)>("NvAPI_DRS_GetSetting", session_handle,
                                                                      profile_handle, setting_id, setting);
}
