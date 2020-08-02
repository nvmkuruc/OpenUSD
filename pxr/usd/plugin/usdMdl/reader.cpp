/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#include "pxr/pxr.h"

#include "pxr/usd/ndr/declare.h"
#include "pxr/usd/sdf/attributeSpec.h"
#include "pxr/usd/sdf/layer.h"
#include "pxr/usd/sdf/tokens.h"
#include "pxr/usd/sdf/types.h"
#include "pxr/usd/usd/collectionAPI.h"
#include "pxr/usd/usd/editContext.h"
#include "pxr/usd/usd/specializes.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/base/tf/stringUtils.h"
#include "pxr/usd/usdGeom/primvar.h"
#include "pxr/usd/usdGeom/primvarsAPI.h"
#include "pxr/usd/usdGeom/xform.h"
#include "pxr/usd/usdGeom/sphere.h"
#include "pxr/usd/plugin/usdMdl/reader.h"
#include "pxr/usd/plugin/usdMdl/neuray.h"
#include "pxr/usd/plugin/usdMdl/utils.h"
#include "pxr/usd/usdShade/material.h"
#include "pxr/usd/usdShade/materialBindingAPI.h"
#include "pxr/usd/usdShade/nodeGraph.h"
#include "pxr/usd/usdShade/shader.h"
#include "pxr/usd/usdShade/tokens.h"
#include "pxr/usd/usdShade/utils.h"
#include "pxr/usd/usdShade/shader.h"
#include "pxr/usd/usdShade/material.h"
#include "pxr/usd/usdUI/nodeGraphNodeAPI.h"
#include "pxr/usd/usdUtils/flattenLayerStack.h"
#include "pxr/usd/plugin/usdMdl/mdlToUsd.h"
#include <algorithm>
#include <stack>
#include <mi/mdl_sdk.h>
#include <mi/neuraylib/imdl_compiler.h>

PXR_NAMESPACE_OPEN_SCOPE

class Context
{
    std::stack<UsdShadeMaterial> m_usdMaterialStack;
    std::stack<UsdShadeShader> m_usdShaderStack;
    std::map<std::string,
        std::map<std::string, UsdShadeConnectableAPI>> m_shaders;
    SdfPath m_materialsPath;
    SdfPath m_shadersPath;
    UsdStagePtr m_stage;
    std::map<std::string, UsdShadeMaterial> m_materials;

    UsdPrim m_currentModule;

public:
    mi::base::Handle<mi::neuraylib::INeuray> m_neuray;
    mi::base::Handle<mi::neuraylib::ITransaction> m_transaction;
    mi::base::Handle<mi::neuraylib::IMdl_compiler> m_mdl_compiler;
    mi::base::Handle<mi::neuraylib::IMdl_factory> m_mdl_factory;
    std::set<std::string> m_traversed_modules;
    static const std::string g_mdl_dictionary_name;// "mdl"
    static const std::string g_annotations_dictionary_name;// "annotations"

private:
    // Set Custom data
    void setCustomData(const UsdObject & object, const std::map<std::string, VtValue> & customdata) const
    {
        if (!customdata.empty())
        {
            VtDictionary dict(object.GetCustomData());
            for (const auto & anno : customdata)
            {
                dict[anno.first] = anno.second;
            }
            VtDictionary annotations;
            annotations[g_annotations_dictionary_name] = dict;
            VtDictionary mdl;
            mdl[g_mdl_dictionary_name] = annotations;
            object.SetCustomData(mdl);

            // Special case of the "::anno::hidden()" annotation
            std::map<std::string, VtValue>::const_iterator it(customdata.find("::anno::hidden()"));
            if (it != customdata.end() && it->second.CanCast<bool>())
            {
                // If the annotation is found, is a boolean and is true, set the hidden metadata
                object.SetHidden(VtValue(it->second).Cast<bool>() == true);
            }
        }
    }

    // Utility function to process expression list
    template <class USDPRIM>
    void processExpressionList(
        USDPRIM * usdShaderOrMaterial,
        mi::neuraylib::ITransaction* transaction,
        mi::neuraylib::IMdl_factory* mdl_factory,
        const mi::neuraylib::IExpression_list * expressionList, // parameters
        const mi::neuraylib::IAnnotation_list * parm_annotations,  // parameters annotations
        mi::Size parmCount)
    {
        MdlToUsd converter;

        // Keep a list of parameters which have connections between themselves and need post-processing
        typedef std::pair<UsdShadeInput, VtValue> CONNECTION;
        typedef std::vector<CONNECTION> CONNECTION_LIST;
        CONNECTION_LIST parameterConnections;

        // Keep a list of properties which are connected to other shaders and need post-processing
        CONNECTION_LIST callConnections;

        // A first pass creates all input parameters and sets the one which have simple values
        for (mi::Size index = 0; index < parmCount; index++)
        {
            const char * name(expressionList->get_name(index));
            if (!name)
            {
                mi::neuraylib::Mdl::LogError("[processExpressionList] Skip parameter with no name");
                continue;
            }
            std::string parmName(name);
            mi::base::Handle<const mi::neuraylib::IExpression> expression(expressionList->get_expression(index));
            mi::base::Handle<const mi::neuraylib::IType> type(expression->get_type());
            if (!expression || !type)
            {
                mi::neuraylib::Mdl::LogError("[processExpressionList] Skip bogus parameter, no value or type: " + parmName);
                continue;
            }
            TfToken typeOut(converter.ConvertType(type.get()));
            if (typeOut.IsEmpty())
            {
                mi::neuraylib::Mdl::LogError("[processExpressionList] Skip bogus parameter, no value or type" + parmName);
                continue;
            }
            // Convert parameter value to USD
            MdlToUsd::ExtraInfo extra;
            VtValue expressionOut(converter.GetValue(type.get(), expression.get(), typeOut, extra));

            // Create input property
            SdfValueTypeName typeName(SdfSchema::GetInstance().FindType(typeOut.GetString()));
            UsdShadeInput shadeInput(usdShaderOrMaterial->CreateInput(TfToken(name), typeName));

            // Name of annotation, string value for annotation
            std::map<std::string, std::string> metadata;
            MdlToUsd::extractAnnotations(parmName, parm_annotations, metadata);
            for (const auto & anno : metadata)
            {
                shadeInput.SetSdrMetadataByKey(TfToken(anno.first), anno.second);
            }
            std::map<std::string, VtValue> customdata;
            MdlToUsd::extractAnnotations(parmName, parm_annotations, customdata);
            setCustomData(shadeInput.GetAttr(), customdata);
            
            // Add metadata to enum values
            if(extra.m_valueType == MdlToUsd::ExtraInfo::VTT_ENUM)
            {
                // From Pixar doc about Metadata RenderType : https://graphics.pixar.com/usd/docs/api/class_usd_shade_input.html#afeeb2fbd92c5600adc6e8ae9eeda5772
                // ...For example, we set the renderType to "struct" for Inputs that are of renderman custom struct types.
                shadeInput.SetRenderType(TfToken(extra.m_enumSymbol));
                shadeInput.SetSdrMetadataByKey(TfToken("__SDR__enum_value"), extra.m_enumValueName);
                if (!extra.m_enumValueDescription.empty())
                {
                    shadeInput.SetSdrMetadataByKey(TfToken("description"), extra.m_enumValueDescription);
                }
                shadeInput.SetSdrMetadataByKey(TfToken("options"), extra.m_enumValueOptions);
            }
            // Non-square matrices are stored as vectors.
            // Float and Int square matrices stoed as double matrices.
            // Need to store the type of the original matrix to be able to reconstruct it.
            if (extra.m_valueType == MdlToUsd::ExtraInfo::VTT_TYPE_INFO)
            {
                shadeInput.GetAttr().SetCustomDataByKey(TfToken(MdlToUsd::KEY_MDL_TYPE_INFO), VtValue(extra.m_mdlType));
            }

            // We need to avoid connecting incompatible expressions and inputs.
            // Either the input and expression have same name (e.g. bool, int, float, ...)
            // or they appear in the following map
            typedef std::string INPUT_TYPE;
            typedef std::string EXPRESSION_TYPE;
            typedef std::map<INPUT_TYPE, EXPRESSION_TYPE> INPUT_EXPRESSION_MAP;
            INPUT_EXPRESSION_MAP InputExpressionMap =
            {
                {"std::string","string"}
                ,{"VtArray<std::string>","VtArray<string >"}
            };
            bool compatible = (shadeInput.GetTypeName().GetCPPTypeName() == expressionOut.GetTypeName());
            if (!compatible)
            {
                INPUT_EXPRESSION_MAP::iterator it(InputExpressionMap.find(shadeInput.GetTypeName().GetCPPTypeName()));
                if (it != InputExpressionMap.end())
                {
                    compatible = (it->second == expressionOut.GetTypeName());
                }
            }
            if (compatible)
            {
                //mi::neuraylib::Mdl::LogInfo("[processExpressionList] types match for parameter: " + std::string(name) + " (expecting: '" + shadeInput.GetTypeName().GetCPPTypeName() + "'/ got: '" + expressionOut.GetTypeName() + "')");
                shadeInput.Set(expressionOut);
            }
            else
            {
                if (MdlToUsd::ExtraInfo::VTT_NOT_SET == extra.m_valueType)
                {
                    std::stringstream str;
                    str << "[processExpressionList] types do not match for parameter: " << name << " (expecting: '" << shadeInput.GetTypeName().GetCPPTypeName() << "'/ got: '" << expressionOut.GetTypeName() + "')";
                    mi::neuraylib::Mdl::LogError(str.str());
                }
            }
            // Save connections for later
            if (MdlToUsd::ExtraInfo::VTT_PARM == extra.m_valueType)
            {
                parameterConnections.emplace_back(CONNECTION(shadeInput, expressionOut));
            }
            else if (MdlToUsd::ExtraInfo::VTT_CALL == extra.m_valueType)
            {
                callConnections.emplace_back(CONNECTION(shadeInput, expressionOut));
            }
        }

        for (auto & connect : parameterConnections)
        {
            VtValue val(connect.second);
            mi::Size parmIndex(val.Get<mi::Size>());
            const char* parmName(expressionList->get_name(parmIndex));
            UsdShadeInput input(usdShaderOrMaterial->GetInput(TfToken(parmName)));
            if (input.IsDefined())
            {
                connect.first.ConnectToSource(input);
            }
        }

        for (auto & connect : callConnections)
        {
            VtValue val(connect.second);
            std::string callString(val.Get<std::string>());
            if (!callString.empty())
            {
                mi::base::Handle<const mi::neuraylib::IMaterial_instance> matInstance(
                    m_transaction->access<mi::neuraylib::IMaterial_instance>(callString.c_str())
                );
                if (matInstance)
                {
   		    auto namedElement = mi::neuraylib::NamedElement(callString);
                    UsdShadeMaterial material(BeginMaterialInstance(namedElement));
                    EndMaterialInstance();

                    UsdShadeOutput materialCallOutput(material.GetOutput(TfToken("mdl:surface")));
                    if (materialCallOutput.IsDefined())
                    {
                        connect.first.ConnectToSource(materialCallOutput);
                    }
                    else
                    {
                        mi::neuraylib::Mdl::LogError("[processExpressionList] Undefined material output");
                    }
                }
                else
                {
                    mi::base::Handle<const mi::neuraylib::IFunction_call> fCall(
                        m_transaction->access<mi::neuraylib::IFunction_call>(callString.c_str())
                    );
                    if (fCall)
                    {
   		        auto namedElement = mi::neuraylib::NamedElement(callString);
                        UsdShadeMaterial shader(BeginFunctionCall(namedElement));
                        EndFunctionCall();

                        UsdShadeOutput functionCallOutput(shader.GetOutput(TfToken("out")));
                        if (functionCallOutput.IsDefined())
                        {
                            connect.first.ConnectToSource(functionCallOutput);
                        }
                        else
                        {
                            mi::neuraylib::Mdl::LogError("[processExpressionList] Undefined shader output");
                        }
                    }
                }
            }
        }
    }

public:
    Context(const UsdStagePtr& stage, const SdfPath& internalPath)
        : m_stage(stage)
        , m_materialsPath(internalPath.AppendChild(TfToken("Materials")))
        , m_shadersPath(internalPath.AppendChild(TfToken("Shaders")))
    {
        m_neuray = mi::base::make_handle_dup< mi::neuraylib::INeuray>(mi::neuraylib::Mdl::Get().GetNeuray());
        // Access the database and create a transaction.
        mi::base::Handle<mi::neuraylib::IDatabase> database(
            m_neuray->get_api_component<mi::neuraylib::IDatabase>());
        mi::base::Handle<mi::neuraylib::IScope> scope(database->get_global_scope());
        m_transaction = scope->create_transaction();
        m_mdl_compiler = m_neuray->get_api_component<mi::neuraylib::IMdl_compiler>();
        m_mdl_factory = m_neuray->get_api_component<mi::neuraylib::IMdl_factory>();
    }

    ~Context()
    {
        // All transactions need to get committed.
        m_transaction->commit();
    }

    UsdShadeMaterial BeginMaterialInstance(mi::neuraylib::NamedElement& materialInstance);
    void EndMaterialInstance();

    UsdShadeMaterial BeginMaterialDefinition(mi::neuraylib::Material& mdlMaterial);
    void EndMaterialDefinition();

    UsdShadeShader BeginFunctionCall(mi::neuraylib::NamedElement& fCall);
    void EndFunctionCall();

    UsdShadeShader BeginFunctionDefinition(mi::neuraylib::Shader & shader)
    {
        std::string name(shader.GetName());
        if (!name.empty())
        {
            SdfPath path(MdlNameToSdfPath(name));
            if (!m_usdMaterialStack.empty())
            {
                path = path.MakeRelativePath(SdfPath("/"));
                path = m_usdMaterialStack.top().GetPath().AppendPath(path);
            }
            UsdShadeShader newShader(UsdShadeShader::Define(m_stage, path));

            if (newShader.GetPrim().IsValid())
            {
                mi::neuraylib::Mdl::LogVerbose("[BeginFunctionDefinition] Dumping function definition: " + name);

                mi::base::Handle<const mi::neuraylib::IFunction_definition> function_definition(m_transaction->access<mi::neuraylib::IFunction_definition>(name.c_str()));
                
                mi::base::Handle<const mi::neuraylib::IExpression_list> defaults(function_definition->get_defaults());
                mi::Size count(function_definition->get_parameter_count());

                mi::base::Handle<const mi::neuraylib::IAnnotation_list> parm_annotations(function_definition->get_parameter_annotations());

                processExpressionList(&newShader, m_transaction.get(), m_mdl_factory.get(), defaults.get(), parm_annotations.get(), count);

                // Annotations
                mi::base::Handle<const mi::neuraylib::IAnnotation_block> annotations(function_definition->get_annotations());
                if (annotations)
                {
                    std::map<std::string, std::string> metadata;
                    MdlToUsd::extractAnnotations(annotations.get(), metadata);
                    for (auto & anno : metadata)
                    {
                        newShader.SetSdrMetadataByKey(TfToken(anno.first), anno.second);
                    }
                    std::map<std::string, VtValue> customdata;
                    MdlToUsd::extractAnnotations(annotations.get(), customdata);
                    setCustomData(newShader.GetPrim(), customdata);
                }
            }
            return newShader;
        }
        return UsdShadeShader();
    }

    void EndFunctionDefinition()
    {
    }

    bool BeginModule(mi::neuraylib::Module & module);
    void EndModule();
    void SetCurrentModule(const UsdPrim & currentModule)
    {
        m_currentModule = currentModule;
    }
    UsdPrim GetCurrentModule()
    {
        return m_currentModule;
    }

    SdfPath MdlNameToSdfPath(const std::string & name)
    {
        std::string token(name);
        token = MdlUtils::ReplaceAll(token, "::", "/");
        if (token.find_first_of('/') != 0)
        {
            token = "/" + token;
        }
        // remove everything between ()
        size_t b(token.find_first_of('('));
        size_t e(token.find_last_of(')'));
        if (b != token.npos && e != token.npos)
        {
            token = token.substr(0, b) + token.substr(e+1);
        }
        token = MdlUtils::ReplaceAll(token, ".", "_");
        token = MdlUtils::ReplaceAll(token, ",", "_");
        bool valid(SdfPath::IsValidPathString(token));
        if (valid)
        {
            // Test each component of the path otherwise IsValidPathString() fails to detect invalid "operator-"
            std::vector<std::string> tokens = PXR_INTERNAL_NS::TfStringSplit(token, "/");
            for (auto & t : tokens)
            {
                if (!t.empty() && !SdfPath::IsValidIdentifier(t))
                {
                    valid = false;
                }
            }
        }
        if (valid)
        {
            return SdfPath(token);
        }
        else
        {
            mi::neuraylib::Mdl::LogError("[MdlNameToSdfPath] Invalid path: " + token + " (" + name + ")");
        }
        return SdfPath();
    }
};

const std::string Context::g_mdl_dictionary_name("mdl");
const std::string Context::g_annotations_dictionary_name("annotations");

bool Context::BeginModule(mi::neuraylib::Module & module)
{
    std::string moduleName(module.GetModuleName());
    if (moduleName.empty())
    {
        return false;
    }
    if (m_traversed_modules.insert(moduleName).second == false)
    {
        // Module already traversed
        return true;
    }

    // Is module loaded?
    bool loaded(false);
    mi::base::Handle<const mi::neuraylib::IModule> nrmodule;
    {
        nrmodule = m_transaction->access<mi::neuraylib::IModule>(module.GetModuleDBName().c_str());
        loaded = nrmodule.is_valid_interface();
    }

    if (!loaded)
    {
        check_success(m_mdl_compiler->load_module(m_transaction.get(), moduleName.c_str()) >= 0);

        // Access the module by its name. The name to be used here is the MDL name of the module
        // ("example") plus the "mdl::" prefix.
        nrmodule = m_transaction->access<mi::neuraylib::IModule>(module.GetModuleDBName().c_str());
        check_success(nrmodule.is_valid_interface());
    }

    if (!nrmodule)
    {
        return false;
    }

    // Print the module name and the file name it was loaded from.
    const char * mfn(nrmodule->get_filename());
    mi::neuraylib::Mdl::LogVerbose("[BeginModule] Loaded file: " + (mfn ? std::string(mfn) : "<no filename>"));
    const char * mdlfn(nrmodule->get_mdl_name());
    mi::neuraylib::Mdl::LogVerbose("[BeginModule] Found module: " + (mdlfn ? std::string(mdlfn) : "<no filename>"));

    // Dump imported modules.
    mi::Size module_count = nrmodule->get_import_count();
    mi::neuraylib::Mdl::LogVerbose("[BeginModule] The module imports the following modules:");
    for (mi::Size i = 0; i < module_count; i++)
    {
        std::string imported_module(nrmodule->get_import(i));
        mi::neuraylib::Mdl::LogVerbose("[BeginModule] imported module: " + imported_module + " (skipped)");

        // TODO - Do not traverse imported modules
        //BeginModule(mi::neuraylib::Module(imported_module));
        //EndModule();
    }

    // Dump exported types.
    mi::base::Handle<mi::neuraylib::IType_factory> type_factory(m_mdl_factory->create_type_factory(m_transaction.get()));
    mi::base::Handle<const mi::neuraylib::IType_list> types(nrmodule->get_types());
    mi::neuraylib::Mdl::LogVerbose("[BeginModule] The module contains the following types: ");

    for (mi::Size i = 0; i < types->get_size(); ++i)
    {
        mi::base::Handle<const mi::neuraylib::IType> type(types->get_type(i));
        mi::base::Handle<const mi::IString> result(type_factory->dump(type.get(), 1));
        if (result->get_c_str())
        {
#if 0
            // Experimental code...
            mi::neuraylib::Mdl::LogVerbose("[BeginModule] New type: " + std::string(result->get_c_str()));

            if (type->get_kind() == mi::neuraylib::IType::TK_STRUCT)
            {
                mi::base::Handle<const mi::neuraylib::IType_struct> structValue(type->get_interface<mi::neuraylib::IType_struct>());

                if (structValue)
                {
                    std::string name;
                    const char* symbol(structValue->get_symbol());
                    if (symbol)
                    {
                        name = symbol;
                    }
                    if (!name.empty())
                    {
                        SdfPath path(MdlNameToSdfPath(name));
                        if (!m_usdMaterialStack.empty())
                        {
                            path = path.MakeRelativePath(SdfPath("/"));
                            path = m_usdMaterialStack.top().GetPath().AppendPath(path);
                        }
                        UsdShadeShader newShader(UsdShadeShader::Define(m_stage, path));

                        if (newShader.GetPrim().IsValid())
                        {
                            mi::neuraylib::Mdl::LogVerbose("[BeginFunctionDefinition] Dumping function definition: " + name);

                            //mi::base::Handle<const mi::neuraylib::IFunction_definition> function_definition(m_transaction->access<mi::neuraylib::IFunction_definition>(name.c_str()));

                            //mi::base::Handle<const mi::neuraylib::IExpression_list> defaults(function_definition->get_defaults());
                            //mi::Size count(function_definition->get_parameter_count());

                            //processExpressionList(&newShader, m_transaction.get(), m_mdl_factory.get(), defaults.get(), count);
                        }
                    }
                }
            }            
            if (type->get_kind() == mi::neuraylib::IType::TK_ENUM)
            {
                mi::base::Handle<const mi::neuraylib::IType_enum> enumValue(type->get_interface<mi::neuraylib::IType_enum>());
                if (enumValue)
                {
                    std::string name;
                    const char* symbol(enumValue->get_symbol());
                    if (symbol)
                    {
                        name = symbol;
                    }
                    if (!name.empty())
                    {
                        SdfPath path(MdlNameToSdfPath(name));
                        if (!m_usdMaterialStack.empty())
                        {
                            path = path.MakeRelativePath(SdfPath("/"));
                            path = m_usdMaterialStack.top().GetPath().AppendPath(path);
                        }
                        UsdShadeShader newShader(UsdShadeShader::Define(m_stage, path));
                        if (newShader.GetPrim().IsValid())
                        {
                            mi::Size sz(enumValue->get_size());
                            for (mi::Size i = 0; i < sz; i++)
                            {
                                const char* valueName(enumValue->get_value_name(i));
                                if (valueName)
                                {
                                    mi::Sint32 errors;
                                    mi::Sint32 valueCode(enumValue->get_value_code(i, &errors));
                                    if (0 == errors)
                                    {
                                        UsdShadeOutput shadeParm(newShader.CreateOutput(TfToken(valueName), SdfValueTypeNames->Int));
                                        shadeParm.Set(valueCode);
                                    }
                                }
                            }

                        }
                    }
                }
            }
#endif 0
        }
    }

    // Dump exported constants.
    mi::base::Handle<mi::neuraylib::IValue_factory> value_factory(m_mdl_factory->create_value_factory(m_transaction.get()));
    mi::base::Handle<const mi::neuraylib::IValue_list> constants(nrmodule->get_constants());
    mi::neuraylib::Mdl::LogVerbose("[BeginModule] The module contains the following constants: ");
    for (mi::Size i = 0; i < constants->get_size(); ++i)
    {
        mi::base::Handle<const mi::neuraylib::IValue> constant(constants->get_value(i));
        mi::base::Handle<const mi::IString> result(value_factory->dump(constant.get(), 0, 1));
        mi::neuraylib::Mdl::LogVerbose("[BeginModule] " + std::string(result->get_c_str()));
    }

    // Dump function definitions of the module.
    mi::Size function_count = nrmodule->get_function_count();
    mi::neuraylib::Mdl::LogVerbose("[BeginModule] The module contains the following function definitions: ");
    for (mi::Size i = 0; i < function_count; i++)
    {
        const char* function_name = nrmodule->get_function(i);
        if (function_name)
        {
            //if (BeginFunctionDefinition(mi::neuraylib::Shader(function_name)))
            //{
            //    EndFunctionDefinition();
            //}
        }
    }

    // Dump material definitions of the module.
    mi::Size material_count = nrmodule->get_material_count();
    mi::neuraylib::Mdl::LogVerbose("[BeginModule] The module contains the following material definitions: ");
    for (mi::Size i = 0; i < material_count; i++)
    {
        const char* material_name = nrmodule->get_material(i);
        if (material_name)
        {
	    auto material = mi::neuraylib::Material(material_name);
            if (BeginMaterialDefinition(material))
            {
                EndMaterialDefinition();
            }
        }
    }

    // Dump the resources referenced by this module
    mi::neuraylib::Mdl::LogVerbose("[BeginModule] Dumping resources of this module");
    for (mi::Size r = 0, rn = nrmodule->get_resources_count(); r < rn; ++r)
    {
        const char* db_name = nrmodule->get_resource_name(r);
        const char* mdl_file_path = nrmodule->get_resource_mdl_file_path(r);

        if (db_name == NULL)
        {
            // Not necessarily an error. 
            // If module only contains a non - exported material that is not used by any other exported material in the module,
            // the compiler throws it away when and therefore neuray newer sees the texture and thus cannot store it in the database.
            // That s why the database name is null.
            // The function IModule::get_resources_count() however reports all resources that exist in the module,
            // regardless of being used or not.
            mi::neuraylib::Mdl::LogVerbose("[BeginModule] The module contains a resource that could not be resolved");
            mi::neuraylib::Mdl::LogVerbose("[BeginModule] mdl_file_path: " + std::string(mdl_file_path));
            continue;
        }
        mi::neuraylib::Mdl::LogVerbose("[BeginModule] db_name: " + std::string(db_name));
        mi::neuraylib::Mdl::LogVerbose("[BeginModule] mdl_file_path: " + std::string(mdl_file_path));

        const mi::base::Handle<const mi::neuraylib::IType_resource> type(
            nrmodule->get_resource_type(r));
        switch (type->get_kind())
        {
        case mi::neuraylib::IType::TK_TEXTURE:
        {
            const mi::base::Handle<const mi::neuraylib::ITexture> texture(m_transaction->access<mi::neuraylib::ITexture>(db_name));
            if (texture)
            {
                const mi::base::Handle<const mi::neuraylib::IImage> image(m_transaction->access<mi::neuraylib::IImage>(texture->get_image()));

                for (mi::Size t = 0, tn = image->get_uvtile_length(); t < tn; ++t)
                {
                    const char* system_file_path = image->get_filename(
                        static_cast<mi::Uint32>(t));
                    std::stringstream str;
                    str << "resolved_file_path[" << t << "]: " << system_file_path;
                    mi::neuraylib::Mdl::LogVerbose("[BeginModule] " + str.str());
                }
            }
            break;
        }

        case mi::neuraylib::IType::TK_LIGHT_PROFILE:
        {
            const mi::base::Handle<const mi::neuraylib::ILightprofile> light_profile(m_transaction->access<mi::neuraylib::ILightprofile>(db_name));
            if (light_profile)
            {
                const char* system_file_path = light_profile->get_filename();
                mi::neuraylib::Mdl::LogVerbose("[BeginModule] resolved_file_path " + std::string(system_file_path));
            }
            break;
        }

        case mi::neuraylib::IType::TK_BSDF_MEASUREMENT:
        {
            const mi::base::Handle<const mi::neuraylib::IBsdf_measurement> mbsdf(m_transaction->access<mi::neuraylib::IBsdf_measurement>(db_name));
            if (mbsdf)
            {
                const char* system_file_path = mbsdf->get_filename();
                mi::neuraylib::Mdl::LogVerbose("[BeginModule] resolved_file_path " + std::string(system_file_path));
            }
            break;
        }

        default:
            break;
        }
    }

    return true;
}

void Context::EndModule()
{
    SetCurrentModule(UsdPrim());
}

void CreateStandardAttributes(UsdShadeShader & shader, const std::string & module, const std::string & MDLShortName)
{
    // Create attributes
    // Source Asset, e.g.:
    //   uniform token info:implementationSource = "sourceAsset"
    UsdAttribute sourceAsset(shader.GetImplementationSourceAttr());
    sourceAsset.Set(TfToken("sourceAsset"));
    //   uniform asset info : mdl:sourceAsset = @nvidia/test_types.mdl@
    shader.SetSourceAsset(SdfAssetPath(module), TfToken("mdl"));

    // Source Asset subIdentifier, e.g.:
    //   uniform token info:mdl:sourceAsset:subIdentifier = "materialWithEnum"
    if (PXR_VERSION >= 2002)
    {
        shader.SetSourceAssetSubIdentifier(TfToken(MDLShortName), TfToken("mdl"));
    }
    else
    {
    UsdShadeInput matName(shader.GetPrim().CreateAttribute(TfToken("info:mdl:sourceAsset:subIdentifier"), SdfValueTypeNames->Token, false/*custom*/, SdfVariabilityUniform));
    matName.Set(TfToken(MDLShortName));
}
}

UsdShadeMaterial Context::BeginMaterialDefinition(mi::neuraylib::Material & mdlMaterial)
{
    std::string dbName(mdlMaterial.GetName());
    if (!dbName.empty())
    {
        SdfPath path(MdlNameToSdfPath(dbName));
        if (!m_usdMaterialStack.empty())
        {
            path = path.MakeRelativePath(SdfPath("/"));
            path = m_usdMaterialStack.top().GetPath().AppendPath(path);
        }
        UsdShadeMaterial newMaterial(UsdShadeMaterial::Define(m_stage, path));
        if (newMaterial.GetPrim().IsValid())
        {
            mi::neuraylib::Mdl::LogVerbose("[BeginMaterialDefinition] Dumping material definition: " + dbName);
            mi::neuraylib::Mdl::LogVerbose("[BeginMaterialDefinition] Path: " + path.GetString());

            mi::base::Handle<const mi::neuraylib::IMaterial_definition> material_definition(m_transaction->access<mi::neuraylib::IMaterial_definition>(dbName.c_str()));

            if (material_definition)
            {
                const char * mdlName(material_definition->get_mdl_name());
                mi::base::Handle<const mi::neuraylib::IExpression_list> defaults(material_definition->get_defaults());
                mi::Size count(material_definition->get_parameter_count());
                if (mdlName)
                {
                    std::string mname(mdlMaterial.GetShortName());
                    std::string module(mdlMaterial.GetModuleName());
                    std::string newPath(path.AppendChild(TfToken(mname)).GetString());
                    UsdShadeShader surfaceShader(UsdShadeShader::Define(m_stage, SdfPath(newPath)));

                    // Create attributes
                    CreateStandardAttributes(surfaceShader, module, mname);

                    m_usdMaterialStack.push(newMaterial);

                    mi::base::Handle<const mi::neuraylib::IAnnotation_list> parm_annotations(material_definition->get_parameter_annotations());

                    processExpressionList(&newMaterial, m_transaction.get(), m_mdl_factory.get(), defaults.get(), parm_annotations.get(), count);

                    m_usdMaterialStack.pop();

                    // Connect all surface shader parms to the material interface parms
                    std::vector<UsdShadeInput> inputs(newMaterial.GetInputs());
                    for (auto & i : inputs)
                    {
                        UsdShadeInput surfaceInput(surfaceShader.CreateInput(i.GetBaseName(), i.GetTypeName()));
                        surfaceInput.ConnectToSource(i);
                    }

                    // Connect material outputs to surface output
                    UsdShadeOutput shaderOutput(surfaceShader.CreateOutput(TfToken("out"), SdfValueTypeNames->Token));
                    UsdShadeOutput materialOutput(newMaterial.CreateOutput(TfToken("mdl:surface"), SdfValueTypeNames->Token));
                    materialOutput.ConnectToSource(shaderOutput);
                    UsdShadeOutput displacementOutput(newMaterial.CreateOutput(TfToken("mdl:displacement"), SdfValueTypeNames->Token));
                    displacementOutput.ConnectToSource(shaderOutput);
                    UsdShadeOutput volumeOutput(newMaterial.CreateOutput(TfToken("mdl:volume"), SdfValueTypeNames->Token));
                    volumeOutput.ConnectToSource(shaderOutput);

                    // Annotations
                    mi::base::Handle<const mi::neuraylib::IAnnotation_block> annotations(material_definition->get_annotations());
                    std::map<std::string, std::string> metadata;
                    MdlToUsd::extractAnnotations(annotations.get(), metadata);
                    for (auto & anno : metadata)
                    {
                        surfaceShader.SetSdrMetadataByKey(TfToken(anno.first), anno.second);
                    }
                    std::map<std::string, VtValue> customdata;
                    MdlToUsd::extractAnnotations(annotations.get(), customdata);
                    setCustomData(surfaceShader.GetPrim(), customdata);
                }
            }
        }
        return newMaterial;
    }
    return UsdShadeMaterial();
}

void Context::EndMaterialDefinition()
{
}

UsdShadeMaterial Context::BeginMaterialInstance(mi::neuraylib::NamedElement& materialInstance)
{
    mi::base::Handle<const mi::neuraylib::IMaterial_instance> matInstance(
        m_transaction->access<mi::neuraylib::IMaterial_instance>(materialInstance.GetName().c_str())
    );
    if (matInstance)
    {
        const char * mdlName(matInstance->get_mdl_material_definition());
        mi::base::Handle<const mi::neuraylib::IExpression_list> defaults(matInstance->get_arguments());
        mi::Size count(matInstance->get_parameter_count());

        std::string materialDefinition(materialInstance.GetName());
        SdfPath path(MdlNameToSdfPath(materialDefinition));
        if (!m_usdMaterialStack.empty())
        {
            path = path.MakeRelativePath(SdfPath("/"));
            path = m_usdMaterialStack.top().GetPath().AppendPath(path);
        }
        UsdShadeMaterial newMaterial(UsdShadeMaterial::Define(m_stage, path));
        if (newMaterial.GetPrim().IsValid())
        {
            mi::neuraylib::Material mtemp(mdlName);
            std::string mname = mtemp.GetShortName();
            std::string mdlModuleName = mtemp.GetModuleName();

            mi::neuraylib::Mdl::LogVerbose("[BeginMaterialInstance] Dumping material definition: " + materialDefinition);
            mi::neuraylib::Mdl::LogVerbose("[BeginMaterialInstance] Path: " + path.GetString());
            mi::neuraylib::Mdl::LogVerbose("[BeginMaterialInstance] Module: " + mdlModuleName);

            std::string newPath(path.AppendChild(TfToken(mname)).GetString());
            UsdShadeShader surfaceShader(UsdShadeShader::Define(m_stage, SdfPath(newPath)));

            // Create attributes
            CreateStandardAttributes(surfaceShader, mdlModuleName, mname);

            // Create output
            surfaceShader.CreateOutput(TfToken("out"), SdfValueTypeNames->Token);

            m_usdMaterialStack.push(newMaterial);

            mi::base::Handle<const mi::neuraylib::IMaterial_definition> material_definition(m_transaction->access<mi::neuraylib::IMaterial_definition>(mdlName));
            mi::base::Handle<const mi::neuraylib::IAnnotation_list> parm_annotations;
            mi::base::Handle<const mi::neuraylib::IAnnotation_block> annotations;
            if (material_definition)
            {
                parm_annotations = mi::base::Handle<const mi::neuraylib::IAnnotation_list>(material_definition->get_parameter_annotations());
                annotations = mi::base::Handle<const mi::neuraylib::IAnnotation_block>(material_definition->get_annotations());
            }
            const mi::neuraylib::IAnnotation_list * parm_anno = parm_annotations ? parm_annotations.get() : NULL;
            processExpressionList(&newMaterial, m_transaction.get(), m_mdl_factory.get(), defaults.get(), parm_anno, count);

            m_usdMaterialStack.pop();

            // Connect all surface shader parms to the material interface parms
            std::vector<UsdShadeInput> inputs(newMaterial.GetInputs());
            for (auto & i : inputs)
            {
                UsdShadeInput surfaceInput(surfaceShader.CreateInput(i.GetBaseName(), i.GetTypeName()));
                surfaceInput.ConnectToSource(i);
            }
            
            // Connect material outputs to surface output
            UsdShadeOutput shaderOutput(surfaceShader.CreateOutput(TfToken("out"), SdfValueTypeNames->Token));
            UsdShadeOutput materialOutput(newMaterial.CreateOutput(TfToken("mdl:surface"), SdfValueTypeNames->Token));
            materialOutput.ConnectToSource(shaderOutput);
            UsdShadeOutput displacementOutput(newMaterial.CreateOutput(TfToken("mdl:displacement"), SdfValueTypeNames->Token));
            displacementOutput.ConnectToSource(shaderOutput);
            UsdShadeOutput volumeOutput(newMaterial.CreateOutput(TfToken("mdl:volume"), SdfValueTypeNames->Token));
            volumeOutput.ConnectToSource(shaderOutput);

            // Annotations
            if (annotations)
            {
                std::map<std::string, std::string> metadata;
                MdlToUsd::extractAnnotations(annotations.get(), metadata);
                for (auto & anno : metadata)
                {
                    surfaceShader.SetSdrMetadataByKey(TfToken(anno.first), anno.second);
                }
                std::map<std::string, VtValue> customdata;
                MdlToUsd::extractAnnotations(annotations.get(), customdata);
                setCustomData(surfaceShader.GetPrim(), customdata);
            }

            return newMaterial;
        }
    }
    return UsdShadeMaterial();
}

void Context::EndMaterialInstance()
{
}

UsdShadeShader Context::BeginFunctionCall(mi::neuraylib::NamedElement& fCall)
{
    mi::base::Handle<const mi::neuraylib::IFunction_call> fct(
        m_transaction->access<mi::neuraylib::IFunction_call>(fCall.GetName().c_str())
    );
    if (fct)
    {
        std::string functionDefinition(fct->get_mdl_function_definition());
        std::string moduleName;
        mi::base::Handle<const mi::neuraylib::IFunction_definition> function_definition(m_transaction->access<mi::neuraylib::IFunction_definition>(functionDefinition.c_str()));
        if (function_definition)
        {
            const char * m(function_definition->get_module());
            moduleName = (m ? m : "");
        }

        std::string fCallName(fCall.GetName());
        SdfPath path(MdlNameToSdfPath(fCallName));
        if (!m_usdMaterialStack.empty())
        {
            path = path.MakeRelativePath(SdfPath("/"));
            path = m_usdMaterialStack.top().GetPath().AppendPath(path);
        }
        UsdShadeShader newShader(UsdShadeShader::Define(m_stage, path));
        if (newShader.GetPrim().IsValid())
        {
            mi::neuraylib::Mdl::LogVerbose("[BeginFunctionCall] Dumping function call: " + fCallName);
            mi::neuraylib::Mdl::LogVerbose("[BeginFunctionCall] Path: " + path.GetString());

            // Create output
            newShader.CreateOutput(TfToken("out"), SdfValueTypeNames->Token);

            // Create attributes
            if (moduleName.empty())
            {
                // Source asset
                UsdAttribute sourceAsset(newShader.GetImplementationSourceAttr());
                sourceAsset.Set(TfToken("id"));

                // Source info:id
                newShader.CreateIdAttr(VtValue(TfToken(functionDefinition)));
            }
            else
            {
                // Create attributes
                mi::neuraylib::Function ftemp(functionDefinition);
                std::string fname(ftemp.GetShortName());
                CreateStandardAttributes(newShader, moduleName, fname);
            }

            mi::base::Handle<const mi::neuraylib::IExpression_list> defaults(fct->get_arguments());
            mi::Size count(fct->get_parameter_count());

            m_usdShaderStack.push(newShader);

            mi::base::Handle<const mi::neuraylib::IAnnotation_list> parm_annotations;
            if (function_definition)
            {
                parm_annotations = mi::base::Handle<const mi::neuraylib::IAnnotation_list>(function_definition->get_parameter_annotations());
            }
            const mi::neuraylib::IAnnotation_list * parm_annotations_ptr = parm_annotations ? parm_annotations.get() : NULL;
            processExpressionList(&newShader, m_transaction.get(), m_mdl_factory.get(), defaults.get(), parm_annotations_ptr, count);

            if (function_definition)
            {
                // Annotations
                mi::base::Handle<const mi::neuraylib::IAnnotation_block> annotations(function_definition->get_annotations());
                if (annotations)
                {
                    std::map<std::string, std::string> metadata;
                    MdlToUsd::extractAnnotations(annotations.get(), metadata);
                    for (auto & anno : metadata)
                    {
                        newShader.SetSdrMetadataByKey(TfToken(anno.first), anno.second);
                    }
                    std::map<std::string, VtValue> customdata;
                    MdlToUsd::extractAnnotations(annotations.get(), customdata);
                    setCustomData(newShader.GetPrim(), customdata);
                }
            }

            m_usdShaderStack.pop();
        }
        return newShader;
    }
    return UsdShadeShader();
}

void Context::EndFunctionCall()
{
}

void
UsdMdlRead(
    mi::neuraylib::Module& mdlModule,
    const UsdStagePtr& stage,
    const SdfPath& internalPath,
    const SdfPath& externalPath)
{
    Context context(stage, internalPath);
    context.BeginModule(mdlModule);
    context.EndModule();
}

PXR_NAMESPACE_CLOSE_SCOPE
