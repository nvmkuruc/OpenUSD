/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#pragma once
#include "pxr/usd/plugin/usdMdl/neuray.h"
#include <map>
#include <string>
#include "pxr/usd/sdf/types.h"

PXR_NAMESPACE_OPEN_SCOPE

/// Helper class for the MDL Distiller setup
class Distiller
{
public:
    /// UsdPreviewSurface inputs
    /// Some inputs are not set, see typeMapInputs for details
    /// See also https://graphics.pixar.com/usd/docs/UsdPreviewSurface-Proposal.html
    typedef enum
    {
          invalid = -1
        , diffuseColor // color3f
        , emissiveColor // color3f
        , useSpecularWorkflow // int
        , specularColor // color3f used if useSpecularWorkflow = 1
        , metallic // float used if useSpecularWorkflow = 0
        , roughness // float
        , clearcoat // float
        , clearcoatRoughness // float
        , opacity // float
        , ior // float
        , normal // normal3f
        , displacement // float
        , occlusion // float

    } USD_PREVIEW_SURFACE_INPUT;

    /// Parameters computed after distillation
    class InputParm
    {
    public:
        void SetTexture(const std::string & input)
        {
            Reset();
            m_texture = input;
            m_is_texture = true;
        }
        void SetConstantColor(const float input[3])
        {
            Reset();
            m_floats[0] = input[0];
            m_floats[1] = input[1];
            m_floats[2] = input[2];
            m_is_constant_color = true;
        }
        void SetFloat(const float & input)
        {
            Reset();
            m_floats[0] = input;
            m_is_float = true;
        }
        void SetFloatArray(const float input[3])
        {
            Reset();
            m_floats[0] = input[0];
            m_floats[1] = input[1];
            m_floats[2] = input[2];
            m_is_float_array = true;
        }
        bool IsTexture(std::string & output) const
        {
            output = m_texture;
            return m_is_texture;
        }
        bool IsConstantColor(float output[3]) const
        {
            output[0] = m_floats[0];
            output[1] = m_floats[1];
            output[2] = m_floats[2];
            return m_is_constant_color;
        }
        bool IsFloat(float & output) const
        {
            output = m_floats[0];
            return m_is_float;
        }
        bool IsFloatArray(float output[3]) const
        {
            output[0] = m_floats[0];
            output[1] = m_floats[1];
            output[2] = m_floats[2];
            return m_is_float_array;
        }
    private:
        float m_floats[3];
        std::string m_texture;
        bool m_is_texture = false;
        bool m_is_constant_color = false;
        bool m_is_float = false;
        bool m_is_float_array = false;
    private:
        void Reset()
        {
            m_is_texture = false;
            m_is_constant_color = false;
            m_is_float = false;
            m_is_float_array = false;
            m_texture = "";
            m_floats[0] = m_floats[1] = m_floats[2] = 0;
        }
    };
    
public:
    /// Which material to distill
    void SetMaterialName(const std::string & material)
    {
        m_material_name = material;
    }

    /// Which model to target 
    /// IGNORED for the time being
    void SetTargetModel(const std::string & target)
    {
        m_target_model = target;
    }

    /// Set output folder for the baked images files
    /// IGNORED for the time being
    void SetOutputFolder(const std::string & output_folder)
    {
        m_output_folder = output_folder;
    }

    /// Start the distillation process
    bool Distill();

    /// Access the parameters after the distillation process
    bool GetParameters(const USD_PREVIEW_SURFACE_INPUT & input, InputParm & parm) const;

private:
    std::string m_material_name;
    std::string m_target_model = "ue4";
    std::string m_output_folder;
    std::map<USD_PREVIEW_SURFACE_INPUT, InputParm> m_input_parms;


private:
    /// Reset output information before distilling
    void Reset()
    {
        m_input_parms.clear();
    }
};

PXR_NAMESPACE_CLOSE_SCOPE
