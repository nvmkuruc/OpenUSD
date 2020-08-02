/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#include "pxr/usd/plugin/usdMdl/neuray.h"
#include "pxr/usd/plugin/usdMdl/utils.h"
#include "pxr/base/tf/stringUtils.h"
#include "pxr/base/tf/getenv.h"
#include "pxr/base/arch/fileSystem.h"
#include <mi/base.h>
#include <mi/neuraylib/imdl_compiler.h>
#include <iostream>
using mi::base::Handle;
using mi::base::ILogger;
using mi::neuraylib::INeuray;
using mi::neuraylib::IMdl_compiler;
using mi::neuraylib::Neuray_factory;
using mi::neuraylib::IModule;
using mi::neuraylib::IAnnotation;
using mi::neuraylib::IAnnotation_list;
using mi::neuraylib::IAnnotation_block;
using mi::neuraylib::IExpression_list;
using mi::neuraylib::IExpression;
using mi::neuraylib::IFunction_definition;
using mi::neuraylib::IMaterial_definition;
using mi::neuraylib::IValue_string;
using mi::neuraylib::IValue;
using mi::neuraylib::IExpression_constant;
using mi::neuraylib::IDatabase;
using mi::neuraylib::IMdl_discovery_api;
using mi::neuraylib::IMdl_discovery_result;
using mi::neuraylib::IMdl_package_info;
using mi::neuraylib::IMdl_info;
using mi::neuraylib::IMdl_module_info;
using mi::neuraylib::DiscoveryHelper;

using std::size_t;
using std::vector;
using std::string;
using std::cout;
using std::max;
using std::left;
using std::endl;
using std::istringstream;
using std::set;

// printf() format specifier for arguments of type LPTSTR (Windows only).
#ifdef MI_PLATFORM_WINDOWS
#ifdef UNICODE
#define FMT_LPTSTR "%ls"
#else // UNICODE
#define FMT_LPTSTR "%s"
#endif // UNICODE
#endif // MI_PLATFORM_WINDOWS

bool UsdMdl::SetNeuray(mi::neuraylib::INeuray * neuray)
{
    return mi::neuraylib::Mdl::Get().SetNeuray(neuray);
}

/// Configure the MDL SDK with module search paths and load necessary plugins.
void mi::neuraylib::Mdl::Configuration(mi::neuraylib::INeuray* neuray, ILogger* logger)
{
    Handle<IMdl_compiler> mdl_compiler(neuray->get_api_component<IMdl_compiler>());
    mdl_compiler->set_logger(logger);

    // Start removing the current directory "." from the list of search paths
    mdl_compiler->remove_module_path(".");

    // Gather list of path to add to MDL search path
    //
    // Behavior to set MDL search path is :
    // 1- Dedicated environement variable
    //    If it is set, PXR_USDMDL_PLUGIN_SEARCH_PATHS overwrites any MDL search path.
    //    PXR_USDMDL_PLUGIN_SEARCH_PATHS can be set to a list of paths.
    // 2- System and User Path
    //    if PXR_USDMDL_PLUGIN_SEARCH_PATHS is not set :
    //    + If set, add MDL_SYSTEM_PATH to the MDL search path
    //    + If set, add MDL_USER_PATH to the MDL search path
    vector<string> directories;
    {
        // Add standard path
        std::string paths = PXR_INTERNAL_NS::TfGetenv("PXR_USDMDL_PLUGIN_SEARCH_PATHS");
        if (!paths.empty())
        {
            directories = PXR_INTERNAL_NS::TfStringSplit(paths, ARCH_PATH_LIST_SEP);
        }
        else
        {
            paths = PXR_INTERNAL_NS::TfGetenv("MDL_SYSTEM_PATH");
            if (!paths.empty())
            {
                directories.emplace_back(paths);
            }
            paths = PXR_INTERNAL_NS::TfGetenv("MDL_USER_PATH");
            if (!paths.empty())
            {
                directories.emplace_back(paths);
            }
        }
    }

    for (vector<string>::const_iterator it = directories.begin();
         it != directories.end();
         it++)
    {
        string path(*it);
        const char * p(it->c_str());
        const mi::Sint32 rtn = mdl_compiler->add_module_path(p);
        if (rtn == -1)
        {
            //-1: Invalid parameters(\c NULL pointer).
            std::cerr << "MDL add module search path: Invalid parameters: " << path << std::endl;
        }
        else if (rtn == -2)
        {
            //-2: Invalid path.
            std::cerr << "MDL add module search path: Invalid path: " << path << std::endl;
        }
    }

    check_success(mdl_compiler->load_plugin_library("nv_freeimage" MI_BASE_DLL_FILE_EXT) == 0);
}

int mi::neuraylib::Mdl::g_verbosity(5);
std::once_flag mi::neuraylib::Mdl::g_onceFlagLoad;

Neuray_factory::Neuray_factory(mi::base::ILogger* logger, const char* filename)
    : m_result_code(RESULT_SUCCESS),
    m_filename(0),
    m_dso_handle(0),
    m_neuray(0)
{
    if (!filename)
#ifdef MI_MDL_SDK_H
        filename = "libmdl_sdk_usd" MI_BASE_DLL_FILE_EXT;
#else
        filename = "libneuray" MI_BASE_DLL_FILE_EXT;
#endif
    m_filename = filename;
#ifdef MI_PLATFORM_WINDOWS
    void* handle = LoadLibraryA((LPSTR)filename);
    if (!handle)
    {
        m_result_code = RESULT_LOAD_FAILURE;
        DWORD error_code = GetLastError();
        LPTSTR buffer = 0;
        //TODO                    LPTSTR message = TEXT("unknown failure");
        LPTSTR message = nullptr;
        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
            | FORMAT_MESSAGE_FROM_SYSTEM
            | FORMAT_MESSAGE_IGNORE_INSERTS, 0, error_code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buffer, 0, 0))
            message = buffer;
        if (logger)
        {
            logger->printf(mi::base::MESSAGE_SEVERITY_FATAL, "MAIN",
                "Failed to load library (%u): " FMT_LPTSTR,
                error_code, message);
        }
        else
        {
            std::cerr << "Fatal: Failed to load library " << FMT_LPTSTR << " " << error_code << " " << message << std::endl;
        }
        if (buffer)
            LocalFree(buffer);
        return;
    }
    void* symbol = GetProcAddress((HMODULE)handle, "mi_factory");
    if (!symbol) {
        m_result_code = RESULT_SYMBOL_LOOKUP_FAILURE;
        DWORD error_code = GetLastError();
        LPTSTR buffer = 0;
        //TODO                    LPTSTR message = TEXT("unknown failure");
        LPTSTR message = nullptr;
        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
            | FORMAT_MESSAGE_FROM_SYSTEM
            | FORMAT_MESSAGE_IGNORE_INSERTS, 0, error_code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buffer, 0, 0))
            message = buffer;
        if (logger)
        {
            logger->printf(mi::base::MESSAGE_SEVERITY_FATAL, "MAIN",
                "GetProcAddress error (%u): " FMT_LPTSTR, error_code, message);
        }
        else
        {
            std::cerr << "Fatal: GetProcAddress error " << FMT_LPTSTR << " " << error_code << " " << message << std::endl;
        }
        if (buffer)
        {
            LocalFree(buffer);
        }
        return;
    }
#else // MI_PLATFORM_WINDOWS
#ifdef MI_PLATFORM_MACOSX
    void* handle = dlopen(filename, RTLD_LAZY);
#else // MI_PLATFORM_MACOSX
    void* handle = dlopen(filename, RTLD_LAZY | RTLD_DEEPBIND);
#endif // MI_PLATFORM_MACOSX
    if (!handle) {
        m_result_code = RESULT_LOAD_FAILURE;
        if (logger)
            logger->message(mi::base::MESSAGE_SEVERITY_FATAL, "MAIN", dlerror());
        return;
    }
    void* symbol = dlsym(handle, "mi_factory");
    if (!symbol) {
        m_result_code = RESULT_SYMBOL_LOOKUP_FAILURE;
        if (logger)
            logger->message(mi::base::MESSAGE_SEVERITY_FATAL, "MAIN", dlerror());
        return;
    }
#endif // MI_PLATFORM_WINDOWS
    m_dso_handle = handle;

    m_neuray = mi::neuraylib::mi_factory<mi::neuraylib::INeuray>(symbol);
    if (!m_neuray) {
        mi::base::Handle<mi::neuraylib::IVersion> version(
            mi::neuraylib::mi_factory<mi::neuraylib::IVersion>(symbol));
        if (!version) {
            m_result_code = RESULT_INCOMPATIBLE_LIBRARY;
            if (logger)
            {
                logger->message(mi::base::MESSAGE_SEVERITY_FATAL, "MAIN",
                    "Incompatible SDK shared library. Could not retrieve INeuray "
                    "nor IVersion interface.");
            }
            else
            {
                std::cerr << "Fatal: Incompatible SDK shared library. Could not retrieve INeuray nor IVersion interface." << std::endl;
            }
        }
        else {
            m_result_code = RESULT_VERSION_MISMATCH;
            if (logger)
                logger->printf(mi::base::MESSAGE_SEVERITY_FATAL, "MAIN",
                    "SDK shared library version mismatch: Header version "
                    "%s does not match library version %s.",
                    MI_NEURAYLIB_PRODUCT_VERSION_STRING,
                    version->get_product_version());
            else
                std::cerr << "Fatal: SDK shared library version mismatch: Header version " <<
                MI_NEURAYLIB_PRODUCT_VERSION_STRING <<
                " does not match library version " <<
                version->get_product_version() << std::endl;
        }
    }
}

void mi::neuraylib::Mdl::Initialize()
{
    if (!m_factory)
    {
        m_factory = new mi::neuraylib::Neuray_factory();
        Neuray_factory::Result result_code(m_factory->get_result_code());
        if (result_code == Neuray_factory::RESULT_SUCCESS)
        {
            m_logger = new Logger(g_verbosity);

            // Configure the MDL SDK library
            Configuration(m_factory->get(), m_logger.get());
            // Start the MDL SDK
            m_factory->get()->start();
        }
        else
        {
            std::map<Neuray_factory::Result, std::string> error_messages = 
            { 
                { Neuray_factory::RESULT_LOAD_FAILURE, "MDL SDK shared library failed to load."}
                ,{ Neuray_factory::RESULT_SYMBOL_LOOKUP_FAILURE, "MDL SDK shared library does not contain the expected mi_factory symbol." }
                ,{ Neuray_factory::RESULT_VERSION_MISMATCH, "MDL SDK version mismatch." }
                ,{ Neuray_factory::RESULT_INCOMPATIBLE_LIBRARY, "MDL SDK incompatible library." }
            };
            std::cerr << error_messages[result_code] << std::endl;
        }
    }
}

void mi::neuraylib::Mdl::LogInternal(const std::string & msg, const mi::base::Message_severity & level)
{
    if (m_logger)
    {
        m_logger->message(level, "", msg.c_str());
    }
    else
    {
#if ! defined(DEBUG)
        // neuray not started
        // In release build, do not log message which level is not at least warning
        if (level <= mi::base::MESSAGE_SEVERITY_WARNING)
#endif
        {
            std::cerr << msg << endl;
        }
    }
}

void mi::neuraylib::Mdl::LogFatal(const std::string & msg)
{
    mi::neuraylib::Mdl::Get().LogInternal(msg, mi::base::MESSAGE_SEVERITY_FATAL);
}

void mi::neuraylib::Mdl::LogError(const std::string & msg)
{
    mi::neuraylib::Mdl::Get().LogInternal(msg, mi::base::MESSAGE_SEVERITY_ERROR);
}

void mi::neuraylib::Mdl::LogWarning(const std::string & msg)
{
    mi::neuraylib::Mdl::Get().LogInternal(msg, mi::base::MESSAGE_SEVERITY_WARNING);
}

void mi::neuraylib::Mdl::LogInfo(const std::string & msg)
{
    mi::neuraylib::Mdl::Get().LogInternal(msg, mi::base::MESSAGE_SEVERITY_INFO);
}

void mi::neuraylib::Mdl::LogVerbose(const std::string & msg)
{
    mi::neuraylib::Mdl::Get().LogInternal(msg, mi::base::MESSAGE_SEVERITY_VERBOSE);
}

void mi::neuraylib::Mdl::LogDebug(const std::string & msg)
{
    mi::neuraylib::Mdl::Get().LogInternal(msg, mi::base::MESSAGE_SEVERITY_DEBUG);
}

bool Neuray_factory::SetNeuray(mi::neuraylib::INeuray* neuray)
{
    UnloadNeuray();
    m_neuray = neuray;
    return true;
}

std::string mi::neuraylib::Module::GetModuleDBName() const
{
    return PXR_INTERNAL_NS::MdlUtils::GetDBName(GetName());
}

bool mi::neuraylib::ModuleLoader::LoadModule(mi::neuraylib::ITransaction * transactionInput)
{
    return LoadModule(transactionInput, NULL);
}

bool mi::neuraylib::ModuleLoader::LoadModule(mi::neuraylib::ITransaction * transactionInput, mi::neuraylib::INeuray * neurayIn)
{
    if (m_moduleName.empty())
    {
        return false;
    }
    mi::neuraylib::ITransaction * transaction(transactionInput);
    mi::base::Handle<mi::neuraylib::ITransaction> ownTransaction;
    mi::base::Handle<mi::neuraylib::IDatabase> ownDatabase;
    mi::base::Handle<mi::neuraylib::IScope> ownScope;
    bool rtn(false);
    mi::base::Handle<mi::neuraylib::INeuray> neuray;
    if (neurayIn)
    {
        neuray = mi::base::make_handle_dup< mi::neuraylib::INeuray>(neurayIn);
    }
    else
    {
        neuray = mi::base::make_handle_dup< mi::neuraylib::INeuray>(mi::neuraylib::Mdl::Get().GetNeuray());
    }
    if (!transaction)
    {
        // Access the database and create a transaction.
        ownDatabase = mi::base::Handle<mi::neuraylib::IDatabase>(
            neuray->get_api_component<mi::neuraylib::IDatabase>());
        ownScope = mi::base::Handle<mi::neuraylib::IScope>(ownDatabase->get_global_scope());
        ownTransaction = mi::base::Handle<mi::neuraylib::ITransaction>(ownScope->create_transaction());
        transaction = ownTransaction.get();
    }
    {
        mi::base::Handle<mi::neuraylib::IMdl_compiler> mdl_compiler(
            neuray->get_api_component<mi::neuraylib::IMdl_compiler>());
        mi::base::Handle<mi::neuraylib::IMdl_factory> mdl_factory(
            neuray->get_api_component<mi::neuraylib::IMdl_factory>());

        check_success(mdl_compiler->load_module(transaction, m_moduleName.c_str()) >= 0);

        // Access the module by its name. The name to be used here is the MDL name of the module
        // ("example") plus the "mdl::" prefix.
        mi::base::Handle<const mi::neuraylib::IModule> module(
            transaction->access<mi::neuraylib::IModule>(PXR_INTERNAL_NS::MdlUtils::GetDBName(m_moduleName).c_str()));
        check_success(module.is_valid_interface());

        if (module.is_valid_interface())
        {
            rtn = true;
            const char * fn(module->get_filename());
            m_moduleFileName = (fn ? fn : "");
            // Exported types.
            mi::base::Handle<mi::neuraylib::IType_factory> type_factory(
                mdl_factory->create_type_factory(transaction));
            mi::base::Handle<const mi::neuraylib::IType_list> types(module->get_types());
            for (mi::Size i = 0; i < types->get_size(); ++i)
            {
                mi::base::Handle<const mi::neuraylib::IType> type(types->get_type(i));
                mi::base::Handle<const mi::IString> result(type_factory->dump(type.get(), 1));
                //std::cout << "    " << result->get_c_str() << std::endl;
            }

            // Exported constants.
            mi::base::Handle<mi::neuraylib::IValue_factory> value_factory(
                mdl_factory->create_value_factory(transaction));
            mi::base::Handle<const mi::neuraylib::IValue_list> constants(module->get_constants());
            for (mi::Size i = 0; i < constants->get_size(); ++i)
            {
                mi::base::Handle<const mi::neuraylib::IValue> constant(constants->get_value(i));
                mi::base::Handle<const mi::IString> result(value_factory->dump(constant.get(), 0, 1));
                //std::cout << "    " << result->get_c_str() << std::endl;
            }

            // Function definitions of the module.
            if (m_enumFunctions)
            {
                mi::Size function_count = module->get_function_count();
                for (mi::Size i = 0; i < function_count; i++)
                {
                    const char* function_definition_name = module->get_function(i);
                    if (function_definition_name)
                    {
                        mi::base::Handle<const mi::neuraylib::IFunction_definition> function_definition(
                            transaction->access<mi::neuraylib::IFunction_definition>(function_definition_name));
                        if (function_definition)
                        {
                            FunctionCallback(function_definition.get());
                        }
                    }
                }
            }

            // Dump material definitions of the module.
            if (m_enumMaterials)
            {
                mi::Size material_count = module->get_material_count();
                for (mi::Size i = 0; i < material_count; i++)
                {
                    const char* material_definition_name(module->get_material(i));
                    if (material_definition_name)
                    {
                        mi::base::Handle<const mi::neuraylib::IMaterial_definition> material_definition(
                            transaction->access<mi::neuraylib::IMaterial_definition>(material_definition_name));
                        if (material_definition)
                        {
                            MaterialCallback(material_definition.get());
                        }
                    }
                }
            }

            // Resources referenced by this module
            for (mi::Size r = 0, rn = module->get_resources_count(); r < rn; ++r)
            {
                const char* db_name = module->get_resource_name(r);
                const char* mdl_file_path = module->get_resource_mdl_file_path(r);
                const mi::base::Handle<const mi::neuraylib::IType_resource> type(
                    module->get_resource_type(r));
                switch (type->get_kind())
                {
                case mi::neuraylib::IType::TK_TEXTURE:
                {
                    const mi::base::Handle<const mi::neuraylib::ITexture> texture(
                        transaction->access<mi::neuraylib::ITexture>(db_name));
                    if (texture)
                    {
                        const mi::base::Handle<const mi::neuraylib::IImage> image(
                            transaction->access<mi::neuraylib::IImage>(texture->get_image()));

                        for (mi::Size t = 0, tn = image->get_uvtile_length(); t < tn; ++t)
                        {
                            const char* system_file_path = image->get_filename(
                                static_cast<mi::Uint32>(t));
                        }
                    }
                    break;
                }

                case mi::neuraylib::IType::TK_LIGHT_PROFILE:
                {
                    const mi::base::Handle<const mi::neuraylib::ILightprofile> light_profile(
                        transaction->access<mi::neuraylib::ILightprofile>(db_name));
                    if (light_profile)
                    {
                        const char* system_file_path = light_profile->get_filename();
                    }
                    break;
                }

                case mi::neuraylib::IType::TK_BSDF_MEASUREMENT:
                {
                    const mi::base::Handle<const mi::neuraylib::IBsdf_measurement> mbsdf(
                        transaction->access<mi::neuraylib::IBsdf_measurement>(db_name));
                    if (mbsdf)
                    {
                        const char* system_file_path = mbsdf->get_filename();
                    }
                    break;
                }

                default:
                    break;
                }
            }
        }
    }

    if (ownTransaction)
    {
        // All transactions need to get committed.
        ownTransaction->commit();
        ownDatabase.reset();
        ownScope.reset();
        ownTransaction.reset();
    }
    return rtn;
}

std::string mi::neuraylib::Material::GetShortName() const
{
    std::string pathString(GetName());
    size_t e(pathString.find_last_of(':'));
    if (e != pathString.npos)
    {
        return pathString.substr(e + 1);
    }
    return "";
}

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

/// Module and package
std::string mi::neuraylib::Material::GetModuleName() const
{
    std::string module;
    std::string pathString(GetName());
    size_t e(pathString.find_last_of(':'));
    if (e != pathString.npos)
    {
        module = pathString.substr(0, e - 1);
        if (module.find("mdl::") == 0)
        {
            module = module.substr(5);
        }
        if (module.find("::") == 0)
        {
            module = module.substr(2);
        }
    }
    module = ReplaceAll(module, "::", "/");
    module = module + ".mdl";
    return module;
}

////////////////////////////////////////////////////////////
// Discovery_helper
////////////////////////////////////////////////////////////

bool DiscoveryHelper::Discover()
{
    if (Initialize())
    {
        return TraverseInternal(m_root.get());
    }
    return false;
}

bool DiscoveryHelper::TraverseInternal(const mi::neuraylib::IMdl_package_info * package)
{
    if(! package)
    {
        return false;
    }
    for (mi::Size i = 0; i < package->get_child_count(); i++)
    {
        Handle<const IMdl_info> child(package->get_child(i));

        const IMdl_info::Kind kind(child->get_kind());

        if (kind == IMdl_info::DK_PACKAGE)
        {
            Handle<const IMdl_package_info> child_package(
                child->get_interface<IMdl_package_info>());
            FoundPackage(child_package.get());
            TraverseInternal(child_package.get());
        }
        else if (kind == IMdl_info::DK_MODULE)
        {
            Handle<const IMdl_module_info> child_module(
                child->get_interface<IMdl_module_info>());
            FoundModule(child_module.get());
        }
    }
    return false;

}

bool DiscoveryHelper::Initialize()
{
    if (!m_root)
    {
        Handle<mi::neuraylib::INeuray> nr(mi::base::make_handle_dup< mi::neuraylib::INeuray>(mi::neuraylib::Mdl::Get().GetNeuray()));
        // Use discovery API to get some information regarding MDL elements
        Handle<IMdl_discovery_api> discovery(nr->get_api_component<IMdl_discovery_api>());
        Handle<const IMdl_discovery_result> discovery_result(discovery->discover());
        m_root = Handle<const IMdl_package_info>(discovery_result->get_graph());
    }
    return m_root != NULL;
}

void DiscoveryHelper::FoundModule(const mi::neuraylib::IMdl_module_info * elem)
{
    Mdl::LogVerbose("Found MDL module: " + string(elem->get_qualified_name()));
}

void DiscoveryHelper::FoundPackage(const mi::neuraylib::IMdl_package_info * elem)
{
    Mdl::LogVerbose("MDL Package: " + string(elem->get_qualified_name()));
}

// Logger

mi::neuraylib::Logger::Logger(int level) :
    m_level(level)
{
    if (m_to_file)
    {
        m_file.open("UsdMdlLog.text", std::ios_base::out);
    }
}

mi::neuraylib::Logger::~Logger()
{
    // Close file stream
    if (m_file.is_open())
    {
        m_file.close();
    }
}

void mi::neuraylib::Logger::message(
    mi::base::Message_severity level,
    const char* module_category,
    const mi::base::Message_details&,
    const char* message)
{
    if (int(level) < m_level)
    {
        static std::map<mi::base::Message_severity, std::string> LevelNames =
        {
              { mi::base::MESSAGE_SEVERITY_FATAL, "fatal" }
            , { mi::base::MESSAGE_SEVERITY_ERROR, "error" }
            , { mi::base::MESSAGE_SEVERITY_WARNING, "warn" }
            , { mi::base::MESSAGE_SEVERITY_INFO, "info" }
            , { mi::base::MESSAGE_SEVERITY_VERBOSE, "verb" }
            , { mi::base::MESSAGE_SEVERITY_DEBUG, "debug" }
        };
        std::string severity(LevelNames[level]);
        // We want to ignore module_category
        std::cerr << severity << ":\t" << message << endl;
        if (m_file.is_open())
        {
            m_file << severity << ":\t" << message << endl;
        }
    }
}
