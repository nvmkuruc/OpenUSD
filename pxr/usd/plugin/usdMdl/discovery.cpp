/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#include "pxr/pxr.h"
#include "pxr/usd/ndr/declare.h"
#include "pxr/usd/ndr/discoveryPlugin.h"
#include "pxr/usd/ndr/filesystemDiscoveryHelpers.h"
#include "pxr/base/tf/getenv.h"
#include "pxr/base/tf/staticTokens.h"
#include "pxr/base/tf/stringUtils.h"
#include "pxr/base/arch/fileSystem.h"
#include <algorithm>
#include <cctype>

#include "pxr/usd/plugin/usdMdl/neuray.h"

using mi::neuraylib::Mdl;

PXR_NAMESPACE_OPEN_SCOPE

namespace {

    TF_DEFINE_PRIVATE_TOKENS(
        _tokens,

        ((discoveryType, "mdl"))

        ((defaultSourceType, "mdl"))
    );

    class ModuleDiscovery : public mi::neuraylib::ModuleLoader
    {
        NdrNodeDiscoveryResultVec & m_resultvec;

    protected:
        virtual void FunctionCallback(const mi::neuraylib::IFunction_definition * function_definition) override
        {
            Mdl::LogVerbose("[ModuleDiscovery] function: " + std::string(function_definition->get_mdl_name()));
            const char * name(function_definition->get_mdl_name());
            if (name)
            {
                mi::neuraylib::Function m(name);
                std::string moduleName(m.GetModuleName());
                std::string materialShortName(m.GetShortName());

                // Build metadata 
                NdrTokenMap metadata;
                metadata[TfToken("info:mdl:sourceAsset")] = moduleName;
                metadata[TfToken("info:mdl:sourceAsset:subIdentifier")] = materialShortName;

                std::string sourceCode;

                NdrNodeDiscoveryResult result(
                    NdrIdentifier(name),                // identifier
                    NdrVersion().GetAsDefault(),        // version unused
                    name,                               // name
                    TfToken(),                          // family
                    _tokens->discoveryType,             // discoveryType
                    _tokens->discoveryType,             // sourceType
                    GetModuleName(),                    // uri
                    GetModuleFileName(),                // resovedUri
                    sourceCode,                         // sourceCode
                    metadata                            // metadata
                );
                m_resultvec.emplace_back(result);
            }
        }

        virtual void MaterialCallback(const mi::neuraylib::IMaterial_definition * material_definition) override
        {
            Mdl::LogVerbose("[ModuleDiscovery] material: " + std::string(material_definition->get_mdl_name()));
            const char * name(material_definition->get_mdl_name());
            if (name)
            {
                mi::neuraylib::Material m(name);
                std::string moduleName(m.GetModuleName());
                std::string materialShortName(m.GetShortName());

                // Build metadata 
                NdrTokenMap metadata;
                metadata[TfToken("info:mdl:sourceAsset")] = moduleName;
                metadata[TfToken("info:mdl:sourceAsset:subIdentifier")] = materialShortName;

                std::string sourceCode;

                NdrNodeDiscoveryResult result(
                    NdrIdentifier(name),                // identifier
                    NdrVersion().GetAsDefault(),        // version unused
                    name,                               // name
                    TfToken(),                          // family
                    _tokens->discoveryType,             // discoveryType
                    _tokens->discoveryType,             // sourceType
                    GetModuleName(),                    // uri
                    GetModuleFileName(),                // resovedUri
                    sourceCode,                         // sourceCode
                    metadata                            // metadata
                );
                m_resultvec.emplace_back(result);
            }
        }
    public:
        ModuleDiscovery(NdrNodeDiscoveryResultVec & resultvec)
            : m_resultvec(resultvec)
        {
            SetEnumFunctions(true);
            SetEnumMaterials(true);
        }
    };


    class DiscoverMDLNodes : public mi::neuraylib::DiscoveryHelper
    {
        NdrNodeDiscoveryResultVec & m_resultvec;
    protected:
        void FoundModule(const mi::neuraylib::IMdl_module_info * elem) override
        {

            if (elem)
            {
                const char * qname(elem->get_qualified_name());
                if (qname)
                {
                    ModuleDiscovery discovery(m_resultvec);
                    discovery.SetModuleName(elem->get_qualified_name());
                    discovery.LoadModule();
                }
            }
        }
    public:
        DiscoverMDLNodes(NdrNodeDiscoveryResultVec & resultvec)
            : m_resultvec(resultvec)
        {
            Discover();
        }
    };

} // anonymous namespace

/// Custom way of discovering MDL nodes
class DiscoveryResults
{
public:
    void AddFunction(const mi::neuraylib::IModule * module, const mi::neuraylib::IFunction_definition * function_definition);

    void AddMaterial(const mi::neuraylib::IModule * module, const mi::neuraylib::IMaterial_definition * material_definition);

    NdrNodeDiscoveryResultVec & GetDiscoveryResults();
};

/// Custom way of discovering MDL nodes
class UsdMdlDiscoveryPluginCallback
{
public:
    virtual bool BuildDiscoveryResults(NdrNodeDiscoveryResultVec & vec);
};

static void SetUsdMdlDiscoveryPluginCallback(UsdMdlDiscoveryPluginCallback* callback);

/// Discovers nodes in MDL files.
class UsdMdlDiscoveryPlugin : public NdrDiscoveryPlugin 
{
public:
    UsdMdlDiscoveryPlugin();
    ~UsdMdlDiscoveryPlugin() override = default;

    /// Discover all of the nodes that appear within the the search paths
    /// provided and match the extensions provided.
    NdrNodeDiscoveryResultVec DiscoverNodes(const Context&) override;

    /// Gets the paths that this plugin is searching for nodes in.
    const NdrStringVec& GetSearchURIs() const override;

public:
    static void Enable(bool enable)
    {
        g_enabledFlag = enable;
    }

private:
    /// The paths (abs) indicating where the plugin should search for nodes.
    NdrStringVec _searchPaths;
    NdrStringVec _allSearchPaths;
private:
    static bool g_enabledFlag;
    
    friend void EnableMdlDiscoveryPlugin(bool enable);
};

bool UsdMdlDiscoveryPlugin::g_enabledFlag = true;

void EnableMdlDiscoveryPlugin(bool enable)
{
    UsdMdlDiscoveryPlugin::g_enabledFlag = enable;
}

UsdMdlDiscoveryPlugin::UsdMdlDiscoveryPlugin()
{
}

NdrNodeDiscoveryResultVec
UsdMdlDiscoveryPlugin::DiscoverNodes(const Context& context)
{
    NdrNodeDiscoveryResultVec resultvec;
    
    if (UsdMdlDiscoveryPlugin::g_enabledFlag)
    {
        DiscoverMDLNodes helper(resultvec);
    }

    return resultvec;
}

const NdrStringVec&
UsdMdlDiscoveryPlugin::GetSearchURIs() const
{
    return _allSearchPaths;
}

NDR_REGISTER_DISCOVERY_PLUGIN(UsdMdlDiscoveryPlugin)

PXR_NAMESPACE_CLOSE_SCOPE
