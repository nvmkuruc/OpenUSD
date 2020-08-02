/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#include "pxr/pxr.h"

#include "pxr/usd/plugin/usdMdl/backdoor.h"
#include "pxr/usd/plugin/usdMdl/distiller.h"
#include "pxr/usd/plugin/usdMdl/usdToMdl.h"
#include "pxr/usd/plugin/usdMdl/utils.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/base/tf/diagnostic.h"
#include "pxr/base/tf/fileUtils.h"
#include "pxr/usd/usdShade/material.h"

using mi::neuraylib::Mdl;

PXR_NAMESPACE_OPEN_SCOPE

UsdStageRefPtr UsdMdl_TestDistill(const std::string& materialQualifiedName)
{
    Distiller distiller;
    distiller.SetMaterialName(materialQualifiedName);
    distiller.Distill();

    std::string identifier("preview.usda");
    UsdStageRefPtr stage(NULL);
    if (SdfLayer::Find(identifier))
    { 
        stage = UsdStage::Open(identifier);
    }
    if (!stage)
    {
        stage = UsdStage::CreateNew(identifier);
    }

    std::string sphere("sphere.usda");
    if (TfPathExists(sphere))
    {
        stage->GetRootLayer()->Import(sphere);
    }
    else
    {
        Mdl::LogError("[UsdMdl_TestDistill] missing file: " + std::string(sphere));
    }

    SdfPath path("/mat");
    UsdShadeMaterial material(UsdShadeMaterial::Define(stage, path));


    UsdShadeShader previewShader(UsdShadeShader::Define(stage, path.AppendChild(TfToken("shader"))));

    previewShader.CreateIdAttr(VtValue(TfToken("UsdPreviewSurface")));
    auto shaderSurface(previewShader.CreateOutput(TfToken("surface"), SdfValueTypeNames->Token));
    auto shaderDisplacement(previewShader.CreateOutput(TfToken("displacement"), SdfValueTypeNames->Token));

    auto materialSurface(material.CreateOutput(TfToken("surface"), SdfValueTypeNames->Token));
    materialSurface.ConnectToSource(shaderSurface);

    auto materialDisplacement(material.CreateOutput(TfToken("displacement"), SdfValueTypeNames->Token));
    materialDisplacement.ConnectToSource(shaderDisplacement);

    //def Shader "PrimvarSt1"
    //{
    //    uniform token info : id = "UsdPrimvarReader_float2"
    //        token inputs : varname = "st"
    //        float2 outputs : result
    //}
    UsdShadeShader stShader(UsdShadeShader::Define(stage, path.AppendChild(TfToken("PrimvarSt1"))));
    stShader.CreateIdAttr(VtValue(TfToken("UsdPrimvarReader_float2")));
    auto varname(stShader.CreateInput(TfToken("varname"), SdfValueTypeNames->Token));
    varname.Set(TfToken("st"));
    auto stResult(stShader.CreateOutput(TfToken("result"), SdfValueTypeNames->Float2));

    Distiller::InputParm p;
    if (distiller.GetParameters(Distiller::diffuseColor, p))
    {
        auto diffuseColor(previewShader.CreateInput(TfToken("diffuseColor"), SdfValueTypeNames->Color3f));

        float color[3];
        std::string texture;
        if (p.IsTexture(texture))
        {
            UsdShadeShader baseColorShader(UsdShadeShader::Define(stage, path.AppendChild(TfToken("baseColorTex"))));
            baseColorShader.CreateIdAttr(VtValue(TfToken("UsdUVTexture")));
            auto fallback(baseColorShader.CreateInput(TfToken("fallback"), SdfValueTypeNames->Float4));
            fallback.Set(VtValue(GfVec4f(0, 1, 0, 1)));

            auto file(baseColorShader.CreateInput(TfToken("file"), SdfValueTypeNames->Asset));
            file.Set(SdfAssetPath(texture));

            auto st(baseColorShader.CreateInput(TfToken("st"), SdfValueTypeNames->Float2));
            st.ConnectToSource(stResult);

            auto shaderOutputRgb(baseColorShader.CreateOutput(TfToken("rgb"), SdfValueTypeNames->Float3));

            //asset inputs : file = @example_distilling1-base_color.png@
            diffuseColor.ConnectToSource(shaderOutputRgb);
        }
        else if (p.IsConstantColor(color))
        {
            diffuseColor.Set(VtValue(GfVec3f(color[0], color[1], color[2])));
        }
    }
    if (distiller.GetParameters(Distiller::emissiveColor, p))
    {
        auto emissiveColor(previewShader.CreateInput(TfToken("emissiveColor"), SdfValueTypeNames->Color3f));

        float color[3];
        std::string texture;
        if (p.IsTexture(texture))
        {
            UsdShadeShader emissiveColorShader(UsdShadeShader::Define(stage, path.AppendChild(TfToken("emissiveColorTex"))));
            emissiveColorShader.CreateIdAttr(VtValue(TfToken("UsdUVTexture")));

            auto file(emissiveColorShader.CreateInput(TfToken("file"), SdfValueTypeNames->Asset));
            file.Set(SdfAssetPath(texture));

            auto st(emissiveColorShader.CreateInput(TfToken("st"), SdfValueTypeNames->Float2));
            st.ConnectToSource(stResult);

            auto shaderOutputRgb(emissiveColorShader.CreateOutput(TfToken("rgb"), SdfValueTypeNames->Float3));

            emissiveColor.ConnectToSource(shaderOutputRgb);
        }
        else if (p.IsConstantColor(color))
        {
            emissiveColor.Set(VtValue(GfVec3f(color[0], color[1], color[2])));
        }
    }
    if (distiller.GetParameters(Distiller::clearcoatRoughness, p))
    {
        auto clearcoatRoughness(previewShader.CreateInput(TfToken("clearcoatRoughness"), SdfValueTypeNames->Float));
        float value;
        std::string texture;
        if (p.IsTexture(texture))
        {
            //def Shader "clearcoatTex"
            //{
            //    uniform token info : id = "UsdUVTexture"
            //        float4 inputs : fallback = (.5, .5, .5, .5)
            //        asset inputs : file = @mat_clearcoat.png@
            //        float2 inputs : st.connect = < / mat / PrimvarSt1.outputs : result>
            //        float outputs : r
            //        float outputs : g
            //}
            UsdShadeShader clearcoatTexShader(UsdShadeShader::Define(stage, path.AppendChild(TfToken("clearcoatTex"))));
            clearcoatTexShader.CreateIdAttr(VtValue(TfToken("UsdUVTexture")));
            auto fallback(clearcoatTexShader.CreateInput(TfToken("fallback"), SdfValueTypeNames->Float4));
            fallback.Set(VtValue(GfVec4f(0.5, 0.5, 0.5, 0.5)));

            auto file(clearcoatTexShader.CreateInput(TfToken("file"), SdfValueTypeNames->Asset));
            file.Set(SdfAssetPath(texture));

            auto st(clearcoatTexShader.CreateInput(TfToken("st"), SdfValueTypeNames->Float2));
            st.ConnectToSource(stResult);

            auto shaderOutput(clearcoatTexShader.CreateOutput(TfToken("r"), SdfValueTypeNames->Float));

            clearcoatRoughness.ConnectToSource(shaderOutput);
        }
        else if (p.IsFloat(value))
        {
            clearcoatRoughness.Set(VtValue(value));
        }
    }
    if (distiller.GetParameters(Distiller::clearcoat, p))
    {
        auto clearcoat(previewShader.CreateInput(TfToken("clearcoat"), SdfValueTypeNames->Float));
        float value;
        std::string texture;
        if (p.IsTexture(texture))
        {
            UsdShadeShader clearcoatTexShader(UsdShadeShader::Define(stage, path.AppendChild(TfToken("clearcoatTex"))));
            clearcoatTexShader.CreateIdAttr(VtValue(TfToken("UsdUVTexture")));
            auto fallback(clearcoatTexShader.CreateInput(TfToken("fallback"), SdfValueTypeNames->Float4));
            fallback.Set(VtValue(GfVec4f(0.5, 0.5, 0.5, 0.5)));

            auto file(clearcoatTexShader.CreateInput(TfToken("file"), SdfValueTypeNames->Asset));
            file.Set(SdfAssetPath(texture));

            auto st(clearcoatTexShader.CreateInput(TfToken("st"), SdfValueTypeNames->Float2));
            st.ConnectToSource(stResult);

            auto shaderOutput(clearcoatTexShader.CreateOutput(TfToken("r"), SdfValueTypeNames->Float));

            clearcoat.ConnectToSource(shaderOutput);
        }
        else if (p.IsFloat(value))
        {
            clearcoat.Set(VtValue(value));
        }
    }
    if (distiller.GetParameters(Distiller::metallic, p))
    {
        //def Shader "metallicTex"
        //{
        //    uniform token info : id = "UsdUVTexture"
        //        float4 inputs : fallback = (0.3, 0, 0, 1)
        //        asset inputs : file = @example_distilling1-metallic.png@
        //        float2 inputs : st.connect = < / mat / PrimvarSt1.outputs : result>
        //        float outputs : r
        //}
        auto metallic(previewShader.CreateInput(TfToken("metallic"), SdfValueTypeNames->Float));
        float value;
        std::string texture;
        if (p.IsTexture(texture))
        {
            UsdShadeShader metallicTexShader(UsdShadeShader::Define(stage, path.AppendChild(TfToken("metallicTex"))));
            metallicTexShader.CreateIdAttr(VtValue(TfToken("UsdUVTexture")));
            auto fallback(metallicTexShader.CreateInput(TfToken("fallback"), SdfValueTypeNames->Float4));
            fallback.Set(VtValue(GfVec4f(0.3, 0, 0, 1)));

            auto file(metallicTexShader.CreateInput(TfToken("file"), SdfValueTypeNames->Asset));
            file.Set(SdfAssetPath(texture));

            auto st(metallicTexShader.CreateInput(TfToken("st"), SdfValueTypeNames->Float2));
            st.ConnectToSource(stResult);

            auto shaderOutput(metallicTexShader.CreateOutput(TfToken("r"), SdfValueTypeNames->Float));

            metallic.ConnectToSource(shaderOutput);
        }
        else if (p.IsFloat(value))
        {
            metallic.Set(VtValue(value));
        }
    }
    if (distiller.GetParameters(Distiller::normal, p))
    {
        auto normal(previewShader.CreateInput(TfToken("normal"), SdfValueTypeNames->Normal3f));

        float normalValue[3];
        std::string texture;
        if (p.IsTexture(texture))
        {
            //            def Shader "normalTex"
            //            {
            //                uniform token info : id = "UsdUVTexture"
            //                asset inputs:file = @example_distilling1-normal.png@
            //                    float2 inputs : st.connect = < / mat / PrimvarSt.outputs : result>
            //                    float4 inputs : scale = (.1, .1, .1, .1)
            //                    float4 inputs : bias = (-1.0, -1.0, -1.0, -1.0)
            //                    float3f outputs : rgb
            //            }
            UsdShadeShader normalShader(UsdShadeShader::Define(stage, path.AppendChild(TfToken("normalTex"))));
            normalShader.CreateIdAttr(VtValue(TfToken("UsdUVTexture")));

            auto file(normalShader.CreateInput(TfToken("file"), SdfValueTypeNames->Asset));
            file.Set(SdfAssetPath(texture));

            auto st(normalShader.CreateInput(TfToken("st"), SdfValueTypeNames->Float2));
            st.ConnectToSource(stResult);

            auto scale(normalShader.CreateInput(TfToken("scale"), SdfValueTypeNames->Float4));
            scale.Set(VtValue(GfVec4f(.1, .1, .1, .1)));

            auto bias(normalShader.CreateInput(TfToken("bias"), SdfValueTypeNames->Float4));
            bias.Set(VtValue(GfVec4f(-1.0, -1.0, -1.0, -1.0)));

            auto shaderOutputRgb(normalShader.CreateOutput(TfToken("rgb"), SdfValueTypeNames->Normal3f));

            //asset inputs : file = @example_distilling1-base_color.png@
            normal.ConnectToSource(shaderOutputRgb);
        }
        else if (p.IsFloatArray(normalValue))
        {
            normal.Set(VtValue(GfVec3f(normalValue[0], normalValue[1], normalValue[2])));
        }
    }
    if (distiller.GetParameters(Distiller::opacity, p))
    {
        auto opacity(previewShader.CreateInput(TfToken("opacity"), SdfValueTypeNames->Float));
        float value;
        std::string texture;
        if (p.IsTexture(texture))
        {
            UsdShadeShader opacityTexShader(UsdShadeShader::Define(stage, path.AppendChild(TfToken("opacityTex"))));
            opacityTexShader.CreateIdAttr(VtValue(TfToken("UsdUVTexture")));

            auto file(opacityTexShader.CreateInput(TfToken("file"), SdfValueTypeNames->Asset));
            file.Set(SdfAssetPath(texture));

            auto st(opacityTexShader.CreateInput(TfToken("st"), SdfValueTypeNames->Float2));
            st.ConnectToSource(stResult);

            auto shaderOutput(opacityTexShader.CreateOutput(TfToken("r"), SdfValueTypeNames->Float));

            opacity.ConnectToSource(shaderOutput);
        }
        else if (p.IsFloat(value))
        {
            opacity.Set(VtValue(value));
        }
    }
    if (distiller.GetParameters(Distiller::roughness, p))
    {
        auto roughness(previewShader.CreateInput(TfToken("roughness"), SdfValueTypeNames->Float));
        float value;
        std::string texture;
        if (p.IsTexture(texture))
        {
            UsdShadeShader roughnessTexShader(UsdShadeShader::Define(stage, path.AppendChild(TfToken("roughnessTex"))));
            roughnessTexShader.CreateIdAttr(VtValue(TfToken("UsdUVTexture")));

            auto file(roughnessTexShader.CreateInput(TfToken("file"), SdfValueTypeNames->Asset));
            file.Set(SdfAssetPath(texture));

            auto st(roughnessTexShader.CreateInput(TfToken("st"), SdfValueTypeNames->Float2));
            st.ConnectToSource(stResult);

            auto shaderOutput(roughnessTexShader.CreateOutput(TfToken("r"), SdfValueTypeNames->Float));

            roughness.ConnectToSource(shaderOutput);
        }
        else if (p.IsFloat(value))
        {
            roughness.Set(VtValue(value));
        }
    }
    if (distiller.GetParameters(Distiller::metallic, p))
    {
        auto metallic(previewShader.CreateInput(TfToken("metallic"), SdfValueTypeNames->Float));
        float value;
        std::string texture;
        if (p.IsTexture(texture))
        {
            UsdShadeShader metallicTexShader(UsdShadeShader::Define(stage, path.AppendChild(TfToken("metallicTex"))));
            metallicTexShader.CreateIdAttr(VtValue(TfToken("UsdUVTexture")));

            auto file(metallicTexShader.CreateInput(TfToken("file"), SdfValueTypeNames->Asset));
            file.Set(SdfAssetPath(texture));

            auto st(metallicTexShader.CreateInput(TfToken("st"), SdfValueTypeNames->Float2));
            st.ConnectToSource(stResult);

            auto shaderOutput(metallicTexShader.CreateOutput(TfToken("r"), SdfValueTypeNames->Float));

            metallic.ConnectToSource(shaderOutput);
        }
        else if (p.IsFloat(value))
        {
            metallic.Set(VtValue(value));
        }
    }
    if (distiller.GetParameters(Distiller::ior, p))
    {
        auto ior(previewShader.CreateInput(TfToken("ior"), SdfValueTypeNames->Float));
        float value;
        if (p.IsFloat(value))
        {
            ior.Set(VtValue(value));
        }
    }

    stage->GetRootLayer()->Save();

    return stage;
}

bool UsdMdl_SetVerbosity(int verbosity)
{
    mi::neuraylib::Mdl::SetVerbosity(verbosity);
    return true;
}

bool UsdMdl_CreateMdl(const std::string& stageName, const std::string& primPath)
{
    Mdl::LogInfo("[UsdMdl_CreateMdl] prim: " + primPath);

    UsdStageRefPtr stage = UsdStage::Open(stageName);
    if (stage == NULL)
    {
        Mdl::LogError("[UsdMdl_CreateMdl] invalid stage: " + stageName);
        return false;
    }

    Mdl::LogInfo("[UsdMdl_CreateMdl] stage opened");

    UsdPrim prim(stage->GetPrimAtPath(SdfPath(primPath)));

    if (!prim.IsValid())
    {
        Mdl::LogError("[UsdMdl_CreateMdl] invalid prim: " + primPath);
        return false;
    }
    Mdl::LogInfo("[UsdMdl_CreateMdl] prim type: " + prim.GetTypeName().GetString());

    bool isMaterial(prim.IsA<UsdShadeMaterial>());
    if (isMaterial)
    {
        Mdl::LogInfo("[UsdMdl_CreateMdl] prim is a material");
        UsdShadeMaterial material(prim);
        assert(material);
        std::string mdlMaterialInstanceName("MDLMaterial");
        const int rtn(ConvertUsdMaterialToMdl(material, mi::neuraylib::Mdl::Get().GetNeuray(), mdlMaterialInstanceName.c_str()));
        return (rtn == 0);
    }

    return false;
}

extern void EnableMdlDiscoveryPlugin(bool enable);

bool UsdMdl_EnableDiscoveryPlugin(bool enable)
{
    EnableMdlDiscoveryPlugin(enable);
    return true;
}

PXR_NAMESPACE_CLOSE_SCOPE
