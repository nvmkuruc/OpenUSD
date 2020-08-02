/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#include "pxr/pxr.h"
#include "pxr/usd/plugin/usdMdl/fileFormat.h"
#include "pxr/usd/plugin/usdMdl/neuray.h"
#include "pxr/usd/plugin/usdMdl/reader.h"
#include "pxr/usd/plugin/usdMdl/utils.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usd/usdaFileFormat.h"
#include "pxr/base/tf/pathUtils.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PUBLIC_TOKENS(
    UsdMdlFileFormatTokens, 
    USDMDL_FILE_FORMAT_TOKENS);

TF_REGISTRY_FUNCTION(TfType)
{
    SDF_DEFINE_FILE_FORMAT(UsdMdlFileFormat, SdfFileFormat);
}


UsdMdlFileFormat::UsdMdlFileFormat()
    : SdfFileFormat(
        UsdMdlFileFormatTokens->Id,
        UsdMdlFileFormatTokens->Version,
        UsdMdlFileFormatTokens->Target,
        UsdMdlFileFormatTokens->Id)
{
}

UsdMdlFileFormat::~UsdMdlFileFormat()
{
}

SdfAbstractDataRefPtr
UsdMdlFileFormat::InitData(const FileFormatArguments& args) const
{
    return SdfFileFormat::InitData(args);
}

bool
UsdMdlFileFormat::CanRead(const std::string& filePath) const
{
    const auto extension = TfGetExtension(filePath);
    if (extension != this->GetFormatId()) {
        return false;
    }

    return true;
}

bool
UsdMdlFileFormat::Read(
    SdfLayer* layer,
    const std::string& resolvedPath,
    bool metadataOnly) const
{
    TRACE_FUNCTION();

    auto stage = UsdStage::CreateInMemory();

    std::string moduleName(MdlUtils::GetModuleNameFromModuleFilename(resolvedPath));
    
    if (!moduleName.empty())
    {
        mi::neuraylib::Module mdlModule(moduleName);
        UsdMdlRead(mdlModule, stage);
    }

    layer->TransferContent(stage->GetRootLayer());
    return true;
}

bool
UsdMdlFileFormat::WriteToFile(
    const SdfLayer& layer,
    const std::string& filePath,
    const std::string& comment,
    const FileFormatArguments& args) const
{
    return false;
}

bool
UsdMdlFileFormat::WriteToString(
    const SdfLayer& layer,
    std::string* str,
    const std::string& comment) const
{
    return SdfFileFormat::FindById(UsdUsdaFileFormatTokens->Id)->
        WriteToString(layer, str, comment);
}

bool
UsdMdlFileFormat::WriteToStream(
    const SdfSpecHandle &spec,
    std::ostream& out,
    size_t indent) const
{
    return SdfFileFormat::FindById(UsdUsdaFileFormatTokens->Id)->
        WriteToStream(spec, out, indent);
}

PXR_NAMESPACE_CLOSE_SCOPE
