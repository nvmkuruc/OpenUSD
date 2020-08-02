/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#include "pxr/pxr.h"
#include "pxr/usd/ndr/debugCodes.h"
#include "pxr/usd/ndr/node.h"
#include "pxr/usd/ndr/nodeDiscoveryResult.h"
#include "pxr/usd/ndr/parserPlugin.h"
#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdr/shaderNode.h"
#include "pxr/usd/sdr/shaderProperty.h"
#include "pxr/base/tf/pathUtils.h"
#include "pxr/base/tf/staticTokens.h"
#include "pxr/base/tf/stringUtils.h"
#include "pxr/usd/plugin/usdMdl/neuray.h"
#include "pxr/usd/plugin/usdMdl/utils.h"
#include "pxr/usd/plugin/usdMdl/mdlToUsd.h"
#include <mi/neuraylib/imdl_compiler.h>
#include <iostream>

using std::string;
using mi::neuraylib::Mdl;
using mi::base::Handle;
using mi::neuraylib::IMaterial_definition;
using mi::neuraylib::IFunction_definition;
using mi::neuraylib::IFunction_call;
using mi::neuraylib::IExpression;
using mi::neuraylib::IType;
using mi::neuraylib::IType_alias;
using mi::neuraylib::IType_compound;
using mi::neuraylib::IType_array;
using mi::neuraylib::IType_struct;
using mi::neuraylib::IScene_element;
using mi::neuraylib::IValue_float;
using mi::neuraylib::IValue_int;
using mi::neuraylib::IValue_double;

PXR_NAMESPACE_OPEN_SCOPE

namespace {

TF_DEFINE_PRIVATE_TOKENS(
    _tokens,

    ((discoveryType, "mdl"))
    ((sourceType, "mdl"))

    // The name to use for unnamed outputs.
    ((defaultOutputName, "result"))
);

class ShaderBuilder
{
private:
    const NdrNodeDiscoveryResult & m_discoveryResult;
    std::string m_MDLElementQualifiedName;
    mi::base::Handle<mi::neuraylib::INeuray> m_neuray;
    mi::base::Handle<mi::neuraylib::ITransaction> m_transaction;
    mi::base::Handle<mi::neuraylib::IMdl_compiler> m_mdl_compiler;
    mi::base::Handle<mi::neuraylib::IMdl_factory> m_mdl_factory;
private:
    bool ConvertParm(const IType * type, const IExpression * defaultValue, TfToken & typeOut, VtValue & defaultValueOut, MdlToUsd::ExtraInfo & extra)
    {
        MdlToUsd converter;
        typeOut = converter.ConvertType(type);
        if (typeOut.IsEmpty())
        {
            return false;
        }
        if (defaultValue)
        {
            defaultValueOut = converter.GetValue(type, defaultValue, typeOut, extra);
        }
        return true;
    }

    template<class T> bool traverseFunctionOrMaterial(const T * scene_element, NdrPropertyUniquePtrVec & properties)
    {
        mi::Size count = scene_element->get_parameter_count();
        mi::base::Handle<const mi::neuraylib::IType_list> types(scene_element->get_parameter_types());
        mi::base::Handle<const mi::neuraylib::IExpression_list> defaults(scene_element->get_defaults());
        mi::base::Handle<const mi::neuraylib::IAnnotation_list> parm_annotations(scene_element->get_parameter_annotations());

        mi::base::Handle<const IFunction_definition> function(scene_element->template get_interface<IFunction_definition>());
        if (function)
        {
            mi::base::Handle<const mi::neuraylib::IType> rtn_type(function->get_return_type());

            // TODO
        }

        for (mi::Size index = 0; index < count; index++)
        {
            mi::base::Handle<const IType> parm_type(types->get_type(index));
            std::string parm_name = scene_element->get_parameter_name(index);

            mi::base::Handle<const mi::neuraylib::IExpression> defaultexp(defaults->get_expression(parm_name.c_str()));
            if (!defaultexp.is_valid_interface())
            {
                continue;
            }

            TfToken type;
            VtValue defaultValue;

            MdlToUsd::ExtraInfo extra;
            if (!ConvertParm(parm_type.get(), defaultexp.get(), type, defaultValue, extra))
            {
                mi::neuraylib::Mdl::LogError("[ShaderBuilder] Failed to convert " + std::string(scene_element->get_mdl_name()) + " " + parm_name);
                continue;
            }

            TfToken name(parm_name);
            bool isOutput(false);
            NdrTokenMap metadata;
            NdrTokenMap hints;
            NdrOptionVec options;

            // Add metadata to enum values
            if (extra.m_valueType == MdlToUsd::ExtraInfo::VTT_ENUM)
            {
                //// From Pixar doc about Metadata RenderType : https://graphics.pixar.com/usd/docs/api/class_usd_shade_input.html#afeeb2fbd92c5600adc6e8ae9eeda5772
                metadata[TfToken("renderType")] = extra.m_enumSymbol;
                metadata[TfToken("__SDR__enum_value")] = extra.m_enumValueName;
                if (!extra.m_enumValueDescription.empty())
                {
                    metadata[TfToken("description")] = extra.m_enumValueDescription;
                }

                // Parse options string which contains all valid enum values (e.g. "color_layer_blend:0|color_layer_add:1|color_layer_multiply:2|...")
                std::string enumValuePair;
                std::stringstream input_stringstream(extra.m_enumValueOptions);
                while (std::getline(input_stringstream, enumValuePair, '|'))
                {
                    std::stringstream input2_stringstream(enumValuePair);
                    std::string enumDisplayName;
                    std::string enumValue;
                    if (std::getline(input2_stringstream, enumDisplayName, ':'))
                    {
                        if (std::getline(input2_stringstream, enumValue, ':'))
                        {
			  options.emplace_back(NdrOption(std::make_pair(TfToken(enumDisplayName), TfToken(enumValue))));
                        }
                    }
                }
            }
            if (extra.m_valueType == MdlToUsd::ExtraInfo::VTT_TYPE_INFO)
            {
                std::string key(MdlToUsd::KEY_MDL_TYPE_INFO);
                key += ":string";
                metadata[TfToken(key.c_str())] = extra.m_mdlType;
            }

            // Name of annotation, string value for annotation
            std::map<std::string, std::string> parm_metadata;
            MdlToUsd::extractAnnotations(parm_name, parm_annotations.get(), parm_metadata);
            for (auto & anno : parm_metadata)
            {
                metadata[TfToken(anno.first)] = anno.second;
            }

            std::map<std::string, VtValue> customdata;
            MdlToUsd::extractAnnotations(parm_name, parm_annotations.get(), customdata);
            storeAnnotations(customdata, metadata);

            // Add the property.
            properties.push_back(
                SdrShaderPropertyUniquePtr(
                    new SdrShaderProperty(name,
                                            type,
                                            defaultValue,
                                            isOutput,
                                            defaultValue.IsArrayValued() ? defaultValue.GetArraySize() : 0,
                                            metadata,
                                            hints,
                                            options)));
        }
        return true;
    }
    
    // Store annotations as metadata
    void storeAnnotations(const std::map<std::string, VtValue> & customdata, NdrTokenMap & metadata) const
    {
        class VtValueTypeToUsdTypeMap : public std::map<std::string, std::string>
        {
        public:
            VtValueTypeToUsdTypeMap(const std::map<std::string, std::string> & init)
                :std::map<std::string, std::string>(init)
            {}
            std::string convert(const std::string & type) const
            {
                VtValueTypeToUsdTypeMap::const_iterator it = find(type);
                if (it != end())
                {
                    return it->second;
                }
                return type;
            }
        };

        static VtValueTypeToUsdTypeMap typeMap({
             { "GfVec2i", "int2" }
            ,{ "GfVec3i", "int3" }
            ,{ "GfVec4i", "int4" }
            ,{ "GfVec2f", "float2" }
            ,{ "GfVec3f", "float3" }
            ,{ "GfVec4f", "float4" }
            ,{ "GfVec2d", "double2" }
            ,{ "GfVec3d", "double3" }
            ,{ "GfVec4d", "double4" }
            ,{ "GfMatrix2d", "matrix2d" }
            ,{ "GfMatrix3d", "matrix3d" }
            ,{ "GfMatrix4d", "matrix4d" }
        });

        if (!customdata.empty())
        {
            //VtDictionary dict(object.GetCustomData());
            for (const auto & anno : customdata)
            {
                // Encode value as string
                std::stringstream strValue;
                strValue << anno.second;

                // Store type
                std::string type(typeMap.convert(anno.second.GetTypeName()));

                // Key = mdl:annotations:type + [annotation name]
                std::string key("mdl:annotations:" + type + anno.first);

                //dict[anno.first] = anno.second;
                metadata[TfToken(key)] = strValue.str();
            }
        }
    }

public:
    ShaderBuilder(const NdrNodeDiscoveryResult& discoveryResult)
        : m_discoveryResult(discoveryResult)
    {
        SetMDLElementQualifiedName(m_discoveryResult.identifier.GetString());
    }

    ~ShaderBuilder()
    {
    }

    void SetMDLElementQualifiedName(const std::string & element)
    {
        m_MDLElementQualifiedName = element;
    }
    
    NdrNodeUniquePtr Build()
    {
        m_neuray = mi::base::make_handle_dup< mi::neuraylib::INeuray>(mi::neuraylib::Mdl::Get().GetNeuray());
        // Access the database and create a transaction.
        mi::base::Handle<mi::neuraylib::IDatabase> database(
            m_neuray->get_api_component<mi::neuraylib::IDatabase>());
        mi::base::Handle<mi::neuraylib::IScope> scope(database->get_global_scope());
        m_transaction = scope->create_transaction();
        NdrNodeUniquePtr rtn;
        if (!m_transaction)
        {
            Mdl::LogError("[ShaderBuilder::Build()] Unable to create transaction");
            return rtn;
        }
        m_mdl_compiler = m_neuray->get_api_component<mi::neuraylib::IMdl_compiler>();
        m_mdl_factory = m_neuray->get_api_component<mi::neuraylib::IMdl_factory>();

        NdrPropertyUniquePtrVec properties;

        std::string id(m_MDLElementQualifiedName);
        std::map<std::string, std::string> metadata;
        id = MdlUtils::GetDBName(id);
        {
            mi::base::Handle<const IMaterial_definition> material(m_transaction->access<IMaterial_definition>(id.c_str()));
            mi::base::Handle<const mi::neuraylib::IAnnotation_block> annotations;
            if (material)
            {
                bool rtn(traverseFunctionOrMaterial<IMaterial_definition>(material.get(), properties));
                annotations = mi::base::Handle<const mi::neuraylib::IAnnotation_block>(material->get_annotations());
            }
            else
            {
                mi::base::Handle<const IFunction_definition> function(m_transaction->access<IFunction_definition>(id.c_str()));
                if (function)
                {
                    bool rtn(traverseFunctionOrMaterial<IFunction_definition>(function.get(), properties));
                    annotations = mi::base::Handle<const mi::neuraylib::IAnnotation_block>(function->get_annotations());
                }
                else
                {
                    mi::base::Handle<const IFunction_call> fcall(m_transaction->access<IFunction_call>(id.c_str()));
                    if (fcall)
                    {
                        std::cout << "fct call" << std::endl;
                    }
                }
            }

            if (annotations)
            {
                MdlToUsd::extractAnnotations(annotations.get(), metadata);
            }

            NdrTokenMap shader_metadata(std::move(m_discoveryResult.metadata));
            for (auto & anno : metadata)
            {
                shader_metadata[TfToken(anno.first)] = anno.second;
            }

            std::map<std::string, VtValue> customdata;
            MdlToUsd::extractAnnotations(annotations.get(), customdata);
            storeAnnotations(customdata, shader_metadata);

            TfToken context; // ???

            rtn = NdrNodeUniquePtr(
                new SdrShaderNode(m_discoveryResult.identifier,
                                  m_discoveryResult.version,
                                  m_discoveryResult.name,
                                  m_discoveryResult.family,
                                  context,
                                  m_discoveryResult.sourceType,
                                  m_discoveryResult.uri,
                                  m_discoveryResult.resolvedUri,
                                  std::move(properties),
                                  shader_metadata));
        }
        m_transaction->commit();

        return rtn;
    }
};
} // anonymous namespace

/// Parses nodes in MDL files.
class UsdMdlParserPlugin : public NdrParserPlugin {
public:
    UsdMdlParserPlugin() = default;
    ~UsdMdlParserPlugin() override = default;

    NdrNodeUniquePtr Parse(
        const NdrNodeDiscoveryResult& discoveryResult) override;
    const NdrTokenVec& GetDiscoveryTypes() const override;
    const TfToken& GetSourceType() const override;
};

NdrNodeUniquePtr
UsdMdlParserPlugin::Parse(
    const NdrNodeDiscoveryResult& discoveryResultIn)
{
    NdrNodeDiscoveryResult discoveryResult(discoveryResultIn);
    for (auto & m : discoveryResult.metadata)
    {
        if (m.first == "info:mdl:sourceAsset:subIdentifier")
        {
#if 0
            //discoveryResult.identifier = NdrIdentifier(m.second);
            //discoveryResult.name = NdrIdentifier(m.second);
            NdrNodeConstPtr nodeConst = SdrRegistry::GetInstance().GetNodeByIdentifier(NdrIdentifier(m.second));
            NdrNodePtr node((NdrNodePtr)nodeConst);

            if (node)
            {
                Mdl::LogVerbose("[UsdMdlParserPlugin] node identifier: " + node->GetIdentifier().GetString());
                Mdl::LogVerbose("[UsdMdlParserPlugin] node name: " + node->GetName());
                Mdl::LogVerbose("[UsdMdlParserPlugin] node version: " + string(node->GetVersion().GetString()));
                Mdl::LogVerbose("[UsdMdlParserPlugin] node family: " + string(node->GetFamily()));
                Mdl::LogVerbose("[UsdMdlParserPlugin] node sourceType: " + string(node->GetSourceType()));

                Mdl::LogVerbose("[UsdMdlParserPlugin] discoveryResult identifier: " + discoveryResult.identifier.GetString());
                Mdl::LogVerbose("[UsdMdlParserPlugin] discoveryResult name: " + discoveryResult.name);
                Mdl::LogVerbose("[UsdMdlParserPlugin] discoveryResult version: " + string(discoveryResult.version.GetString()));
                Mdl::LogVerbose("[UsdMdlParserPlugin] discoveryResult family: " + string(discoveryResult.family));
                Mdl::LogVerbose("[UsdMdlParserPlugin] discoveryResult sourceType: " + string(discoveryResult.sourceType));

                return NdrNodeUniquePtr(node);
            }
#endif
        }
    }

    Mdl::LogVerbose("[UsdMdlParserPlugin] MDL discovery result");
    Mdl::LogVerbose("[UsdMdlParserPlugin] identifier: " + discoveryResult.identifier.GetString());
    Mdl::LogVerbose("[UsdMdlParserPlugin] version: " + string(discoveryResult.version.GetString()));
    Mdl::LogVerbose("[UsdMdlParserPlugin] name: " + discoveryResult.name);
    Mdl::LogVerbose("[UsdMdlParserPlugin] family: " + string(discoveryResult.family));
    Mdl::LogVerbose("[UsdMdlParserPlugin] discoveryType: " + string(discoveryResult.discoveryType));
    Mdl::LogVerbose("[UsdMdlParserPlugin] sourceType: " + string(discoveryResult.sourceType));
    Mdl::LogVerbose("[UsdMdlParserPlugin] uri: " + discoveryResult.uri);
    Mdl::LogVerbose("[UsdMdlParserPlugin] resolvedUri: " + discoveryResult.resolvedUri);
    Mdl::LogVerbose("[UsdMdlParserPlugin] sourceCode: " + discoveryResult.sourceCode);
    for( auto & m: discoveryResult.metadata)
    {
        Mdl::LogVerbose("[UsdMdlParserPlugin] metadata: " + string(m.first) + " / " + string(m.second));
    }
    Mdl::LogVerbose("[UsdMdlParserPlugin] blindData: " + discoveryResult.blindData);

    ShaderBuilder builder(discoveryResult);

    for (auto & m : discoveryResult.metadata)
    {
        if (m.first == "info:mdl:sourceAsset:subIdentifier")
        {
            std::string moduleName(MdlUtils::GetModuleNameFromModuleFilename(discoveryResult.uri));

            // Comment out the following, the MDL Element Qualified Name is set in the ctor of ShaderBuilder
            // // Build MDL qualifed name from module name and element name
            // std::string elementName(m.second);
            // std::string qualifiedName = moduleName + "::" + elementName;
            // builder.SetMDLElementQualifiedName(qualifiedName);

            // Load module
            mi::neuraylib::ModuleLoader loader;
            loader.SetModuleName(moduleName);
            loader.LoadModule();
        }
    }

    return builder.Build();
}

const NdrTokenVec&
UsdMdlParserPlugin::GetDiscoveryTypes() const
{
    static const NdrTokenVec discoveryTypes = {
        _tokens->discoveryType
    };
    return discoveryTypes;
}

const TfToken&
UsdMdlParserPlugin::GetSourceType() const
{
    return _tokens->sourceType;
}

NDR_REGISTER_PARSER_PLUGIN(UsdMdlParserPlugin)

PXR_NAMESPACE_CLOSE_SCOPE
