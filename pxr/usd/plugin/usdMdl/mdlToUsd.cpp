/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#include "pxr/pxr.h"
#include "pxr/usd/plugin/usdMdl/mdlToUsd.h"
#include "pxr/usd/plugin/usdMdl/utils.h"
#include "pxr/usd/plugin/usdMdl/neuray.h"
#include "pxr/usd/usdUI/sceneGraphPrimAPI.h"

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

class ConversionTable
{
private:
    typedef std::map<mi::neuraylib::IType::Kind, SdfValueTypeName> KindToSdfValueTypeNameType;
    class KindToSdfValueTypeName : public KindToSdfValueTypeNameType
    {
    };

    KindToSdfValueTypeName m_ArrayOfSimpleTypeMap;

    // Vectors
    typedef mi::Size DIMENSION;
    typedef std::map<DIMENSION, SdfValueTypeName> DimensionToSdfValueTypeNameType;
    class DimensionToSdfValueTypeName : public DimensionToSdfValueTypeNameType
    {
    };

    typedef std::map<mi::neuraylib::IType::Kind, DimensionToSdfValueTypeName> KindToDimensionToSdfValueTypeNameType;
    class KindToDimensionToSdfValueTypeName : public KindToDimensionToSdfValueTypeNameType
    {
    };

    KindToDimensionToSdfValueTypeName m_VectorTypeMap;
    KindToDimensionToSdfValueTypeName m_ArrayOfVectorTypeMap;

    // Matrices
    typedef std::map<DIMENSION, DimensionToSdfValueTypeName> DimensionToDimensionToSdfValueTypeNameType;
    class DimensionToDimensionToSdfValueTypeName : public DimensionToDimensionToSdfValueTypeNameType
    {

    };

    typedef std::map<mi::neuraylib::IType::Kind, DimensionToDimensionToSdfValueTypeName> KindToDimensionToDimensionToSdfValueTypeNameType;
    class KindToDimensionToDimensionToSdfValueTypeName : public KindToDimensionToDimensionToSdfValueTypeNameType
    {
    public:
        bool findType(mi::neuraylib::IType::Kind kind, mi::Size size_matrix, mi::Size size_compound, SdfValueTypeName & output) const
        {
            KindToDimensionToDimensionToSdfValueTypeName::const_iterator it(find(kind));
            if (it != end())
            {
                DimensionToDimensionToSdfValueTypeNameType::const_iterator it2(it->second.find(size_matrix));
                if (it2 != it->second.end())
                {
                    DimensionToSdfValueTypeNameType::const_iterator it3(it2->second.find(size_compound));
                    if (it3 != it2->second.end())
                    {
                        output = it3->second;
                        return true;
                    }
                }
            }
            return false;
        }
    };
    KindToDimensionToDimensionToSdfValueTypeName m_MatrixTypeMap;
    KindToDimensionToDimensionToSdfValueTypeName m_ArrayOfMatrixTypeMap;

private:

    void AddConversion(KindToDimensionToSdfValueTypeName & toArray, mi::neuraylib::IType::Kind kind, mi::Size dimension, SdfValueTypeName typeName)
    {
        DimensionToSdfValueTypeName table;
        KindToDimensionToSdfValueTypeName::iterator it(toArray.find(kind));
        if (it != toArray.end())
        {
            table = it->second;
        }
        table[dimension] = typeName;
        toArray[kind] = table;
    }

    void AddVectorConversion(mi::neuraylib::IType::Kind kind, mi::Size dimension, SdfValueTypeName typeName)
    {
        return AddConversion(m_VectorTypeMap, kind, dimension, typeName);
    }

    void AddArrayOfVectorConversion(mi::neuraylib::IType::Kind kind, mi::Size dimension, SdfValueTypeName typeName)
    {
        return AddConversion(m_ArrayOfVectorTypeMap, kind, dimension, typeName);
    }

    void AddMatrixConversionToMap(
        KindToDimensionToDimensionToSdfValueTypeName & toMap
        , mi::neuraylib::IType::Kind kind
        , mi::Size size_matrix
        , mi::Size size_compound
        , SdfValueTypeName typeName
    )
    {
        DimensionToSdfValueTypeName table;
        DimensionToDimensionToSdfValueTypeName firstDimTable;
        KindToDimensionToDimensionToSdfValueTypeName::iterator it(toMap.find(kind));
        if (it != toMap.end())
        {
            firstDimTable = it->second;
            DimensionToDimensionToSdfValueTypeNameType::const_iterator it2(firstDimTable.find(size_matrix));
            if (it2 != firstDimTable.end())
            {
                table = it2->second;
            }
        }
        table[size_compound] = typeName;
        firstDimTable[size_matrix] = table;
        toMap[kind] = firstDimTable;
    }

    void AddMatrixConversion(mi::neuraylib::IType::Kind kind, mi::Size size_matrix, mi::Size size_compound, SdfValueTypeName typeName)
    {
        AddMatrixConversionToMap(m_MatrixTypeMap, kind, size_matrix, size_compound, typeName);
    }

    void AddArrayOfMatrixConversion(mi::neuraylib::IType::Kind kind, mi::Size size_matrix, mi::Size size_compound, SdfValueTypeName typeName)
    {
        AddMatrixConversionToMap(m_ArrayOfMatrixTypeMap, kind, size_matrix, size_compound, typeName);
    }
    
    void AddArrayOfSimpleConversion(mi::neuraylib::IType::Kind kind, SdfValueTypeName typeName)
    {
        m_ArrayOfSimpleTypeMap[kind] = typeName;
    }

public:
    ConversionTable()
    {
        // Vector
        AddVectorConversion(mi::neuraylib::IType::TK_BOOL, 2, SdfValueTypeNames->Int2);
        AddVectorConversion(mi::neuraylib::IType::TK_BOOL, 3, SdfValueTypeNames->Int3);
        AddVectorConversion(mi::neuraylib::IType::TK_BOOL, 4, SdfValueTypeNames->Int4);
        AddVectorConversion(mi::neuraylib::IType::TK_INT, 2, SdfValueTypeNames->Int2);
        AddVectorConversion(mi::neuraylib::IType::TK_INT, 3, SdfValueTypeNames->Int3);
        AddVectorConversion(mi::neuraylib::IType::TK_INT, 4, SdfValueTypeNames->Int4);
        AddVectorConversion(mi::neuraylib::IType::TK_FLOAT, 2, SdfValueTypeNames->Float2);
        AddVectorConversion(mi::neuraylib::IType::TK_FLOAT, 3, SdfValueTypeNames->Float3);
        AddVectorConversion(mi::neuraylib::IType::TK_FLOAT, 4, SdfValueTypeNames->Float4);
        AddVectorConversion(mi::neuraylib::IType::TK_DOUBLE, 2, SdfValueTypeNames->Double2);
        AddVectorConversion(mi::neuraylib::IType::TK_DOUBLE, 3, SdfValueTypeNames->Double3);
        AddVectorConversion(mi::neuraylib::IType::TK_DOUBLE, 4, SdfValueTypeNames->Double4);

        // Matrix
        // Integer
        AddMatrixConversion(mi::neuraylib::IType::TK_INT, 2, 2, SdfValueTypeNames->Matrix2d);
        AddMatrixConversion(mi::neuraylib::IType::TK_INT, 2, 3, SdfValueTypeNames->Int3Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_INT, 2, 4, SdfValueTypeNames->Int4Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_INT, 3, 2, SdfValueTypeNames->Int2Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_INT, 3, 3, SdfValueTypeNames->Matrix3d);
        AddMatrixConversion(mi::neuraylib::IType::TK_INT, 3, 4, SdfValueTypeNames->Int4Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_INT, 4, 2, SdfValueTypeNames->Int2Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_INT, 4, 3, SdfValueTypeNames->Int3Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_INT, 4, 4, SdfValueTypeNames->Matrix4d);
        // Float
        AddMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 2, 2, SdfValueTypeNames->Matrix2d);
        AddMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 2, 3, SdfValueTypeNames->Float3Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 2, 4, SdfValueTypeNames->Float4Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 3, 2, SdfValueTypeNames->Float2Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 3, 3, SdfValueTypeNames->Matrix3d);
        AddMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 3, 4, SdfValueTypeNames->Float4Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 4, 2, SdfValueTypeNames->Float2Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 4, 3, SdfValueTypeNames->Float3Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 4, 4, SdfValueTypeNames->Matrix4d);
        // Double
        AddMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 2, 2, SdfValueTypeNames->Matrix2d);
        AddMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 2, 3, SdfValueTypeNames->Double3Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 2, 4, SdfValueTypeNames->Double4Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 3, 2, SdfValueTypeNames->Double2Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 3, 3, SdfValueTypeNames->Matrix3d);
        AddMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 3, 4, SdfValueTypeNames->Double4Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 4, 2, SdfValueTypeNames->Double2Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 4, 3, SdfValueTypeNames->Double3Array);
        AddMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 4, 4, SdfValueTypeNames->Matrix4d);
        
        // Array of Vector
        AddArrayOfVectorConversion(mi::neuraylib::IType::TK_BOOL, 2, SdfValueTypeNames->Int2Array);
        AddArrayOfVectorConversion(mi::neuraylib::IType::TK_BOOL, 3, SdfValueTypeNames->Int3Array);
        AddArrayOfVectorConversion(mi::neuraylib::IType::TK_BOOL, 4, SdfValueTypeNames->Int4Array);
        AddArrayOfVectorConversion(mi::neuraylib::IType::TK_INT, 2, SdfValueTypeNames->Int2Array);
        AddArrayOfVectorConversion(mi::neuraylib::IType::TK_INT, 3, SdfValueTypeNames->Int3Array);
        AddArrayOfVectorConversion(mi::neuraylib::IType::TK_INT, 4, SdfValueTypeNames->Int4Array);
        AddArrayOfVectorConversion(mi::neuraylib::IType::TK_FLOAT, 2, SdfValueTypeNames->Float2Array);
        AddArrayOfVectorConversion(mi::neuraylib::IType::TK_FLOAT, 3, SdfValueTypeNames->Float3Array);
        AddArrayOfVectorConversion(mi::neuraylib::IType::TK_FLOAT, 4, SdfValueTypeNames->Float4Array);
        AddArrayOfVectorConversion(mi::neuraylib::IType::TK_DOUBLE, 2, SdfValueTypeNames->Double2Array);
        AddArrayOfVectorConversion(mi::neuraylib::IType::TK_DOUBLE, 3, SdfValueTypeNames->Double3Array);
        AddArrayOfVectorConversion(mi::neuraylib::IType::TK_DOUBLE, 4, SdfValueTypeNames->Double4Array);

        // Array of matrix
        // Integer
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_INT, 2, 2, SdfValueTypeNames->Matrix2dArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_INT, 2, 3, SdfValueTypeNames->IntArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_INT, 2, 4, SdfValueTypeNames->IntArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_INT, 3, 2, SdfValueTypeNames->IntArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_INT, 3, 3, SdfValueTypeNames->Matrix3dArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_INT, 3, 4, SdfValueTypeNames->IntArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_INT, 4, 2, SdfValueTypeNames->IntArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_INT, 4, 3, SdfValueTypeNames->IntArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_INT, 4, 4, SdfValueTypeNames->Matrix4dArray);

        // Float
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 2, 2, SdfValueTypeNames->Matrix2dArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 2, 3, SdfValueTypeNames->FloatArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 2, 4, SdfValueTypeNames->FloatArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 3, 2, SdfValueTypeNames->FloatArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 3, 3, SdfValueTypeNames->Matrix3dArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 3, 4, SdfValueTypeNames->FloatArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 4, 2, SdfValueTypeNames->FloatArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 4, 3, SdfValueTypeNames->FloatArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_FLOAT, 4, 4, SdfValueTypeNames->Matrix4dArray);

        // Double
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 2, 2, SdfValueTypeNames->Matrix2dArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 2, 3, SdfValueTypeNames->DoubleArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 2, 4, SdfValueTypeNames->DoubleArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 3, 2, SdfValueTypeNames->DoubleArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 3, 3, SdfValueTypeNames->Matrix3dArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 3, 4, SdfValueTypeNames->DoubleArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 4, 2, SdfValueTypeNames->DoubleArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 4, 3, SdfValueTypeNames->DoubleArray);
        AddArrayOfMatrixConversion(mi::neuraylib::IType::TK_DOUBLE, 4, 4, SdfValueTypeNames->Matrix4dArray);

        // Array of simple types
        AddArrayOfSimpleConversion(mi::neuraylib::IType::TK_BOOL, SdfValueTypeNames->BoolArray);
        AddArrayOfSimpleConversion(mi::neuraylib::IType::TK_INT, SdfValueTypeNames->IntArray);
        AddArrayOfSimpleConversion(mi::neuraylib::IType::TK_ENUM, SdfValueTypeNames->IntArray);
        AddArrayOfSimpleConversion(mi::neuraylib::IType::TK_FLOAT, SdfValueTypeNames->FloatArray);
        AddArrayOfSimpleConversion(mi::neuraylib::IType::TK_DOUBLE, SdfValueTypeNames->DoubleArray);
        AddArrayOfSimpleConversion(mi::neuraylib::IType::TK_STRING, SdfValueTypeNames->StringArray);
        AddArrayOfSimpleConversion(mi::neuraylib::IType::TK_COLOR, SdfValueTypeNames->Color3fArray);
        AddArrayOfSimpleConversion(mi::neuraylib::IType::TK_STRUCT, SdfValueTypeNames->TokenArray);
        AddArrayOfSimpleConversion(mi::neuraylib::IType::TK_TEXTURE, SdfValueTypeNames->AssetArray);
        // mi::neuraylib::IType::TK_ARRAY - Not allowed
        // mi::neuraylib::IType::TK_LIGHT_PROFILE - Not allowed
        // mi::neuraylib::IType::TK_BSDF_MEASUREMENT - Not allowed
        // mi::neuraylib::IType::TK_BSDF - Not allowed
        // mi::neuraylib::IType::TK_EDF - Not allowed
        // mi::neuraylib::IType::TK_VDF - Not allowed
    }
    bool getSdfValueTypeName(const KindToSdfValueTypeName & fromArray, const IType * type, SdfValueTypeName & output) const
    {
        KindToSdfValueTypeName::const_iterator it(fromArray.find(type->get_kind()));
        if (it != fromArray.end())
        {
            output = it->second;
            return true;
        }
        return false;
    }

    bool getSdfValueTypeName(const KindToDimensionToSdfValueTypeName & fromArray, const IType * type, SdfValueTypeName & output) const
    {
        if (type && type->get_kind() == mi::neuraylib::IType::TK_VECTOR)
        {
            mi::base::Handle<const IType_compound> type_compound(type->get_interface<IType_compound>());
            if (type_compound)
            {
                mi::base::Handle<const IType> component_type(type_compound->get_component_type(0));
                mi::Size size(type_compound->get_size());
                KindToDimensionToSdfValueTypeName::const_iterator it(fromArray.find(component_type.get()->get_kind()));
                if (it != fromArray.end())
                {
                    DimensionToSdfValueTypeNameType::const_iterator it2(it->second.find(size));
                    if (it2 != it->second.end())
                    {
                        output = it2->second;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool getSdfValueTypeName(const IType * type, SdfValueTypeName & output) const
    {
        if (type && type->get_kind() == mi::neuraylib::IType::TK_VECTOR)
        {
            return getSdfValueTypeName(m_VectorTypeMap, type, output);
        }
        else if (type->get_kind() == mi::neuraylib::IType::TK_ARRAY)
        {
            mi::base::Handle<const IType_array> type_array(type->get_interface<IType_array>());
            mi::base::Handle<const IType> element_type(type_array->get_element_type());

            const mi::neuraylib::IType::Kind elementKind(element_type->get_kind());
            mi::Size array_size(type_array->get_size());
            if (elementKind == mi::neuraylib::IType::TK_VECTOR)
            {
                // Arrays of vectors
                return getSdfValueTypeName(m_ArrayOfVectorTypeMap, element_type.get(), output);
            }
            else if (elementKind == mi::neuraylib::IType::TK_MATRIX)
            {
                mi::base::Handle<const IType_compound> type_compound(element_type->get_interface<IType_compound>());
                mi::base::Handle<const IType> component_type(type_compound->get_component_type(0));
                mi::Size size_matrix(type_compound->get_size());
                if (component_type->get_kind() == mi::neuraylib::IType::TK_VECTOR)
                {
                    mi::base::Handle<const IType_compound> component_type_compound(component_type->get_interface<IType_compound>());
                    mi::base::Handle<const IType> component_type(component_type_compound->get_component_type(0));
                    mi::Size size_compound(component_type_compound->get_size());
                    return m_ArrayOfMatrixTypeMap.findType(component_type.get()->get_kind(), size_matrix, size_compound, output);
                }
            }
            else if (
                elementKind == mi::neuraylib::IType::TK_BOOL
                || elementKind == mi::neuraylib::IType::TK_INT
                || elementKind == mi::neuraylib::IType::TK_ENUM
                || elementKind == mi::neuraylib::IType::TK_FLOAT
                || elementKind == mi::neuraylib::IType::TK_DOUBLE
                || elementKind == mi::neuraylib::IType::TK_STRING
                || elementKind == mi::neuraylib::IType::TK_COLOR
                || elementKind == mi::neuraylib::IType::TK_STRUCT
                || elementKind == mi::neuraylib::IType::TK_TEXTURE
                )
            {
                // Array of simple types
                return getSdfValueTypeName(m_ArrayOfSimpleTypeMap, element_type.get(), output);
            }
        }
        else if (type->get_kind() == mi::neuraylib::IType::TK_MATRIX)
        {
            mi::base::Handle<const IType_compound> type_compound(type->get_interface<IType_compound>());
            mi::base::Handle<const IType> component_type(type_compound->get_component_type(0));
            mi::Size size_matrix(type_compound->get_size());
            if (component_type->get_kind() == mi::neuraylib::IType::TK_VECTOR)
            {
                mi::base::Handle<const IType_compound> component_type_compound(component_type->get_interface<IType_compound>());
                mi::base::Handle<const IType> component_type(component_type_compound->get_component_type(0));
                mi::Size size_compound(component_type_compound->get_size());
                return m_MatrixTypeMap.findType(component_type.get()->get_kind(), size_matrix, size_compound, output);
            }
        }
        return false;
    }
};

ConversionTable theConversionTable;

const char * MdlToUsd::KEY_MDL_TYPE_INFO = "mdl:type";
VtValue GetValueFromArrayOfMatrix(const IExpression * expression, MdlToUsd::ExtraInfo & extra);

TfToken MdlToUsd::ConvertComplexType(const IType * type) const
{
    if (type->get_kind() == mi::neuraylib::IType::TK_ALIAS)
    {
        mi::base::Handle<const IType_alias> type_alias(type->get_interface<IType_alias>());
        mi::base::Handle<const IType> aliased_type(type_alias->get_aliased_type());
        return ConvertType(aliased_type.get());
    }
    else if (
           type->get_kind() == mi::neuraylib::IType::TK_VECTOR 
        || type->get_kind() == mi::neuraylib::IType::TK_MATRIX
        || type->get_kind() == mi::neuraylib::IType::TK_ARRAY
        )
    {
        SdfValueTypeName typeName;
        if (theConversionTable.getSdfValueTypeName(type, typeName))
        {
            return typeName.GetAsToken();
        }
    }
    return TfToken();
}

TfToken MdlToUsd::ConvertType(const IType * type) const
{
    mi::neuraylib::IType::Kind kind(type->get_kind());
    TYPENAMEMAP::const_iterator itname(m_TypeNameMap.find(kind));
    std::string itnamestr("Unknown");
    if (itname != m_TypeNameMap.end())
    {
        itnamestr = itname->second;
    }
    TYPEMAP::const_iterator it(m_TypeMap.find(kind));
    if (it != m_TypeMap.end())
    {
        //mi::neuraylib::Mdl::LogVerbose("[MdlToUsd] Kind converted as token: " + itnamestr);
        return it->second.GetAsToken();
    }

    // Try other types
    TfToken tk(ConvertComplexType(type));
    if (!tk.IsEmpty())
    {
        return tk;
    }

    if (itname != m_TypeNameMap.end())
    {
        mi::neuraylib::Mdl::LogWarning("[MdlToUsd] Unsupported type: " + itname->second);
    }
    else
    {
        mi::neuraylib::Mdl::LogError("[MdlToUsd] Unknown type");
    }
    return TfToken();
}

template<class neurayType, class gfType> VtValue GetValueFromArrayOfVector(const mi::neuraylib::IValue_array * valuearray, mi::Size numberOfElements, mi::Size vectorElementSize)
{
    VtArray<gfType> output;
    for (mi::Size i = 0; i < numberOfElements; i++)
    {
        mi::base::Handle<const IValue_vector> vec(valuearray->get_value<IValue_vector>(i));

        gfType item;
        for (mi::Size j = 0; j < vectorElementSize; j++)
        {
            mi::base::Handle<const neurayType> ifv(vec->get_value<neurayType>(j));
            if (ifv)
            {
                item[j] = ifv->get_value();
            }
        }
        output.push_back(item);
    }
    return VtValue(output);
}

const char * GetStringAnnotation(const mi::neuraylib::IAnnotation_block * block, const std::string & name)
{
    mi::Size sz(block->get_size());
    for (mi::Size i = 0; i < sz; i++)
    {
        mi::base::Handle<const mi::neuraylib::IAnnotation> anno(block->get_annotation(i));
        if (anno)
        {
            if (name == anno->get_name())
            {
                mi::base::Handle<const mi::neuraylib::IExpression_list> args(anno->get_arguments());
                if (args)
                {
                    mi::base::Handle<const mi::neuraylib::IExpression> expr(args->get_expression(mi::Size(0)));
                    if (expr)
                    {
                        mi::base::Handle<const IType> type(expr->get_type());
                        if (type)
                        {
                            if (type->get_kind() == mi::neuraylib::IType::TK_STRING)
                            {
                                mi::base::Handle<const mi::neuraylib::IExpression_constant> constant(expr->get_interface<mi::neuraylib::IExpression_constant>());
                                if (constant)
                                {
                                    mi::base::Handle<const mi::neuraylib::IValue_string> value(constant->get_value<mi::neuraylib::IValue_string>());
                                    if (value)
                                    {
                                        return value->get_value();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

bool SetExtraInfoFromEnumValue(const IExpression * value, MdlToUsd::ExtraInfo & extra)
{
    if (value->get_kind() == mi::neuraylib::IExpression::EK_CONSTANT)
    {
        mi::base::Handle<const mi::neuraylib::IExpression_constant> constant(value->get_interface<mi::neuraylib::IExpression_constant>());
        if (constant)
        {
            mi::base::Handle<const mi::neuraylib::IValue_enum> enumval(constant->get_value<mi::neuraylib::IValue_enum>());

            if (!enumval)
            {
                // Is this an array of enums?
                mi::base::Handle<const mi::neuraylib::IValue_array> valuearray(constant->get_value<mi::neuraylib::IValue_array>());
                if (valuearray)
                {
                    if(valuearray->get_size()>0)
                    {
                        enumval = valuearray->get_value<const mi::neuraylib::IValue_enum>(0);
                    }
                }
            }
            
            if (enumval)
            {
                const char* enumname(enumval->get_name());
                mi::base::Handle<const mi::neuraylib::IType_enum> enumtype(enumval->get_type());
                if (enumtype)
                {
                    const char * symbol(enumtype->get_symbol());
                    if (symbol)
                    {
                        extra.m_valueType = MdlToUsd::ExtraInfo::VTT_ENUM;
                        extra.m_enumSymbol = symbol;
                        extra.m_enumValueName = enumname;
                        mi::base::Handle<const mi::neuraylib::IAnnotation_block> annoblock(enumtype->get_value_annotations(enumval->get_index()));
                        if (annoblock)
                        {
                            const char * anno(GetStringAnnotation(annoblock.get(), "::anno::description(string)"));
                            if (anno)
                            {
                                extra.m_enumValueDescription = anno;
                            }
                        }

                        // enum valid values (e.g. "Display Value 1:0|Display Value 2:1|...")
                        std::stringstream options;
                        for (mi::Size i = 0; i < enumtype->get_size(); i++)
                        {
                            const char* vname(enumtype->get_value_name(i));
                            mi::Sint32 errors;
                            const mi::Sint32 vcode(enumtype->get_value_code(i, &errors));
                            if (0 == errors)
                            {
                                //virtual const IAnnotation_block* get_value_annotations(Size index) const = 0;
                                if (i > 0)
                                {
                                    // Add separator but for the first element
                                    options << "|";
                                }
                                options << vname << ":" << vcode;
                            }
                        }
                        extra.m_enumValueOptions = options.str();
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool GetGfValueFromColor(const mi::neuraylib::IValue_color * value, GfVec3f & output)
{
    if (value)
    {
        mi::base::Handle<const mi::neuraylib::IValue_float> r(value->get_value(0));
        mi::base::Handle<const mi::neuraylib::IValue_float> g(value->get_value(1));
        mi::base::Handle<const mi::neuraylib::IValue_float> b(value->get_value(2));
        float rgb[] = { r->get_value(), g->get_value(), b->get_value() };
        output = GfVec3f(rgb[0], rgb[1], rgb[2]);
        return true;
    }
    return false;
}

bool GetGfValueFromColor(const mi::neuraylib::IExpression * value, GfVec3f & output)
{
    if (value && value->get_kind() == mi::neuraylib::IExpression::EK_CONSTANT)
    {
        mi::base::Handle<const mi::neuraylib::IExpression_constant> constant(value->get_interface<mi::neuraylib::IExpression_constant>());
        if (constant)
        {
            mi::base::Handle<const mi::neuraylib::IValue_color> valueColor(constant->get_value<mi::neuraylib::IValue_color>());
            return GetGfValueFromColor(valueColor.get(), output);
        }
    }
    return false;
}

VtValue GetValueFromColor(const mi::neuraylib::IExpression * value)
{
    GfVec3f color;
    if (GetGfValueFromColor(value, color))
    {
        return VtValue(color);
    }
    return VtValue();
}

VtValue GetValueFromResource(const mi::neuraylib::IExpression * value)
{
    if (value && value->get_kind() == mi::neuraylib::IExpression::EK_CONSTANT)
    {
        mi::base::Handle<const mi::neuraylib::IExpression_constant> constant(value->get_interface<mi::neuraylib::IExpression_constant>());
        if (constant)
        {
            mi::base::Handle<const mi::neuraylib::IValue_resource> resource(constant->get_value<mi::neuraylib::IValue_resource>());
            if (resource)
            {
                const char* file_path(resource->get_file_path());
                if (file_path)
                {
                    return VtValue(SdfAssetPath(file_path));
                }
            }
        }
    }
    return VtValue();
}

VtValue GetValueFromTexture(const mi::neuraylib::IExpression * value)
{
    return GetValueFromResource(value);
}

VtValue GetValueFromLightProfile(const mi::neuraylib::IExpression * value)
{
    return GetValueFromResource(value);
}

VtValue GetValueFromBsdfMeasurement(const mi::neuraylib::IExpression * value)
{
    return GetValueFromResource(value);
}

VtValue MdlToUsd::GetValue(const IType * type, const IExpression * value, TfToken & typeOut, ExtraInfo & extra) const
{
    if (value->get_kind() == mi::neuraylib::IExpression::EK_CALL)
    {
        mi::base::Handle<const mi::neuraylib::IExpression_call> callexpr(value->get_interface<mi::neuraylib::IExpression_call>());
        const char* call(callexpr->get_call());
        typeOut = SdfValueTypeNames->Token.GetAsToken();
        extra.m_valueType = ExtraInfo::VTT_CALL;
        return VtValue(std::string(call));
    }
    else if (value->get_kind() == mi::neuraylib::IExpression::EK_PARAMETER)
    {
        mi::base::Handle<const mi::neuraylib::IExpression_parameter> parameter(value->get_interface<mi::neuraylib::IExpression_parameter>());
        /// Returns the index of the referenced parameter.
        mi::Size index(parameter->get_index());
        extra.m_valueType = ExtraInfo::VTT_PARM;
        return VtValue(index);
    }

    if (type->get_kind() == mi::neuraylib::IType::TK_ALIAS)
    {
        mi::base::Handle<const IType_alias> type_alias(type->get_interface<IType_alias>());
        mi::base::Handle<const IType> aliased_type(type_alias->get_aliased_type());
        return GetValue(aliased_type.get(), value, typeOut, extra);
    }
    else if (type->get_kind() == mi::neuraylib::IType::TK_BOOL)
    {
        return ConvertValue<mi::neuraylib::IValue_bool>(value);
    }
    else if (type->get_kind() == mi::neuraylib::IType::TK_INT)
    {
        return ConvertValue<mi::neuraylib::IValue_int>(value);
    }
    else if (type->get_kind() == mi::neuraylib::IType::TK_ENUM)
    {
        SetExtraInfoFromEnumValue(value, extra);
        return ConvertValue<mi::neuraylib::IValue_enum>(value);
    }
    else if (type->get_kind() == mi::neuraylib::IType::TK_FLOAT)
    {
        return ConvertValue<mi::neuraylib::IValue_float>(value);
    }
    else if (type->get_kind() == mi::neuraylib::IType::TK_DOUBLE)
    {
        return ConvertValue<mi::neuraylib::IValue_double>(value);
    }
    else if (type->get_kind() == mi::neuraylib::IType::TK_STRING)
    {
        return ConvertValue<mi::neuraylib::IValue_string>(value);
    }
    else if (type->get_kind() == mi::neuraylib::IType::TK_VECTOR)
    {
        return GetValueFromVector(value, extra);
    }
    else if (type->get_kind() == mi::neuraylib::IType::TK_MATRIX)
    {
        return GetValueFromMatrix(value, extra);
    }
    else if (type->get_kind() == mi::neuraylib::IType::TK_COLOR)
    {
        return GetValueFromColor(value);
    }
    else if (type->get_kind() == mi::neuraylib::IType::TK_TEXTURE)
    {
        return GetValueFromTexture(value);
    }
    else if (type->get_kind() == mi::neuraylib::IType::TK_LIGHT_PROFILE)
    {
        extra.SetMdlType("light_profile");
        return GetValueFromLightProfile(value);
    }
    else if (type->get_kind() == mi::neuraylib::IType::TK_BSDF_MEASUREMENT)
    {
        extra.SetMdlType("bsdf_measurement");
        return GetValueFromBsdfMeasurement(value);
    }
    else if (type->get_kind() == mi::neuraylib::IType::TK_ARRAY)
    {
        mi::base::Handle<const IType_array> type_array(type->get_interface<IType_array>());
        mi::base::Handle<const IType> element_type(type_array->get_element_type());
        const mi::neuraylib::IType::Kind elementKind(element_type->get_kind());
        if (elementKind == mi::neuraylib::IType::TK_BOOL)
        {
            return ConvertArrayValue<mi::neuraylib::IValue_bool, bool>(value);
        }
        else if (elementKind == mi::neuraylib::IType::TK_INT)
        {
            return ConvertArrayValue<mi::neuraylib::IValue_int, int>(value);
        }
        else if (elementKind == mi::neuraylib::IType::TK_FLOAT)
        {
            return ConvertArrayValue<mi::neuraylib::IValue_float, float>(value);
        }
        else if (elementKind == mi::neuraylib::IType::TK_DOUBLE)
        {
            return ConvertArrayValue<mi::neuraylib::IValue_double, double>(value);
        }
        else if (elementKind == mi::neuraylib::IType::TK_STRING)
        {
            return ConvertArrayValue<mi::neuraylib::IValue_string, string>(value);
        }
        else if (elementKind == mi::neuraylib::IType::TK_MATRIX)
        {
            return GetValueFromArrayOfMatrix(value, extra);
        }
        else if (elementKind == mi::neuraylib::IType::TK_VECTOR)
        {
            if (value->get_kind() == mi::neuraylib::IExpression::EK_CONSTANT)
            {
                mi::base::Handle<const mi::neuraylib::IExpression_constant> constant(value->get_interface<mi::neuraylib::IExpression_constant>());
                if (constant)
                {
                    mi::base::Handle<const mi::neuraylib::IValue_array> valuearray(constant->get_value<mi::neuraylib::IValue_array>());
                    if (valuearray)
                    {
                        mi::neuraylib::IType::Kind vectorElementTypeKind;
                        mi::Size vectorElementSize(0);
                        mi::Size numberOfElements(valuearray->get_size());

                        if (numberOfElements > 0)
                        {
                            mi::base::Handle<const IValue_vector> vec(valuearray->get_value<IValue_vector>(0));
                            mi::base::Handle<const IValue_atomic> atom(vec->get_value(0));
                            mi::base::Handle<const IType> type(atom->get_type());                            
                            vectorElementTypeKind = type->get_kind();
                            vectorElementSize = vec->get_size();
                        }

                        std::stringstream type;
                        switch(vectorElementSize)
                        { 
                        case 2:
                            switch (vectorElementTypeKind)
                            {
                            case(mi::neuraylib::IType::TK_BOOL):
                                type << "bool2[" << numberOfElements << "]";
                                extra.SetMdlType(type.str());
                                return GetValueFromArrayOfVector<IValue_bool, GfVec2i>(valuearray.get(), numberOfElements, vectorElementSize);
                                break;
                            case(mi::neuraylib::IType::TK_INT):
                                return GetValueFromArrayOfVector<IValue_int, GfVec2i>(valuearray.get(), numberOfElements, vectorElementSize);
                                break;
                            case(mi::neuraylib::IType::TK_FLOAT):
                                return GetValueFromArrayOfVector<IValue_float, GfVec2f>(valuearray.get(), numberOfElements, vectorElementSize);
                                break;
                            case(mi::neuraylib::IType::TK_DOUBLE):
                                return GetValueFromArrayOfVector<IValue_double, GfVec2d>(valuearray.get(), numberOfElements, vectorElementSize);
                                break;
                            default:
                                break;
                            }
                            break;
                        case 3:
                            switch (vectorElementTypeKind)
                            {
                            case(mi::neuraylib::IType::TK_BOOL):
                                type << "bool3[" << numberOfElements << "]";
                                extra.SetMdlType(type.str());
                                return GetValueFromArrayOfVector<IValue_bool, GfVec3i>(valuearray.get(), numberOfElements, vectorElementSize);
                                break;
                            case(mi::neuraylib::IType::TK_INT):
                                return GetValueFromArrayOfVector<IValue_int, GfVec3i>(valuearray.get(), numberOfElements, vectorElementSize);
                                break;
                            case(mi::neuraylib::IType::TK_FLOAT):
                                return GetValueFromArrayOfVector<IValue_float, GfVec3f>(valuearray.get(), numberOfElements, vectorElementSize);
                                break;
                            case(mi::neuraylib::IType::TK_DOUBLE):
                                return GetValueFromArrayOfVector<IValue_double, GfVec3d>(valuearray.get(), numberOfElements, vectorElementSize);
                                break;
                            default:
                                break;
                            }
                            break;
                        case 4:
                            switch (vectorElementTypeKind)
                            {
                            case(mi::neuraylib::IType::TK_BOOL):
                                type << "bool4[" << numberOfElements << "]";
                                extra.SetMdlType(type.str());
                                return GetValueFromArrayOfVector<IValue_bool, GfVec4i>(valuearray.get(), numberOfElements, vectorElementSize);
                                break;
                            case(mi::neuraylib::IType::TK_INT):
                                return GetValueFromArrayOfVector<IValue_int, GfVec4i>(valuearray.get(), numberOfElements, vectorElementSize);
                                break;
                            case(mi::neuraylib::IType::TK_FLOAT):
                                return GetValueFromArrayOfVector<IValue_float, GfVec4f>(valuearray.get(), numberOfElements, vectorElementSize);
                                break;
                            case(mi::neuraylib::IType::TK_DOUBLE):
                                return GetValueFromArrayOfVector<IValue_double, GfVec4d>(valuearray.get(), numberOfElements, vectorElementSize);
                                break;
                            default:
                                break;
                            }
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        }
        else if (elementKind == mi::neuraylib::IType::TK_COLOR)
        {
            if (value->get_kind() == mi::neuraylib::IExpression::EK_CONSTANT)
            {
                mi::base::Handle<const mi::neuraylib::IExpression_constant> constant(value->get_interface<mi::neuraylib::IExpression_constant>());
                if (constant)
                {
                    mi::base::Handle<const mi::neuraylib::IValue_array> valuearray(constant->get_value<mi::neuraylib::IValue_array>());
                    if (valuearray)
                    {
                        VtArray<GfVec3f> output;
                        GfVec3f outputColor;

                        mi::Size sz(valuearray->get_size());
                        for (mi::Size i = 0; i<sz; i++)
                        {
                            mi::base::Handle<const mi::neuraylib::IValue_color> cvalue(valuearray->get_value<mi::neuraylib::IValue_color>(i));
                            if (cvalue && GetGfValueFromColor(cvalue.get(), outputColor))
                            {
                                output.push_back(outputColor);
                            }
                        }
                        VtValue outputvaluecolor(output);
                        return outputvaluecolor;
                    }
                }
            }
        }
        else if (elementKind == mi::neuraylib::IType::TK_STRUCT)
        {
            VtArray<TfToken> arrayOut;
            mi::base::Handle<const mi::neuraylib::IExpression_constant> constant(value->get_interface<mi::neuraylib::IExpression_constant>());
            if (constant)
            {
                mi::base::Handle<const mi::neuraylib::IValue_array> arr(constant->get_value<mi::neuraylib::IValue_array>());

                if (arr)
                {
                    mi::Size len(arr->get_size());
                    for (mi::Size i = 0; i < len; i++)
                    {
                        mi::base::Handle<const mi::neuraylib::IValue> val(arr->get_value(i));
                        if (val)
                        {

                        }
                    }
                }
            }
        }
        else if (elementKind == mi::neuraylib::IType::TK_ENUM)
        {
            SetExtraInfoFromEnumValue(value, extra);
            return ConvertArrayValue<mi::neuraylib::IValue_enum, int>(value);
        }

        mi::neuraylib::Mdl::LogError("[MdlToUsd] Array type not supported");
    }
    else if (type->get_kind() == mi::neuraylib::IType::TK_STRUCT)
    {
        if (value->get_kind() == mi::neuraylib::IExpression::EK_CONSTANT)
        {
            mi::base::Handle<const mi::neuraylib::IExpression_constant> constant(value->get_interface<mi::neuraylib::IExpression_constant>());
            if (constant)
            {
                mi::base::Handle<const mi::neuraylib::IValue_struct> structure(constant->get_value<mi::neuraylib::IValue_struct>());
                if (structure)
                {
                    mi::Size sz(structure->get_size());
                    for (mi::Size i = 0; i < sz; i++)
                    {
                        mi::base::Handle<const mi::neuraylib::IValue> val(structure->get_value(i));

                    }

                    mi::base::Handle<const mi::neuraylib::IType_struct> castval(type->get_interface<mi::neuraylib::IType_struct>());

                    const char* symbol(castval->get_symbol());
                    if (symbol)
                    {
                        //typeOut = SdrPropertyTypes->Vstruct;          WRONG

                        extra.m_valueType = ExtraInfo::VTT_STRUCT;
                        return VtValue();
                        return VtValue(std::string(symbol));
                    }
                }
            }
        }
        mi::neuraylib::Mdl::LogError("[MdlToUsd] Structure not supported");
    }
    else
    {
        TYPENAMEMAP::const_iterator itname(m_TypeNameMap.find(type->get_kind()));
        if (itname != m_TypeNameMap.end())
        {
            mi::neuraylib::Mdl::LogError("[MdlToUsd] Can not set default value for: " + itname->second);
        }
    }
    return VtValue();
}

template<class neurayType, class gfType> VtValue MdlToUsd::GetValueFromVector(const mi::neuraylib::IValue_vector * vec) const
{
    gfType rtnval;
    for (mi::Size i = 0; i < vec->get_size(); i++)
    {
        mi::base::Handle<const neurayType> ifv(vec->get_value<neurayType>(i));
        if (ifv)
        {
            rtnval[i] = ifv->get_value();
        }
        else
        {
            mi::neuraylib::Mdl::LogError("[GetValueFromVector] Bad type");
            return VtValue();
        }
    }
    return VtValue(rtnval);
}

VtValue MdlToUsd::GetValueFromVector(const mi::neuraylib::IExpression * value, ExtraInfo & extra) const
{
    if (value && value->get_kind() == mi::neuraylib::IExpression::EK_CONSTANT)
    {
        mi::base::Handle<const mi::neuraylib::IExpression_constant> constant(value->get_interface<mi::neuraylib::IExpression_constant>());
        if (constant)
        {
            mi::base::Handle<const mi::neuraylib::IValue_vector> v(constant->get_value<mi::neuraylib::IValue_vector>());
            if (v)
            {
                mi::Size size(v->get_size());

                mi::base::Handle<const mi::neuraylib::IType_vector> t(v->get_type());

                if (t)
                {
                    mi::base::Handle<const mi::neuraylib::IType_atomic> ta(t->get_element_type());
                    if (ta)
                    {
                        switch (ta->get_kind())
                        {
                        case mi::neuraylib::IType::TK_FLOAT:
                            switch (size)
                            {
                            case 2:
                                return GetValueFromVector<IValue_float, GfVec2f>(v.get());
                            case 3:
                                return GetValueFromVector<IValue_float, GfVec3f>(v.get());
                            case 4:
                                return GetValueFromVector<IValue_float, GfVec4f>(v.get());
                            }
                            break;
                        case mi::neuraylib::IType::TK_INT:
                            switch (size)
                            {
                            case 2:
                                return GetValueFromVector<IValue_int, GfVec2i>(v.get());
                            case 3:
                                return GetValueFromVector<IValue_int, GfVec3i>(v.get());
                            case 4:
                                return GetValueFromVector<IValue_int, GfVec4i>(v.get());
                            }
                            break;
                        case mi::neuraylib::IType::TK_BOOL:
                            switch (size)
                            {
                            case 2:
                                extra.SetMdlType("bool2");
                                return GetValueFromVector<IValue_bool, GfVec2i>(v.get());
                            case 3:
                                extra.SetMdlType("bool3");
                                return GetValueFromVector<IValue_bool, GfVec3i>(v.get());
                            case 4:
                                extra.SetMdlType("bool4");
                                return GetValueFromVector<IValue_bool, GfVec4i>(v.get());
                            }
                            break;
                        case mi::neuraylib::IType::TK_DOUBLE:
                            switch (size)
                            {
                            case 2:
                                return GetValueFromVector<IValue_double, GfVec2d>(v.get());
                            case 3:
                                return GetValueFromVector<IValue_double, GfVec3d>(v.get());
                            case 4:
                                return GetValueFromVector<IValue_double, GfVec4d>(v.get());
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    return VtValue();
}

template<class T> VtValue MdlToUsd::ConvertValue(const IExpression * expression) const
{
    if (expression->get_kind() == mi::neuraylib::IExpression::EK_CONSTANT)
    {
        mi::base::Handle<const mi::neuraylib::IExpression_constant> constant(expression->get_interface<mi::neuraylib::IExpression_constant>());
        if (constant)
        {
            mi::base::Handle<const T> value(constant->get_value<T>());
            if (value)
            {
                return VtValue(value->get_value());
            }
        }
    }
    else if (expression->get_kind() == mi::neuraylib::IExpression::EK_CALL)
    {
        mi::neuraylib::Mdl::LogError("[MdlToUsd] Function call not supported");
    }
    else if (expression->get_kind() == mi::neuraylib::IExpression::EK_PARAMETER)
    {
        mi::neuraylib::Mdl::LogError("[MdlToUsd] Parameter reference expression not supported");
    }
    else if (expression->get_kind() == mi::neuraylib::IExpression::EK_DIRECT_CALL)
    {
        mi::neuraylib::Mdl::LogError("[MdlToUsd] Direct call expression not supported");
    }
    else if (expression->get_kind() == mi::neuraylib::IExpression::EK_TEMPORARY)
    {
        mi::neuraylib::Mdl::LogError("[MdlToUsd] Temporary reference expression not supported");
    }
    return VtValue();
}

template<class MdlElementType, class USDElementType> VtValue MdlToUsd::ConvertArrayValue(const IExpression * expression) const
{
    if (expression->get_kind() == mi::neuraylib::IExpression::EK_CONSTANT)
    {
        mi::base::Handle<const mi::neuraylib::IExpression_constant> constant(expression->get_interface<mi::neuraylib::IExpression_constant>());
        if (constant)
        {
            mi::base::Handle<const mi::neuraylib::IValue_array> valuearray(constant->get_value<mi::neuraylib::IValue_array>());
            if (valuearray)
            {
                VtArray<USDElementType> output;

                mi::Size sz(valuearray->get_size());
                for (mi::Size i = 0; i<sz; i++)
                {
                    mi::base::Handle<const MdlElementType> value(valuearray->get_value<MdlElementType>(i));
                    if (value)
                    {
                        output.push_back(value->get_value());
                    }
                }
                return VtValue(output);
            }
        }
    }
    else if (expression->get_kind() == mi::neuraylib::IExpression::EK_CALL)
    {
        mi::neuraylib::Mdl::LogError("[ConvertArrayValue] Function call not supported");
    }
    else if (expression->get_kind() == mi::neuraylib::IExpression::EK_PARAMETER)
    {
        mi::neuraylib::Mdl::LogError("[ConvertArrayValue] Parameter reference expression not supported");
    }
    else if (expression->get_kind() == mi::neuraylib::IExpression::EK_DIRECT_CALL)
    {
        mi::neuraylib::Mdl::LogError("[ConvertArrayValue] Direct call expression not supported");
    }
    else if (expression->get_kind() == mi::neuraylib::IExpression::EK_TEMPORARY)
    {
        mi::neuraylib::Mdl::LogError("[ConvertArrayValue] Temporary reference expression not supported");
    }
    return VtValue();
}

bool GetInfoFromMatrix(
    const mi::neuraylib::IValue_matrix * matrix
    , mi::Size & matrixSize
    , mi::Size & vectorSize
    , mi::neuraylib::IType::Kind & elementKind
)
{
    if (matrix)
    {
        matrixSize = matrix->get_size();
        mi::base::Handle<const mi::neuraylib::IType_matrix> matrixtype(matrix->get_type());
        if (matrixtype)
        {
            mi::base::Handle<const mi::neuraylib::IType_vector> vectype(matrixtype->get_element_type());

            if (vectype)
            {
                vectorSize = vectype->get_size();

                mi::base::Handle<const mi::neuraylib::IType_atomic> elttype(vectype->get_element_type());

                if (elttype)
                {
                    elementKind = elttype->get_kind();
                    return true;
                }
            }
        }
    }
    return false;
}

template<class MdlType, class OutType> OutType GetGfValueFromMatrixInternal(const mi::neuraylib::IValue_matrix * matrix)
{
    OutType output;

    mi::Size matrixSize;
    mi::Size vectorSize;
    mi::neuraylib::IType::Kind elementKind;
    if (GetInfoFromMatrix(matrix, matrixSize, vectorSize, elementKind))
    {
        // column-major order
        for (mi::Size column = 0; column < matrixSize; column++)
        {
            mi::base::Handle<const mi::neuraylib::IValue_vector> v(matrix->get_value(column));
            if (v)
            {
                for (mi::Size row = 0; row < vectorSize; row++)
                {
                    mi::base::Handle<const MdlType> a(v->get_value<MdlType>(row));
                    output.GetArray()[column * matrixSize + row] = a->get_value();
                }
            }
        }
    }
    return output;
}

template<class MdlType, class OutType> VtValue GetValueFromMatrixInternal(const mi::neuraylib::IValue_matrix * matrix)
{
    return VtValue(GetGfValueFromMatrixInternal<MdlType, OutType>(matrix));
}

// GetValueFromNonSquareMatrixInternal()
//
// MdlType : mi::neuraylib::IValue_float, ...
// OutVecType : GfVec2f, ...
template<class MdlType, class OutVecType> VtValue GetValueFromNonSquareMatrixInternal(const mi::neuraylib::IValue_matrix * matrix
    , mi::Size matsize
    , mi::Size vesize)
{
    VtArray<OutVecType> output;

    // column-major order
    for (mi::Size column = 0; column < matsize; column++)
    {
        mi::base::Handle<const mi::neuraylib::IValue_vector> v(matrix->get_value(column));
        if (v)
        {
            OutVecType val;
            for (mi::Size row = 0; row < vesize; row++)
            {
                mi::base::Handle<const MdlType> a(v->get_value<MdlType>(row));
                val.data()[row] = a->get_value();
            }
            output.push_back(val);
        }
    }
    return VtValue(output);
}

template<class MdlType> VtValue GetValueFromMatrixFromMdlType(const mi::neuraylib::IValue_matrix * matrix, MdlToUsd::ExtraInfo & extra)
{
    mi::Size matrixSize;
    mi::Size vectorSize;
    mi::neuraylib::IType::Kind elementKind;
    if (GetInfoFromMatrix(matrix, matrixSize, vectorSize, elementKind))
    {
        std::stringstream matrixType;
        matrixType << ((mi::neuraylib::IType::TK_DOUBLE == elementKind) ? "double" : "float");
        matrixType << matrixSize << "x" << vectorSize;
        if (vectorSize == matrixSize)
        {
            // USD has only matrix of double, therefore for float matrices,
            // we need to store the original MDL type.
            if (elementKind == mi::neuraylib::IType::TK_FLOAT)
            {
                extra.SetMdlType(matrixType.str());
            }
            switch (matrixSize)
            {
            case 2:
                return GetValueFromMatrixInternal<MdlType, GfMatrix2d>(matrix);
            case 3:
                return GetValueFromMatrixInternal<MdlType, GfMatrix3d>(matrix);
            case 4:
                return GetValueFromMatrixInternal<MdlType, GfMatrix4d>(matrix);
            default:
                break;
            }
        }
        else
        {
            // Non square matrix
            switch (elementKind)
            {
            case mi::neuraylib::IType::TK_FLOAT:
            {
                // Set matrix type since this information is lost during matrix -> array conversion
                extra.SetMdlType(matrixType.str());

                switch (vectorSize)
                {
                case 2:
                    return GetValueFromNonSquareMatrixInternal<MdlType, GfVec2f>(matrix, matrixSize, vectorSize);
                case 3:
                    return GetValueFromNonSquareMatrixInternal<MdlType, GfVec3f>(matrix, matrixSize, vectorSize);
                case 4:
                    return GetValueFromNonSquareMatrixInternal<MdlType, GfVec4f>(matrix, matrixSize, vectorSize);
                default:
                    break;
                }
                break;
            }
            case mi::neuraylib::IType::TK_DOUBLE:
            {
                extra.SetMdlType(matrixType.str());

                switch (vectorSize)
                {
                case 2:
                    return GetValueFromNonSquareMatrixInternal<MdlType, GfVec2d>(matrix, matrixSize, vectorSize);
                case 3:
                    return GetValueFromNonSquareMatrixInternal<MdlType, GfVec3d>(matrix, matrixSize, vectorSize);
                case 4:
                    return GetValueFromNonSquareMatrixInternal<MdlType, GfVec4d>(matrix, matrixSize, vectorSize);
                default:
                    break;
                }
                break;
            }
            default:
                break;
            }
        }
    }
    return VtValue();
}

VtValue MdlToUsd::GetValueFromMatrix(const IExpression * expression, ExtraInfo & extra) const
{
    if (expression->get_kind() == mi::neuraylib::IExpression::EK_CONSTANT)
    {
        mi::base::Handle<const mi::neuraylib::IExpression_constant> constant(expression->get_interface<mi::neuraylib::IExpression_constant>());
        if (constant)
        {
            mi::base::Handle<const mi::neuraylib::IValue_matrix> matrix(constant->get_value<mi::neuraylib::IValue_matrix>());
            mi::Size matrixSize;
            mi::Size vectorSize;
            mi::neuraylib::IType::Kind elementKind;
            if (matrix && GetInfoFromMatrix(matrix.get(), matrixSize, vectorSize, elementKind))
            {
                // Note: There is no MDL matrix for integer
                if (mi::neuraylib::IType::TK_FLOAT == elementKind)
                {
                    return GetValueFromMatrixFromMdlType<mi::neuraylib::IValue_float>(matrix.get(), extra);
                }
                else if (mi::neuraylib::IType::TK_DOUBLE == elementKind)
                {
                    return GetValueFromMatrixFromMdlType<mi::neuraylib::IValue_double>(matrix.get(), extra);
                }
            }
        }
    }
    mi::neuraylib::Mdl::LogError("[MdlToUsd] Matrix not supported");
    return VtValue();
}

template<class GfType, class MdlType> VtValue GetGfValueFromArrayOfMatrix(const mi::neuraylib::IValue_compound * compound, MdlToUsd::ExtraInfo & extra)
{
    VtArray<GfType> rtn;

    if (compound)
    {
        for (mi::Size i = 0; i < compound->get_size(); i++)
        {
            mi::base::Handle<const mi::neuraylib::IValue_matrix> matrix(compound->get_value<mi::neuraylib::IValue_matrix>(i));
            mi::Size matrixSize;
            mi::Size vectorSize;
            mi::neuraylib::IType::Kind elementKind;
            if (matrix && GetInfoFromMatrix(matrix.get(), matrixSize, vectorSize, elementKind))
            {
                GfType value = GetGfValueFromMatrixInternal<MdlType, GfType>(matrix.get());
                rtn.push_back(value);
            }
        }
    }

    return VtValue(rtn);
}

// OutType: double, float...
// MdlType: e.g. IValue_double
template<class OutType, class MdlType> VtValue GetVtArrayFromArrayOfMatrix(const mi::neuraylib::IValue_compound * compound)
{
    VtArray<OutType> output;

    for (mi::Size matrix_index = 0; matrix_index < compound->get_size(); matrix_index++)
    {
        mi::base::Handle<const mi::neuraylib::IValue_matrix> matrix(compound->get_value<mi::neuraylib::IValue_matrix>(matrix_index));
        mi::Size matrixSize;
        mi::Size vectorSize;
        mi::neuraylib::IType::Kind elementKind;
        if (matrix && GetInfoFromMatrix(matrix.get(), matrixSize, vectorSize, elementKind))
        {
            // column-major order
            for (mi::Size column = 0; column < matrixSize; column++)
            {
                mi::base::Handle<const mi::neuraylib::IValue_vector> v(matrix->get_value(column));
                if (v)
                {
                    for (mi::Size row = 0; row < vectorSize; row++)
                    {
                        mi::base::Handle<const MdlType> a(v->get_value<MdlType>(row));
                        output.push_back(a->get_value());
                    }
                }
            }
        }
    }
    return VtValue(output);
}

VtValue GetValueFromArrayOfMatrix(const IExpression * expression, MdlToUsd::ExtraInfo & extra)
{
    if (expression->get_kind() == mi::neuraylib::IExpression::EK_CONSTANT)
    {
        mi::base::Handle<const mi::neuraylib::IExpression_constant> constant(expression->get_interface<mi::neuraylib::IExpression_constant>());
        if (constant)
        {
            mi::base::Handle<const mi::neuraylib::IValue_array> valuearray(constant->get_value<mi::neuraylib::IValue_array>());
            if (valuearray)
            {
                mi::base::Handle<const mi::neuraylib::IValue_compound> compound(valuearray->get_interface<mi::neuraylib::IValue_compound>());
                if (compound)
                {
                    if ( compound->get_size() > 0 )
                    {
                        mi::base::Handle<const mi::neuraylib::IValue_matrix> matrix(compound->get_value<mi::neuraylib::IValue_matrix>(0));
                        
                        mi::Size matrixSize;
                        mi::Size vectorSize;
                        mi::neuraylib::IType::Kind elementKind;
                        if (matrix && GetInfoFromMatrix(matrix.get(), matrixSize, vectorSize, elementKind))
                        {
                            std::stringstream matrixType;
                            matrixType << ((mi::neuraylib::IType::TK_DOUBLE == elementKind) ? "double" : "float");
                            matrixType << matrixSize << "x" << vectorSize << "[" << compound->get_size() << "]";

                            if (vectorSize == matrixSize)
                            {
                                if (mi::neuraylib::IType::TK_FLOAT == elementKind)
                                {
                                    extra.SetMdlType(matrixType.str());

                                    switch (matrixSize)
                                    {
                                    case 2:
                                        return GetGfValueFromArrayOfMatrix<GfMatrix2d, IValue_float>(compound.get(), extra);
                                        break;
                                    case 3:
                                        return GetGfValueFromArrayOfMatrix<GfMatrix3d, IValue_float>(compound.get(), extra);
                                        break;
                                    case 4:
                                        return GetGfValueFromArrayOfMatrix<GfMatrix4d, IValue_float>(compound.get(), extra);
                                        break;
                                    default:
                                        break;
                                    }
                                }
                                else if (mi::neuraylib::IType::TK_DOUBLE == elementKind)
                                {
                                    switch (matrixSize)
                                    {
                                    case 2:
                                        return GetGfValueFromArrayOfMatrix<GfMatrix2d, IValue_double>(compound.get(), extra);
                                        break;
                                    case 3:
                                        return GetGfValueFromArrayOfMatrix<GfMatrix3d, IValue_double>(compound.get(), extra);
                                        break;
                                    case 4:
                                        return GetGfValueFromArrayOfMatrix<GfMatrix4d, IValue_double>(compound.get(), extra);
                                        break;
                                    default:
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                // Non-square matrix
                                if (mi::neuraylib::IType::TK_FLOAT == elementKind)
                                {
                                    extra.SetMdlType(matrixType.str());
                                    return GetVtArrayFromArrayOfMatrix<float, IValue_float>(compound.get());
                                }
                                else if (mi::neuraylib::IType::TK_DOUBLE == elementKind)
                                {
                                    extra.SetMdlType(matrixType.str());
                                    return GetVtArrayFromArrayOfMatrix<double, IValue_double>(compound.get());
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    mi::neuraylib::Mdl::LogError("[MdlToUsd] Matrix not supported");
    return VtValue();
}

// Utility function to dump the arguments of a material instance or function call.
template <class T>
void dump_instance(
    mi::neuraylib::IExpression_factory* expression_factory, const T* material, std::ostream& s)
{
    mi::Size count = material->get_parameter_count();
    mi::base::Handle<const mi::neuraylib::IExpression_list> arguments(material->get_arguments());

    for (mi::Size index = 0; index < count; index++) {

        mi::base::Handle<const mi::neuraylib::IExpression> argument(
            arguments->get_expression(index));
        std::string name = material->get_parameter_name(index);
        mi::base::Handle<const mi::IString> argument_text(
            expression_factory->dump(argument.get(), name.c_str(), 1));
        s << argument_text->get_c_str() << std::endl;

    }
    s << std::endl;
}


struct AnnoHelper
{
    enum ANNOTATION_FAMILY
    {
        UNKNONW = -1
        , DISPLAY_NAME
        , DESCRIPTION
        , IN_GROUP
        , HARD_RANGE
        , SOFT_RANGE
        , MODIFIED
        , CREATED
        , AUTHOR
        , CONTRIBUTOR
        , COPYRIGHT_NOTICE
        , KEY_WORDS
        , HIDDEN
        , UNUSED
        , DEPRECATED
        , USAGE
        , ORIGIN
        , ENABLE_IF
        , UI_ORDER
        , THUMBNAIL
        , VERSION
        , DEPENDENCY

    } m_family = UNKNONW;
    typedef std::map<std::string, ANNOTATION_FAMILY> ANNOTATION_FAMILY_MAP;
    ANNOTATION_FAMILY_MAP m_AnnotationFamilyMap =
    {
        { "display_name", DISPLAY_NAME }
        ,{ "description", DESCRIPTION }
        ,{ "in_group", IN_GROUP }
        ,{ "hard_range", HARD_RANGE }
        ,{ "soft_range", SOFT_RANGE }
        ,{ "modified", MODIFIED }
        ,{ "created", CREATED }
        ,{ "author", AUTHOR }
        ,{ "contributor", CONTRIBUTOR }
        ,{ "copyright_notice", COPYRIGHT_NOTICE }
        ,{ "key_words", KEY_WORDS }
        ,{ "hidden", HIDDEN }
        ,{ "unused", UNUSED }
        ,{ "deprecated", DEPRECATED }
        ,{ "usage", USAGE }
        ,{ "origin", ORIGIN }
        ,{ "enable_if", ENABLE_IF }
        ,{ "ui_order", UI_ORDER }
        ,{ "thumbnail", THUMBNAIL }
        ,{ "version", VERSION }
        ,{ "dependency", DEPENDENCY }
    };
    typedef std::map<std::string, mi::neuraylib::IType::Kind> KIND_MAP;
    KIND_MAP m_KindMap =
    {
        { "int", mi::neuraylib::IType::TK_INT }
        ,{ "float", mi::neuraylib::IType::TK_FLOAT }
        ,{ "double", mi::neuraylib::IType::TK_DOUBLE }
        ,{ "color", mi::neuraylib::IType::TK_COLOR }
        ,{ "float2", mi::neuraylib::IType::TK_VECTOR }
        ,{ "float3", mi::neuraylib::IType::TK_VECTOR }
        ,{ "float4", mi::neuraylib::IType::TK_VECTOR }
        ,{ "int2", mi::neuraylib::IType::TK_VECTOR }
        ,{ "int3", mi::neuraylib::IType::TK_VECTOR }
        ,{ "int4", mi::neuraylib::IType::TK_VECTOR }
        ,{ "double2", mi::neuraylib::IType::TK_VECTOR }
        ,{ "double3", mi::neuraylib::IType::TK_VECTOR }
        ,{ "double4", mi::neuraylib::IType::TK_VECTOR }
    };
    mi::neuraylib::IType::Kind m_kind;
    typedef std::map<std::string, mi::neuraylib::IType::Kind> ATOMIC_KIND_MAP;
    ATOMIC_KIND_MAP m_AtomicKindMap =
    {
        { "float2", mi::neuraylib::IType::TK_FLOAT }
        ,{ "float3", mi::neuraylib::IType::TK_FLOAT }
        ,{ "float4", mi::neuraylib::IType::TK_FLOAT }
        ,{ "int2", mi::neuraylib::IType::TK_INT }
        ,{ "int3", mi::neuraylib::IType::TK_INT }
        ,{ "int4", mi::neuraylib::IType::TK_INT }
        ,{ "double2", mi::neuraylib::IType::TK_DOUBLE }
        ,{ "double3", mi::neuraylib::IType::TK_DOUBLE }
        ,{ "double4", mi::neuraylib::IType::TK_DOUBLE }
    };
    mi::neuraylib::IType::Kind m_atomic_kind;
    std::string m_annotation;
    mi::base::Handle<const mi::neuraylib::IExpression_list> m_elist;
    AnnoHelper(const std::string & annotation, const mi::neuraylib::IExpression_list * exprlist)
        : m_annotation(annotation), m_elist(exprlist)
    {
        parse_annotation();
    }
    void parse_annotation()
    {
        //e.g. "::anno::hard_range(float,float)"
        std::vector<std::string> tokens = TfStringSplit(m_annotation, "::");
        if (tokens.size() < 3)
        {
            return;
        }
        //e.g. "" "anno" "hard_range(float,float)"
        std::vector<std::string> tokens2 = TfStringSplit(tokens[2], "(");

        if (tokens2.empty())
        {
            return;
        }
        //mi::neuraylib::Mdl::LogVerbose("[dump] Annotation family: " + tokens2[0]);
        const ANNOTATION_FAMILY_MAP::iterator it(m_AnnotationFamilyMap.find(tokens2[0]));
        if (it != m_AnnotationFamilyMap.end())
        {
            m_family = it->second;
        }
        if (tokens2.size() < 2)
        {
            return;
        }
        //e.g. "hard_range" "float,float)"
        const std::vector<std::string> tokens3 = TfStringSplit(tokens2[1], ")");
        if (tokens3.empty())
        {
            return;
        }
        //e.g. "float,float" ""
        const std::vector<std::string> tokens4 = TfStringSplit(tokens3[0], ",");
        if (!tokens4.empty())
        {
            const KIND_MAP::iterator it(m_KindMap.find(tokens4[0]));
            if (it != m_KindMap.end())
            {
                m_kind = it->second;

                if (mi::neuraylib::IType::TK_VECTOR == m_kind)
                {
                    const ATOMIC_KIND_MAP::iterator it(m_AtomicKindMap.find(tokens4[0]));
                    if (it != m_AtomicKindMap.end())
                    {
                        m_atomic_kind = it->second;
                    }
                }
            }
        }
        for (auto & t : tokens4)
        {
            //mi::neuraylib::Mdl::LogVerbose("[dump] Annotation type: " + t);
        }
    }
    bool getAnnotationColor(const char * name, std::vector<float> & rgb) const
    {
        mi::base::Handle<const mi::neuraylib::IExpression> expr(m_elist->get_expression(name));
        if (expr)
        {
            mi::base::Handle<const mi::neuraylib::IExpression_constant> constant(expr->get_interface<mi::neuraylib::IExpression_constant>());
            if (constant)
            {
                mi::base::Handle<const mi::neuraylib::IValue_color> v(constant->get_value<mi::neuraylib::IValue_color>());
                if (v)
                {
                    mi::base::Handle<const mi::neuraylib::IValue_float> r(v->get_value(0));
                    mi::base::Handle<const mi::neuraylib::IValue_float> g(v->get_value(1));
                    mi::base::Handle<const mi::neuraylib::IValue_float> b(v->get_value(2));
                    rgb.emplace_back(r->get_value());
                    rgb.emplace_back(g->get_value());
                    rgb.emplace_back(b->get_value());
                    return true;
                }
            }
        }
        return false;
    }
    template<class TYPE, class NRTYPE> bool getAnnotation(const char * name, TYPE & out) const
    {
        mi::base::Handle<const mi::neuraylib::IExpression> expr(m_elist->get_expression(name));
        if (expr)
        {
            mi::base::Handle<const mi::neuraylib::IExpression_constant> constant(expr->get_interface<mi::neuraylib::IExpression_constant>());
            if (constant)
            {
                mi::base::Handle<const NRTYPE> value(constant->get_value<NRTYPE>());
                if (value)
                {
                    out = value->get_value();
                    return true;
                }
            }
        }
        return false;
    }
    template<class TYPE, class NRTYPE> bool getAnnotationString(const char * name, std::string & out) const
    {
        TYPE tmp;
        if (getAnnotation<TYPE, NRTYPE>(name, tmp))
        {
            std::stringstream str;
            str << tmp;
            out = str.str();
            return true;
        }
        return false;
    }
    template<class TYPE, class NRTYPE> bool getAnnotationVector(const char * name, std::vector<TYPE> & out) const
    {
        auto getElements = [&out](const mi::neuraylib::IValue_compound * compound)
        {
            if (compound)
            {
                for (mi::Size i = 0; i < compound->get_size(); i++)
                {
                    mi::base::Handle<const NRTYPE> value(compound->get_value<NRTYPE>(i));
                    if (value)
                    {
                        TYPE v = value->get_value();
                        out.emplace_back(v);
                    }
                }
                return true;
            }
            return false;
        };

        mi::base::Handle<const mi::neuraylib::IExpression> expr(m_elist->get_expression(name));
        if (expr)
        {
            mi::base::Handle<const mi::neuraylib::IExpression_constant> constant(expr->get_interface<mi::neuraylib::IExpression_constant>());
            if (constant)
            {
                mi::base::Handle<const mi::neuraylib::IValue_vector> vec(constant->get_value<mi::neuraylib::IValue_vector>());

                if (vec)
                {
                    mi::base::Handle<const mi::neuraylib::IValue_compound> compound(vec->get_interface<mi::neuraylib::IValue_compound>());
                    return getElements(compound.get());
                }
                else
                {
                    mi::base::Handle<const mi::neuraylib::IValue_array> arr(constant->get_value<mi::neuraylib::IValue_array>());
                    if (arr)
                    {
                        mi::base::Handle<const mi::neuraylib::IValue_compound> compound(arr->get_interface<mi::neuraylib::IValue_compound>());
                        return getElements(compound.get());
                    }
                }
            }
        }
        return false;
    }
    template<class TYPE, class NRTYPE> bool getMinMaxRange(TYPE & min, TYPE & max) const
    {
        if (getAnnotation<TYPE, NRTYPE>("min", min) && getAnnotation<TYPE, NRTYPE>("max", max))
        {
            return true;
        }
        return false;
    }
    template<class TYPE, class NRTYPE> bool getMinMaxRangeVector(std::vector<TYPE> & min, std::vector<TYPE> & max) const
    {
        if (getAnnotationVector<TYPE, NRTYPE>("min", min) && getAnnotationVector<TYPE, NRTYPE>("max", max))
        {
            return true;
        }
        return false;
    }

    bool getMinMaxRangeColor(std::string & min, std::string & max) const
    {
        std::vector<float> rgb;
        if (getAnnotationColor("min", rgb))
        {
            vecToString<float>(rgb, min);
            rgb.clear();
            if (getAnnotationColor("max", rgb))
            {
                vecToString<float>(rgb, max);
                return true;
            }
        }
        return false;
    }

    template<class TYPE> void vecToString(const std::vector<TYPE> & vec, std::string & out) const
    {
        std::stringstream str;
        str << "(";
        str << vec[0];
        for (int i = 1; i < vec.size(); i++)
        {
            str << ", " << vec[i];
        }
        str << ")";
        out = str.str();
    }

    template<class TYPE, class NRTYPE> bool getMinMaxRange(std::string & min, std::string & max) const
    {
        TYPE minv, maxv;
        if (getMinMaxRange<TYPE, NRTYPE>(minv, maxv))
        {
            {
                std::stringstream str;
                str << minv;
                min = str.str();
            }
            {
                std::stringstream str;
                str << maxv;
                max = str.str();
            }
            return true;
        }
        return false;
    }
    template<class TYPE, class NRTYPE> bool getMinMaxRangeVector(std::string & min, std::string & max) const
    {
        if (getAnnotationVectorToString<TYPE, NRTYPE>("min", min) && getAnnotationVectorToString<TYPE, NRTYPE>("max", max))
        {
            return true;
        }
        return false;
    }
    template<class TYPE, class NRTYPE> bool getAnnotationVectorToString(const std::string & name, std::string & out) const
    {
        std::vector<TYPE> tmpvec;
        if (getAnnotationVector<TYPE, NRTYPE>(name.c_str(), tmpvec))
        {
            vecToString<TYPE>(tmpvec, out);
            return true;
        }
        return false;
    }
    bool getMinMaxRange(std::string & min, std::string & max) const
    {
        if (m_kind == mi::neuraylib::IType::TK_INT)
        {
            return getMinMaxRange<int, mi::neuraylib::IValue_int>(min, max);
        }
        if (m_kind == mi::neuraylib::IType::TK_FLOAT)
        {
            return getMinMaxRange<float, mi::neuraylib::IValue_float>(min, max);
        }
        if (m_kind == mi::neuraylib::IType::TK_COLOR)
        {
            return getMinMaxRangeColor(min, max);
        }
        if (m_kind == mi::neuraylib::IType::TK_VECTOR)
        {
            if (m_atomic_kind == mi::neuraylib::IType::TK_INT)
            {
                return getMinMaxRangeVector<int, mi::neuraylib::IValue_int>(min, max);
            }
            if (m_atomic_kind == mi::neuraylib::IType::TK_FLOAT)
            {
                return getMinMaxRangeVector<float, mi::neuraylib::IValue_float>(min, max);
            }
            if (m_atomic_kind == mi::neuraylib::IType::TK_DOUBLE)
            {
                return getMinMaxRangeVector<double, mi::neuraylib::IValue_double>(min, max);
            }
        }
        return false;
    }
    void dump()
    {
        for (mi::Size i = 0; i < m_elist->get_size(); i++)
        {
            std::string str("[dump] Expression name: " + std::string(m_elist->get_name(i)));
            mi::neuraylib::Mdl::LogDebug(str.c_str());
            mi::base::Handle<const mi::neuraylib::IExpression> expression(m_elist->get_expression(i));
            if (expression)
            {
                mi::base::Handle<const mi::neuraylib::IType> type(expression->get_type());
                mi::neuraylib::IType::Kind k = type->get_kind();
                std::stringstream str;
                str << k;
                mi::neuraylib::Mdl::LogDebug("[dump] Expression kind: " + str.str());
                mi::base::Handle<const mi::neuraylib::IExpression_constant> constant(expression->get_interface<mi::neuraylib::IExpression_constant>());
                if (constant)
                {

                }
            }
        }
    }

    bool is_a(const ANNOTATION_FAMILY & family)
    {
        return m_family == family;
    }

    bool is_kind(const mi::neuraylib::IType::Kind & kind)
    {
        return m_kind == kind;
    }
};

void MdlToUsd::extractAnnotations(const mi::neuraylib::IAnnotation_block * annotations, std::map<std::string, std::string> & metadata)
{
    auto addAnnotation = [&metadata](const char * annotation_name, const mi::neuraylib::IExpression_list * exprlist)
    {
        if (annotation_name == NULL || exprlist == NULL)
        {
            return;
        }
        std::string stringanno(annotation_name);
        AnnoHelper anno(std::string(annotation_name), exprlist);
        if (anno.is_a(AnnoHelper::DISPLAY_NAME))
        {
            std::string stringanno;
            if (anno.getAnnotation<std::string, mi::neuraylib::IValue_string>("name", stringanno))
            {
                metadata[UsdUITokens->uiDisplayName.GetString().c_str()] = stringanno;
            }
        }
        else if (anno.is_a(AnnoHelper::DESCRIPTION))
        {
            std::string stringanno;
            if (anno.getAnnotation<std::string, mi::neuraylib::IValue_string>("description", stringanno))
            {
                metadata[UsdUITokens->uiDescription.GetString().c_str()] = stringanno;
            }
        }
        else if (anno.is_a(AnnoHelper::IN_GROUP))
        {
            std::string stringanno;
            if (anno.getAnnotation<std::string, mi::neuraylib::IValue_string>("group", stringanno))
            {
                metadata[UsdUITokens->uiDisplayGroup.GetString().c_str()] = stringanno;
            }
#ifndef ANNOTATIONS_AS_METADATA
        }
#else //ANNOTATIONS_AS_METADATA
            if (anno.getAnnotation<std::string, mi::neuraylib::IValue_string>("subgroup", stringanno))
            {
                metadata["displaySubGroup"] = stringanno;
            }
            if (anno.getAnnotation<std::string, mi::neuraylib::IValue_string>("subsubgroup", stringanno))
            {
                metadata["displaySubSubGroup"] = stringanno;
            }
        }
        else if (anno.is_a(AnnoHelper::AUTHOR))
        {
            std::string stringanno;
            if (anno.getAnnotation<std::string, mi::neuraylib::IValue_string>("name", stringanno))
            {
                metadata["author"] = stringanno;
            }
        }
        else if (anno.is_a(AnnoHelper::CONTRIBUTOR))
        {
            std::string stringanno;
            if (anno.getAnnotation<std::string, mi::neuraylib::IValue_string>("name", stringanno))
            {
                metadata["contributor"] = stringanno;
            }
        }
        else if (anno.is_a(AnnoHelper::USAGE))
        {
            std::string stringanno;
            if (anno.getAnnotation<std::string, mi::neuraylib::IValue_string>("hint", stringanno))
            {
                metadata["usage"] = stringanno;
            }
        }
        else if (anno.is_a(AnnoHelper::ORIGIN))
        {
            std::string stringanno;
            if (anno.getAnnotation<std::string, mi::neuraylib::IValue_string>("name", stringanno))
            {
                metadata["origin"] = stringanno;
            }
        }
        else if (anno.is_a(AnnoHelper::COPYRIGHT_NOTICE))
        {
            std::string stringanno;
            if (anno.getAnnotation<std::string, mi::neuraylib::IValue_string>("copyright", stringanno))
            {
                metadata["copyrightNotice"] = stringanno;
            }
        }
        else if (anno.is_a(AnnoHelper::KEY_WORDS))
        {
            std::string stringanno;
            if (anno.getAnnotationVectorToString<std::string, mi::neuraylib::IValue_string>("words", stringanno))
            {
                metadata["keywords"] = stringanno;
            }
        }
        else if (anno.is_a(AnnoHelper::ENABLE_IF))
        {
            std::string stringanno;
            if (anno.getAnnotation<std::string, mi::neuraylib::IValue_string>("condition", stringanno))
            {
                metadata["enableIf"] = stringanno;
            }
        }
        else if (anno.is_a(AnnoHelper::UI_ORDER))
        {
            std::string stringanno;
            if (anno.getAnnotationString<int, mi::neuraylib::IValue_int>("order", stringanno))
            {
                metadata["uiOrder"] = stringanno;
            }
        }
        else if (anno.is_a(AnnoHelper::THUMBNAIL))
        {
            std::string stringanno;
            if (anno.getAnnotationString<string, mi::neuraylib::IValue_string>("name", stringanno))
            {
                metadata["thumbnail"] = stringanno;
            }
        }
        else if (anno.is_a(AnnoHelper::HIDDEN))
        {
            metadata["hidden"] = "";
        }
        else if (anno.is_a(AnnoHelper::UNUSED))
        {
            std::string stringanno;
            if (anno.getAnnotation<std::string, mi::neuraylib::IValue_string>("description", stringanno))
            {
                metadata["unused"] = stringanno;
            }
            else
            {
                metadata["unused"] = "";
            }
        }
        else if (anno.is_a(AnnoHelper::DEPRECATED))
        {
            std::string stringanno;
            // This annotation used to be wrongly called "message". Fixed in r325885 (12/5/2019)
            if (anno.getAnnotation<std::string, mi::neuraylib::IValue_string>("description", stringanno))
            {
                metadata["deprecated"] = stringanno;
            }
            else
            {
                anno.dump();

                metadata["deprecated"] = "";
            }
        }
        else if (anno.is_a(AnnoHelper::DEPENDENCY))
        {
            std::string stringanno;
            if (anno.getAnnotationString<std::string, mi::neuraylib::IValue_string>("module_name", stringanno))
            {
                metadata["dependencyModulename"] = stringanno;
            }
            if (anno.getAnnotationString<int, mi::neuraylib::IValue_int>("major", stringanno))
            {
                metadata["dependencyMajor"] = stringanno;
            }
            if (anno.getAnnotationString<int, mi::neuraylib::IValue_int>("minor", stringanno))
            {
                metadata["dependencyMinor"] = stringanno;
            }
            if (anno.getAnnotationString<int, mi::neuraylib::IValue_int>("patch", stringanno))
            {
                metadata["dependencyPatch"] = stringanno;
            }
            if (anno.getAnnotationString<std::string, mi::neuraylib::IValue_string>("prerelease", stringanno))
            {
                metadata["dependencyPrerelease"] = stringanno;
            }
        }
        else if (anno.is_a(AnnoHelper::VERSION))
        {
            std::string stringanno;
            if (anno.getAnnotationString<int, mi::neuraylib::IValue_int>("major", stringanno))
            {
                metadata["versionMajor"] = stringanno;
            }
            if (anno.getAnnotationString<int, mi::neuraylib::IValue_int>("minor", stringanno))
            {
                metadata["versionMinor"] = stringanno;
            }
            if (anno.getAnnotationString<int, mi::neuraylib::IValue_int>("patch", stringanno))
            {
                metadata["versionPatch"] = stringanno;
            }
            if (anno.getAnnotationString<std::string, mi::neuraylib::IValue_string>("prerelease", stringanno))
            {
                metadata["versionPrerelease"] = stringanno;
            }
        }
        else if (anno.is_a(AnnoHelper::HARD_RANGE))
        {
            std::string min, max;
            if (anno.getMinMaxRange(min, max))
            {
                metadata["hardRangeMin"] = min;
                metadata["hardRangeMax"] = max;
            }
        }
        else if (anno.is_a(AnnoHelper::SOFT_RANGE))
        {
            std::string min, max;
            if (anno.getMinMaxRange(min, max))
            {
                metadata["softRangeMin"] = min;
                metadata["softRangeMax"] = max;
            }
        }
        else if (anno.is_a(AnnoHelper::MODIFIED))
        {
            std::string stringanno;
            if (anno.getAnnotationString<int, mi::neuraylib::IValue_int>("year", stringanno))
            {
                metadata["modifiedYear"] = stringanno;
            }
            if (anno.getAnnotationString<int, mi::neuraylib::IValue_int>("month", stringanno))
            {
                metadata["modifiedMonth"] = stringanno;
            }
            if (anno.getAnnotationString<int, mi::neuraylib::IValue_int>("day", stringanno))
            {
                metadata["modifiedDay"] = stringanno;
            }
            if (anno.getAnnotationString<std::string, mi::neuraylib::IValue_string>("notes", stringanno))
            {
                metadata["modifiedNotes"] = stringanno;
            }
        }
        else if (anno.is_a(AnnoHelper::CREATED))
        {
            std::string stringanno;
            if (anno.getAnnotationString<int, mi::neuraylib::IValue_int>("year", stringanno))
            {
                metadata["createdYear"] = stringanno;
            }
            if (anno.getAnnotationString<int, mi::neuraylib::IValue_int>("month", stringanno))
            {
                metadata["createdMonth"] = stringanno;
            }
            if (anno.getAnnotationString<int, mi::neuraylib::IValue_int>("day", stringanno))
            {
                metadata["createdDay"] = stringanno;
            }
            if (anno.getAnnotationString<std::string, mi::neuraylib::IValue_string>("notes", stringanno))
            {
                metadata["createdNotes"] = stringanno;
            }
        }
        else
        {
            mi::neuraylib::Mdl::LogWarning("[extractAnnotations] Annotation not handled " + stringanno);
            anno.dump();
        }
#endif //ANNOTATIONS_AS_METADATA
    };

    if (annotations)
    {
        for (mi::Size annoi = 0; annoi < annotations->get_size(); annoi++)
        {
            mi::base::Handle<const mi::neuraylib::IAnnotation> anno(annotations->get_annotation(annoi));
            if (anno)
            {
                const char* annoname(anno->get_name());
                mi::base::Handle<const mi::neuraylib::IExpression_list> annoexpr(anno->get_arguments());
                addAnnotation(annoname, annoexpr.get());
            }
        }
    }
}

void MdlToUsd::extractAnnotations(const std::string & parmName, const mi::neuraylib::IAnnotation_list * annotations, std::map<std::string, std::string> & metadata)
{
    if (annotations)
    {
        mi::base::Handle<const mi::neuraylib::IAnnotation_block> block(annotations->get_annotation_block(parmName.c_str()));
        if (block)
        {
            MdlToUsd::extractAnnotations(block.get(), metadata);
        }
    }
}

void MdlToUsd::extractAnnotations(const mi::neuraylib::IAnnotation_block * annotations, std::map<std::string, VtValue> & metadata)
{
    if (annotations)
    {
        for (mi::Size annoi = 0; annoi < annotations->get_size(); annoi++)
        {
            mi::base::Handle<const mi::neuraylib::IAnnotation> anno(annotations->get_annotation(annoi));
            if (anno)
            {
                mi::base::Handle<const mi::neuraylib::IExpression_list> annoexpr(anno->get_arguments());
                if (annoexpr)
                {
                    if (annoexpr->get_size() == 0)
                    {
                        // Special case of annotations without expressions, e.g. ::anno::hidden()
                        // We create a boolean VtValue and set it to true
                        // Annotation:
                        //   ::anno::hidden()
                        // becomes:
                        //   bool "::anno::hidden()" = 1
                        VtValue value(true);
                        const char* annotation_name(anno->get_name());
                        if (annotation_name )
                        {
                            std::string data_name(annotation_name);
                            metadata[data_name.c_str()] = value;
                        }
                    }
                    for (mi::Size index = 0; index < annoexpr->get_size(); index++)
                    {
                        mi::base::Handle<const mi::neuraylib::IExpression> expression(annoexpr->get_expression(index));
                        if (expression)
                        {
                            mi::base::Handle<const mi::neuraylib::IType> type(expression->get_type());
                            if (type)
                            {
                                TfToken typeOut;
                                ExtraInfo extra;
                                MdlToUsd converter;
                                VtValue value(converter.GetValue(type.get(), expression.get(), typeOut, extra));

                                if (!value.IsEmpty())
                                {
                                    const char* annotation_name(anno->get_name());
                                    const char* expression_name(annoexpr->get_name(index));
                                    if (annotation_name && expression_name)
                                    {
                                        std::string data_name(annotation_name);
                                        data_name = data_name + std::string("::") + std::string(expression_name);
                                        metadata[data_name.c_str()] = value;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void MdlToUsd::extractAnnotations(const std::string & parmName, const mi::neuraylib::IAnnotation_list * annotations, std::map<std::string, VtValue> & metadata)
{
    if (annotations)
    {
        mi::base::Handle<const mi::neuraylib::IAnnotation_block> block(annotations->get_annotation_block(parmName.c_str()));
        if (block)
        {
            MdlToUsd::extractAnnotations(block.get(), metadata);
        }
    }
}

PXR_NAMESPACE_CLOSE_SCOPE
