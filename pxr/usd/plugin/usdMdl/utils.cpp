/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#include "pxr/pxr.h"
#include "pxr/usd/plugin/usdMdl/utils.h"
#include "pxr/usd/plugin/usdMdl/neuray.h"
#include "pxr/base/tf/pathUtils.h"

using std::string;
using mi::neuraylib::Mdl;
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
using mi::neuraylib::IValue_bool;
using mi::neuraylib::IValue_vector;
using mi::neuraylib::IValue_atomic;

PXR_NAMESPACE_OPEN_SCOPE

void MdlUtils::DumpTokens(const TfTokenVector & vec)
{
    TfTokenVector::const_iterator it;
    for (auto & item : vec)
    {
        Mdl::LogInfo("[DumpTokens] " + item.GetString());
    }
}

std::string MdlUtils::Uppercase(const std::string& in)
{
    std::string out(in);
#if defined(ARCH_OS_WINDOWS)
    std::transform(out.begin(), out.end(), out.begin(), ::toupper);
#endif
    return out;
}

std::string MdlUtils::ReplaceAll(std::string str, const std::string& from, const std::string& to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

std::string MdlUtils::GetModuleNameFromModuleFilename(const std::string & filename)
{
    mi::base::Handle<mi::neuraylib::IMdl_compiler> compiler(
        mi::neuraylib::Mdl::Get().GetNeuray()->get_api_component<mi::neuraylib::IMdl_compiler>()
    );
    return GetModuleNameFromModuleFilename(filename, compiler.get());
}

std::string MdlUtils::GetModuleNameFromModuleFilename(const std::string & filename, mi::neuraylib::IMdl_compiler *compiler)
{
    std::string fn(filename);
    fn = ReplaceAll(fn, "\\", "/"); // Use consistent separator
    fn = ReplaceAll(fn, ".mdl", ""); // Remove extension
    if (TfIsRelativePath(filename))
    {
        // The filename might be relative, in that case just transform to MDL name
        fn = ReplaceAll(fn, "/", "::");
        if (fn.find("::") != 0)
        {
            // Must start with leading "::"
            fn = "::" + fn;
        }
        return fn;
    }
    assert(compiler);
    std::vector<string> modulePaths;
    for (mi::Size i = 0; i < compiler->get_module_paths_length(); i++)
    {
        std::string path(compiler->get_module_path(i)->get_c_str());
        path = ReplaceAll(path, "\\", "/"); // Use consistent separator
        modulePaths.emplace_back(path);
        size_t index(Uppercase(fn).find(Uppercase(path)));
        if (index == 0)
        {
            fn = fn.substr(path.size());
            fn = ReplaceAll(fn, "/", "::");
            if (fn.find("::") != 0)
            {
                // Must start with leading "::"
                fn = "::" + fn;
            }
            return fn;
        }
    }

    mi::neuraylib::Mdl::LogError("[moduleFilenameToModuleName] Can not find module for: " + filename);
    if (modulePaths.empty())
    {
        mi::neuraylib::Mdl::LogError("[moduleFilenameToModuleName] No MDL module paths set");

    }
    for (auto & p : modulePaths)
    {
        mi::neuraylib::Mdl::LogError("[moduleFilenameToModuleName] Tried MDL module path: " + p);
    }
    return "";
}

std::string MdlUtils::GetDBName(const std::string & name)
{
    std::string DBName(name);
    if (DBName.find("mdl::") == 0)
    {
        return DBName;
    }
    if (DBName.find("::") == 0)
    {
        return "mdl" + DBName;
    }
    return "mdl::" + DBName;
}

std::string MdlUtils::GetUniqueName(mi::neuraylib::ITransaction* transaction, const std::string& prefixIn)
{
    static mi::Uint32 s_uniqueID = 0;
    std::string prefix(prefixIn);
    if (prefix.empty())
    {
        prefix = "elt";
    }
    mi::base::Handle<const mi::base::IInterface> interf(transaction->access<mi::base::IInterface>(prefix.c_str()));
    if (!interf)
    {
        return prefix;
    }
    std::stringstream oss;
    while(true)
    {
        oss.str("");
        oss << prefix << "_" << s_uniqueID++;
        mi::base::Handle<const mi::base::IInterface> interf(transaction->access<mi::base::IInterface>(oss.str().c_str()));
        if (! interf)
        {
            return oss.str();
        }
    }
    return "";
}

bool MdlUtils::GetUsdToMdlSaveToModule()
{
    // Return true for testing the USD->MDL conversion
    // return true;
    return false;
}

void MdlUtils::DumpAttribute(UsdAttribute & attribute)
{
    Mdl::LogInfo("[DumpAttribute] attribute -------> " + std::string(attribute.GetName()));

    SdfValueTypeName tn(attribute.GetTypeName());
    Mdl::LogInfo("[DumpAttribute] type: " + std::string(tn.GetAsToken().GetText()));

    SdfVariability v(attribute.GetVariability());
        //SdfVariabilityVarying,
        //SdfVariabilityUniform,
        //SdfVariabilityConfig,

    Mdl::LogInfo("[DumpAttribute] is custom: " + (attribute.IsCustom() ? std::string("yes") : std::string("no")));
    Mdl::LogInfo("[DumpAttribute] is defined: " + (attribute.IsDefined() ? std::string("yes") : std::string("no")));
    Mdl::LogInfo("[DumpAttribute] is authored: " + (attribute.IsAuthored() ? std::string("yes") : std::string("no")));
    Mdl::LogInfo("[DumpAttribute] role name: " + std::string(attribute.GetRoleName().GetText()));
    Mdl::LogInfo("[DumpAttribute] has value: " + (attribute.HasValue()? std::string("yes"): std::string("no")));
    Mdl::LogInfo("[DumpAttribute] has authored value opinion: " + (attribute.HasAuthoredValueOpinion() ? std::string("yes") : std::string("no")));
    //CRASH Mdl::LogInfo("[DumpAttribute] has fallback value: " + (attribute.HasFallbackValue() ? std::string("yes") : std::string("no")));

    //bool Get(VtValue* value, UsdTimeCode time = UsdTimeCode::Default()) const;
    SdfPathVector sources;
    Mdl::LogInfo("[DumpAttribute] has connections: " + (attribute.GetConnections(&sources) && !sources.empty() ? std::string("yes") : std::string("no")));
    for (auto & p : sources)
    {    
        Mdl::LogInfo("[DumpAttribute] \tconnections: " + (p.GetString()));
    }
}

void MdlUtils::DumpInput(UsdShadeInput & input)
{
    Mdl::LogInfo("[DumpInput] Input fullname: " + std::string(input.GetFullName().GetString()));
    Mdl::LogInfo("[DumpInput] Input basename: " + std::string(input.GetBaseName().GetString()));

    SdfValueTypeName tn(input.GetTypeName());
    Mdl::LogInfo("[DumpInput] type: " + std::string(tn.GetAsToken().GetText()));

    NdrTokenMap metadata(input.GetSdrMetadata());
    for (auto & d : metadata)
    {
        Mdl::LogInfo("[DumpInput] metadata: " + d.first.GetString() + " " + d.second);
    }
    Mdl::LogInfo("[DumpInput] is defined: " + (input.IsDefined() ? std::string("yes") : std::string("no")));
}

PXR_NAMESPACE_CLOSE_SCOPE
