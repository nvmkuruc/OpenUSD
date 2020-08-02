/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#pragma once
#include <mi/mdl_sdk.h>
#include "pxr/usd/sdf/types.h"
#include "pxr/usd/usdShade/material.h"
#include <string>

PXR_NAMESPACE_OPEN_SCOPE

/// Helper utility
namespace MdlUtils
{
    void DumpTokens(const TfTokenVector & vec);
    void DumpAttribute(UsdAttribute & attribute);
    void DumpInput(UsdShadeInput & input);

    /// Return the qualified name from a module filename.
    /// Based on the current MDL search paths.
    /// e.g:
    ///     C:\ProgramData\NVIDIA Corporation\mdl\nvidia\core_definitions.mdl
    /// will transform to
    ///     ::nvidia::core_definitions
    /// If C:\ProgramData\NVIDIA Corporation\mdl is in the MDL search paths
    std::string GetModuleNameFromModuleFilename(const std::string & filename);
    std::string GetModuleNameFromModuleFilename(const std::string & filename, mi::neuraylib::IMdl_compiler *compiler);

    std::string Uppercase(const std::string& in);

    /// Replace all occurences of 'from' string to 'to' in the input 'str'
    std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);

    /// Just ensure the name starts with "mdl::"
    std::string GetDBName(const std::string & name);

    /// Return an MDL element name which does not exist yet
    std::string GetUniqueName(mi::neuraylib::ITransaction* transaction, const std::string & prefix = std::string());

    /// Testing only: Save MDL converted material to module
    bool GetUsdToMdlSaveToModule();

} // namespace MdlUtils

PXR_NAMESPACE_CLOSE_SCOPE

