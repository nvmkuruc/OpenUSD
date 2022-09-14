/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#include "pxr/pxr.h"
#include "pxr/usd/plugin/usdMdl/usdToMdl.h"
#include "pxr/usd/plugin/usdMdl/utils.h"
#include "pxr/usd/plugin/usdMdl/neuray.h"
#include <stack>

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

typedef enum
{
    Success = 0
    , UnknownError = -1
    , CanNotGetModuleFromMaterial = -2
    , CanNotLoadModule = -3
    , CanNotGetMaterialDefinitionFromMaterial = -4
    , CanNotGetMaterialDefinitionFromDB = -5
    , CanNotCreateMaterialInstance = -6
    , CanNotGetMaterialInstance = -7

} RETURN_CODE;

//////////////////////////////////////////////////////////////////////////
/// class DBHelper

class DBHelper
{
public:
    /// Same as ITransaction::store() but keeps track of what is stored
    mi::Sint32 store(
        mi::neuraylib::ITransaction * transaction
        , mi::base::IInterface* db_element
        , const char* name
        , mi::Uint8 privacy = mi::neuraylib::ITransaction::LOCAL_SCOPE
    );
    /// Cleanup
    bool cleanup(mi::neuraylib::ITransaction * transaction);
private:
    std::stack<std::string> m_elements;

} m_DBHelper;

mi::Sint32 DBHelper::store(
    mi::neuraylib::ITransaction * transaction
    , mi::base::IInterface* db_element
    , const char* name
    , mi::Uint8 privacy
)
{
    const mi::Sint32 rtn(transaction->store(db_element, name, privacy));
    if (rtn == 0)
    {
        m_elements.push(name);
    }
    return rtn;
}

bool DBHelper::cleanup(mi::neuraylib::ITransaction * transaction)
{
    std::string element;
    while (!m_elements.empty())
    {
        transaction->remove(m_elements.top().c_str());
        m_elements.pop();
    }
    return true;
}

/// class DBHelper
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/// class UsdToMdlConverter

class UsdToMdlConverter
{
public:
    /// Some settings to control the conversion
    struct Settings
    {
        /// Arbitrary prefix for the new converted material
        std::string m_materialInstanceName = "elt";

        /// If m_cleanupDB is set to true, the constructed elements are removed from DB when class is destroyed
        bool m_cleanupDB = false;
    };

public:
    /// ctor
    UsdToMdlConverter(mi::neuraylib::INeuray * neuray);
    ~UsdToMdlConverter();

    /// Convert()
    ///
    /// Utility routine to convert a USD material to a MDL material instance.
    /// Material instance is created with name given in Settings (see ctor).
    /// The instance is updated from the USD material.
    /// If m_settings.m_saveToModule flag is set the MDL material instance is saved in a new MDL module.
    ///
    /// material: Input material to convert
    ///
    int Convert(const UsdShadeMaterial & material, Settings & settings);

private:
    bool setup(mi::neuraylib::INeuray * neuray);
    bool finalize();

    /// ConvertSimpleValue()
    ///
    /// Convert input value and output an MDL expression
    ///
    /// expression_factory: Expression factory interface
    /// valueFactory: Value factory interface
    /// input: Input value to convert
    /// type: Type of the input value
    /// expression: Output value
    ///
    template <class T> bool ConvertSimpleValue(
        const mi::neuraylib::IExpression_factory * expressionFactory
        , mi::neuraylib::IValue_factory * valueFactory
        , const UsdShadeInput & input
        , const mi::neuraylib::IType * inputType
        , mi::base::Handle<mi::neuraylib::IExpression> & expression
    ) const;

    /// ConvertValue()
    /// 
    /// Convert input value and output an MDL expression
    /// 
    /// transaction: Input transaction
    /// input: Input value to convert
    /// type: Type of the input value
    /// expression: Output value
    ///
    bool ConvertValue(
        mi::neuraylib::ITransaction * transaction
        , const UsdShadeInput & input
        , const mi::neuraylib::IType * inputType
        , mi::base::Handle<mi::neuraylib::IExpression> & expression
    );

    /// UpdateMdlMaterialFromUSDMaterial()
    ///
    /// storedInstanceName: MDL material instance name stored in DB
    /// material: USD material used to update the MDL instance
    ///
    bool UpdateMdlMaterialFromUSDMaterial(
        mi::neuraylib::ITransaction * transaction
        , const std::string & storedInstanceName
        , const UsdShadeMaterial & material
    );

    /// CreateMdlMaterialInstanceFromUSDMaterial()
    ///
    /// Create an MDL material instance from a USD material.
    /// The MDL material instance definition match the USD material specified asset.
    /// See MDLModuleAttributeName and MdlElementQualifiedName.
    ///
    /// materialInstanceName: input name of the MDL material instance to create
    /// material: USD material used to create the MDL instance
    ///
    bool CreateMdlMaterialInstanceFromUSDMaterial(
        mi::neuraylib::ITransaction * transaction
        , const std::string & materialInstanceName
        , const UsdShadeMaterial & material
    );

    bool CreateMdlElementFromUsdShader(
        mi::neuraylib::ITransaction * transaction
        , std::string & elementName
        , const UsdShadeShader & shader
    );

    /// SetMdlObjectValue()
    ///
    /// Update the given MDL intance form the USD shader parameter.
    ///
    /// object: MDL material instance or function call to update
    /// input: Shader input parameter
    ///
    template <class MaterialInstanceOrFunctionCall> bool SetMdlObjectValue(
        mi::neuraylib::ITransaction * transaction
        , MaterialInstanceOrFunctionCall * object
        , const UsdShadeInput & input
    );

private:
    mi::base::Handle<mi::neuraylib::INeuray> m_neuray;
    mi::base::Handle<mi::neuraylib::IMdl_factory> m_factory;
    mi::base::Handle<mi::neuraylib::ITransaction> m_transaction;
    bool m_cleanupDB = false;
    DBHelper m_DBHelper;
    int m_returnCode = 0;

private: // Static
    /// GetAttributeStringValue()
    ///
    /// Return the attribute attributeName attached to the prim.
    /// Assume the attribute is a TfToken which is converted to string
    ///
    static std::string GetAttributeStringValue(const UsdPrim & prim, std::string attributeName);

    /// GetUsdSurfaceShadeShader()
    ///
    /// Return the shader attached to the surface output (See MdlSurfaceOutputAttributeName)
    ///
    static UsdShadeShader GetUsdSurfaceShadeShader(const UsdShadeMaterial & material);

    /// GetMdlMaterialDefinition()
    ///
    /// Look for the shader attached to the surface output and invoke GetMdlMdlElementQualifiedName(shader)
    ///
    static std::string GetMdlMdlElementQualifiedName(const UsdShadeMaterial & material);

    /// GetMdlMdlElementQualifiedName()
    ///
    /// Return the value of the material or function definition with the module prepended (See MdlElementName)
    ///
    static std::string GetMdlMdlElementQualifiedName(const UsdShadeShader & shader);

    /// GetMdlModuleName()
    ///
    /// Look for the shader attached to the surface output and invoke GetMdlModuleName(shader)
    ///
    static std::string GetMdlModuleName(const UsdShadeMaterial & material);

    /// GetMdlModuleName()
    ///
    /// Return the value of the module attribute name (See MdlModuleAttributeName)
    ///
    static std::string GetMdlModuleName(const UsdShadeShader & shader);
};

static const char * MdlSurfaceOutputAttributeName = "mdl:surface";
static const char * MdlModuleAttributeName = "info:mdl:sourceAsset";
static const char * MdlElementName = "info:mdl:sourceAsset:subIdentifier";

UsdToMdlConverter::UsdToMdlConverter(mi::neuraylib::INeuray * neuray)
{
    setup(neuray);
}

UsdToMdlConverter::~UsdToMdlConverter()
{
    finalize();
}

int UsdToMdlConverter::Convert(const UsdShadeMaterial & material, Settings & settings)
{
    m_returnCode = 0;
    m_cleanupDB = settings.m_cleanupDB;
    if (CreateMdlMaterialInstanceFromUSDMaterial(m_transaction.get(), settings.m_materialInstanceName, material))
    {
        UpdateMdlMaterialFromUSDMaterial(m_transaction.get(), settings.m_materialInstanceName, material);
    }
    return m_returnCode;
}

class BaseConverter
{
public:
    virtual bool Convert(
          const UsdShadeInput & input
        , mi::base::Handle<mi::neuraylib::IExpression> & expression
        , mi::neuraylib::ITransaction * transaction
        , mi::neuraylib::IMdl_factory * factory
        , mi::neuraylib::IValue_factory * valueFactory
        , mi::neuraylib::IExpression_factory * expressionFactory
    ) = 0;
};

template <class InputAtomicType> class SimpleValueConverter : public BaseConverter
{
public:
    virtual bool Convert(
          const UsdShadeInput & input
        , mi::base::Handle<mi::neuraylib::IExpression> & expression
        , mi::neuraylib::ITransaction * transaction
        , mi::neuraylib::IMdl_factory * factory
        , mi::neuraylib::IValue_factory * valueFactory
        , mi::neuraylib::IExpression_factory * expressionFactory
    )
    {
        InputAtomicType value;
        if (input.Get(&value))
        {
            mi::base::Handle<mi::neuraylib::IValue> mdlvalue;
            if (CreateValue(valueFactory, mdlvalue))
            {
                const mi::Sint32 rtn(set_value(mdlvalue.get(), value));
                if (rtn == 0)
                {
                    expression = expressionFactory->create_constant(mdlvalue.get());
                    return expression.is_valid_interface();
                }
            }
        }

        return false;
    }
protected:
    virtual bool CreateValue(mi::neuraylib::IValue_factory * valueFactory, mi::base::Handle<mi::neuraylib::IValue> & mdlvalue) = 0;
};

class SimpleIntConverter : public SimpleValueConverter<int>
{
protected:
    bool CreateValue(mi::neuraylib::IValue_factory * valueFactory, mi::base::Handle<mi::neuraylib::IValue> & mdlvalue) override
    {
        mdlvalue = valueFactory->create_int();
        return mdlvalue.is_valid_interface();
    }
};

class SimpleFloatConverter : public SimpleValueConverter<float>
{
protected:
    bool CreateValue(mi::neuraylib::IValue_factory * valueFactory, mi::base::Handle<mi::neuraylib::IValue> & mdlvalue) override
    {
        mdlvalue = valueFactory->create_float();
        return mdlvalue.is_valid_interface();
    }
};

class SimpleBoolConverter : public SimpleValueConverter<bool>
{
protected:
    bool CreateValue(mi::neuraylib::IValue_factory * valueFactory, mi::base::Handle<mi::neuraylib::IValue> & mdlvalue) override
    {
        mdlvalue = valueFactory->create_bool();
        return mdlvalue.is_valid_interface();
    }
};

class SimpleDoubleConverter : public SimpleValueConverter<double>
{
protected:
    bool CreateValue(mi::neuraylib::IValue_factory * valueFactory, mi::base::Handle<mi::neuraylib::IValue> & mdlvalue) override
    {
        mdlvalue = valueFactory->create_double();
        return mdlvalue.is_valid_interface();
    }
};

class SimpleStringConverter : public BaseConverter
{
protected:
    virtual bool Convert(
        const UsdShadeInput & input
        , mi::base::Handle<mi::neuraylib::IExpression> & expression
        , mi::neuraylib::ITransaction * transaction
        , mi::neuraylib::IMdl_factory * factory
        , mi::neuraylib::IValue_factory * valueFactory
        , mi::neuraylib::IExpression_factory * expressionFactory
    )
    {
        std::string value;
        if (input.Get(&value))
        {
            mi::base::Handle<mi::neuraylib::IValue> mdlvalue(valueFactory->create_string(value.c_str()));
            if (mdlvalue)
            {
                expression = expressionFactory->create_constant(mdlvalue.get());
                return expression.is_valid_interface();
            }
        }
        return false;
    }
};

class SimpleColorConverter : public BaseConverter
{
protected:
    virtual bool Convert(
          const UsdShadeInput & input
        , mi::base::Handle<mi::neuraylib::IExpression> & expression
        , mi::neuraylib::ITransaction * transaction
        , mi::neuraylib::IMdl_factory * factory
        , mi::neuraylib::IValue_factory * valueFactory
        , mi::neuraylib::IExpression_factory * expressionFactory
    )
    {
        GfVec3f value;
        if (input.Get(&value))
        {
            mi::base::Handle<mi::neuraylib::IValue_color> mdlColorValue(valueFactory->create_color(value[0], value[1], value[2]));
            expression = expressionFactory->create_constant(mdlColorValue.get());
            return true;
        }
        return false;
    }
};

template <class InputType> class ArrayConverter : public BaseConverter
{
public:
    ArrayConverter()
        : m_size(0)
    {
    }
    virtual bool Convert(
        const UsdShadeInput & input
        , mi::base::Handle<mi::neuraylib::IExpression> & expression
        , mi::neuraylib::ITransaction * transaction
        , mi::neuraylib::IMdl_factory * factory
        , mi::neuraylib::IValue_factory * valueFactory
        , mi::neuraylib::IExpression_factory * expressionFactory
    ) override
    {
        /// Is this an atomic value or an array
        {
            InputType value;
            if (input.Get(&value))
            {
                SetSize(InputType().dimension);

                return ConvertAtomic(input, expression, transaction, factory, valueFactory, expressionFactory);
            }
        }
        {
            VtArray<InputType> value;
            if (input.Get(&value))
            {
                SetSize(value.size());

                return ConvertArrayOfVectors(input, expression, transaction, factory, valueFactory, expressionFactory);
            }
        }
        return false;
    }
    mi::Size GetSize()
    {
        return m_size;
    }
protected:
    virtual bool ConvertAtomic(
          const UsdShadeInput & input
        , mi::base::Handle<mi::neuraylib::IExpression> & expression
        , mi::neuraylib::ITransaction * transaction
        , mi::neuraylib::IMdl_factory * factory
        , mi::neuraylib::IValue_factory * valueFactory
        , mi::neuraylib::IExpression_factory * expressionFactory
    )
    {
        mi::base::Handle<mi::neuraylib::IType_factory> typeFactory(factory->create_type_factory(transaction));
        //mi::base::Handle<const mi::neuraylib::IType> arrayElementType(typeFactory->create_float());
        mi::base::Handle<const mi::neuraylib::IType> arrayElementType;
        if (CreateType(typeFactory.get(), arrayElementType))
        {
            mi::base::Handle<const mi::neuraylib::IType_array> newArray(typeFactory->create_immediate_sized_array(arrayElementType.get(), GetSize()));
            mi::base::Handle<mi::neuraylib::IValue_array> mdlValueArray(valueFactory->create_array(newArray.get()));
            //mi::base::Handle<mi::neuraylib::IValue> arrayElementValue(valueFactory->create_float());
            mi::base::Handle<mi::neuraylib::IValue> arrayElementValue;
            if (CreateValue(valueFactory, arrayElementValue))
            {
                InputType value;
                if (input.Get(&value))
                {
                    for (mi::Size index = 0; index < GetSize(); index++)
                    {
                        //arrayElementValue->set_value(value[index]);
                        mi::neuraylib::set_value(arrayElementValue.get(), value[index]);
                        mdlValueArray->set_value(index, arrayElementValue.get());
                    }
                    expression = expressionFactory->create_constant(mdlValueArray.get());
                    return true;
                }
            }
        }
        return false;
    }
    virtual bool ConvertArrayOfVectors(
          const UsdShadeInput & input
        , mi::base::Handle<mi::neuraylib::IExpression> & expression
        , mi::neuraylib::ITransaction * transaction
        , mi::neuraylib::IMdl_factory * factory
        , mi::neuraylib::IValue_factory * valueFactory
        , mi::neuraylib::IExpression_factory * expressionFactory
    )
    {
        int vectorElementSize = InputType().dimension;
        VtArray<InputType> value;
        if (input.Get(&value))
        {
            mi::base::Handle<mi::neuraylib::IType_factory> typeFactory(factory->create_type_factory(transaction));
            //mi::base::Handle<const mi::neuraylib::IType_atomic> atomicType(typeFactory->create_float());
            mi::base::Handle<const mi::neuraylib::IType> type;
            if (CreateType(typeFactory.get(), type))
            {
                mi::base::Handle<const mi::neuraylib::IType_atomic> atomicType(type->get_interface<const mi::neuraylib::IType_atomic>());
                if (atomicType)
                {
                    mi::base::Handle<const mi::neuraylib::IType_vector> vectorType(typeFactory->create_vector(atomicType.get(), vectorElementSize));
                    mi::base::Handle<const mi::neuraylib::IType_array> arrayType(typeFactory->create_immediate_sized_array(vectorType.get(), GetSize()));
                    mi::base::Handle<mi::neuraylib::IValue_array> mdlValueArray(valueFactory->create_array(arrayType.get()));
                    mdlValueArray->set_size(GetSize());
                    //mi::base::Handle<mi::neuraylib::IValue_float> atomicValue(valueFactory->create_float());
                    mi::base::Handle<mi::neuraylib::IValue> atomicValue;
                    if (CreateValue(valueFactory, atomicValue))
                    {
                        for (mi::Sint32 index = 0; index < GetSize(); index++)
                        {
                            InputType usdValue(value[index]);
                            mi::base::Handle<mi::neuraylib::IValue_vector> mdlValue(mdlValueArray->get_value<mi::neuraylib::IValue_vector>(index));
                            for (mi::Sint32 index2 = 0; index2 < vectorElementSize; index2++)
                            {
                                //atomicValue->set_value(usdValue[index2]);
                                mi::neuraylib::set_value(atomicValue.get(), usdValue[index2]);
                                mdlValue->set_value(index2, atomicValue.get());
                            }
                        }
                        expression = expressionFactory->create_constant(mdlValueArray.get());
                        return true;
                    }
                }
            }
        }
        return false;
    }
    void SetSize(size_t sz)
    {
        m_size = sz;
    }
    virtual bool CreateType(mi::neuraylib::IType_factory * typeFactory, mi::base::Handle<const mi::neuraylib::IType> & type) = 0;
    virtual bool CreateValue(mi::neuraylib::IValue_factory * valueFactory, mi::base::Handle<mi::neuraylib::IValue> & mdlvalue) = 0;
private:
    mi::Size m_size;
};

template <class InputType> class FloatArrayConverter : public ArrayConverter<InputType>
{
public:
    FloatArrayConverter()
      : ArrayConverter<InputType>()
    {
    }
protected:
    bool CreateType(mi::neuraylib::IType_factory * typeFactory, mi::base::Handle<const mi::neuraylib::IType> & type) override
    {
        type = typeFactory->create_float();
        return type.is_valid_interface();
    }

    bool CreateValue(mi::neuraylib::IValue_factory * valueFactory, mi::base::Handle<mi::neuraylib::IValue> & mdlvalue) override
    {
        mdlvalue = valueFactory->create_float();
        return mdlvalue.is_valid_interface();
    }
};

template <class InputType> class IntArrayConverter : public ArrayConverter<InputType>
{
public:
    IntArrayConverter()
        : ArrayConverter<InputType>()
    {
    }
protected:
    bool CreateType(mi::neuraylib::IType_factory * typeFactory, mi::base::Handle<const mi::neuraylib::IType> & type) override
    {
        type = typeFactory->create_int();
        return type.is_valid_interface();
    }

    bool CreateValue(mi::neuraylib::IValue_factory * valueFactory, mi::base::Handle<mi::neuraylib::IValue> & mdlvalue) override
    {
        mdlvalue = valueFactory->create_int();
        return mdlvalue.is_valid_interface();
    }
};

template <class InputType> class DoubleArrayConverter : public ArrayConverter<InputType>
{
public:
    DoubleArrayConverter()
      : ArrayConverter<InputType>()
    {
    }
protected:
    bool CreateType(mi::neuraylib::IType_factory * typeFactory, mi::base::Handle<const mi::neuraylib::IType> & type) override
    {
        type = typeFactory->create_double();
        return type.is_valid_interface();
    }

    bool CreateValue(mi::neuraylib::IValue_factory * valueFactory, mi::base::Handle<mi::neuraylib::IValue> & mdlvalue) override
    {
        mdlvalue = valueFactory->create_double();
        return mdlvalue.is_valid_interface();
    }
};

template <class InputAtomicType, class MdlAtomicType> class ConvertArray : public BaseConverter
{
public:
    virtual bool CreateAtomicType(mi::neuraylib::IType_factory * typeFactory, mi::base::Handle<const mi::neuraylib::IType_atomic> & atomicType) = 0;

    virtual bool Convert(
          const UsdShadeInput & input
        , mi::base::Handle<mi::neuraylib::IExpression> & expression
        , mi::neuraylib::ITransaction * transaction
        , mi::neuraylib::IMdl_factory * factory
        , mi::neuraylib::IValue_factory * valueFactory
        , mi::neuraylib::IExpression_factory * expressionFactory
        )
    {
        VtArray<InputAtomicType> value;
        if (input.Get(&value))
        {
            mi::base::Handle<mi::neuraylib::IType_factory> typeFactory(factory->create_type_factory(transaction));
            mi::base::Handle<const mi::neuraylib::IType_atomic> atomicType;
            if (CreateAtomicType(typeFactory.get(), atomicType))
            {
                mi::base::Handle<const mi::neuraylib::IType_array> arrayType(typeFactory->create_immediate_sized_array(atomicType.get(), value.size()));
                mi::base::Handle<mi::neuraylib::IValue_array> mdlValueArray(valueFactory->create_array(arrayType.get()));
                mdlValueArray->set_size(value.size());
                for (mi::Sint32 index = 0; index < value.size(); index++)
                {
                    InputAtomicType usdValue(value[index]);
                    mi::base::Handle<MdlAtomicType> mdlValue(mdlValueArray->get_value<MdlAtomicType>(index));
                    mdlValue->set_value(usdValue);
                }
                expression = expressionFactory->create_constant(mdlValueArray.get());
                return true;
            }
        }
        return false;
    }
};

class ConvertIntArray : public ConvertArray<int, mi::neuraylib::IValue_int>
{
public:
    bool CreateAtomicType(mi::neuraylib::IType_factory * typeFactory, mi::base::Handle<const mi::neuraylib::IType_atomic> & atomicType) override
    {
        atomicType = typeFactory->create_int();
        return atomicType != NULL;
    }
};

class ConvertFloatArray : public ConvertArray<float, mi::neuraylib::IValue_float>
{
public:
    bool CreateAtomicType(mi::neuraylib::IType_factory * typeFactory, mi::base::Handle<const mi::neuraylib::IType_atomic> & atomicType) override
    {
        atomicType = typeFactory->create_float();
        return atomicType != NULL;
    }
};

class ConvertBoolArray : public ConvertArray<bool, mi::neuraylib::IValue_bool>
{
public:
    bool CreateAtomicType(mi::neuraylib::IType_factory * typeFactory, mi::base::Handle<const mi::neuraylib::IType_atomic> & atomicType) override
    {
        atomicType = typeFactory->create_bool();
        return atomicType != NULL;
    }
};

class ConvertDoubleArray : public ConvertArray<double, mi::neuraylib::IValue_double>
{
public:
    bool CreateAtomicType(mi::neuraylib::IType_factory * typeFactory, mi::base::Handle<const mi::neuraylib::IType_atomic> & atomicType) override
    {
        atomicType = typeFactory->create_double();
        return atomicType != NULL;
    }
};

class ConvertStringArray : public BaseConverter
{
public:
    bool CreateAtomicType(mi::neuraylib::IType_factory * typeFactory, mi::base::Handle<const mi::neuraylib::IType_atomic> & atomicType)
    {
        atomicType = typeFactory->create_string();
        return atomicType != NULL;
    }
    virtual bool Convert(
          const UsdShadeInput & input
        , mi::base::Handle<mi::neuraylib::IExpression> & expression
        , mi::neuraylib::ITransaction * transaction
        , mi::neuraylib::IMdl_factory * factory
        , mi::neuraylib::IValue_factory * valueFactory
        , mi::neuraylib::IExpression_factory * expressionFactory
    )
    {
        VtArray<std::string> value;
        if (input.Get(&value))
        {
            mi::base::Handle<mi::neuraylib::IType_factory> typeFactory(factory->create_type_factory(transaction));
            mi::base::Handle<const mi::neuraylib::IType_atomic> atomicType;
            if (CreateAtomicType(typeFactory.get(), atomicType))
            {
                mi::base::Handle<const mi::neuraylib::IType_array> arrayType(typeFactory->create_immediate_sized_array(atomicType.get(), value.size()));
                mi::base::Handle<mi::neuraylib::IValue_array> mdlValueArray(valueFactory->create_array(arrayType.get()));
                mdlValueArray->set_size(value.size());
                for (mi::Sint32 index = 0; index < value.size(); index++)
                {
                    std::string usdValue(value[index]);
                    mi::base::Handle<mi::neuraylib::IValue_string> mdlValue(mdlValueArray->get_value<mi::neuraylib::IValue_string>(index));
                    mdlValue->set_value(usdValue.c_str());
                }
                expression = expressionFactory->create_constant(mdlValueArray.get());
                return true;
            }
        }
        return false;
    }
};

class ConvertColorArray : public BaseConverter
{
public:
    virtual bool Convert(
          const UsdShadeInput & input
        , mi::base::Handle<mi::neuraylib::IExpression> & expression
        , mi::neuraylib::ITransaction * transaction
        , mi::neuraylib::IMdl_factory * factory
        , mi::neuraylib::IValue_factory * valueFactory
        , mi::neuraylib::IExpression_factory * expressionFactory
    )
    {
        VtArray<GfVec3f> value;
        if (input.Get(&value))
        {
            mi::base::Handle<mi::neuraylib::IType_factory> typeFactory(factory->create_type_factory(transaction));
            mi::base::Handle<const mi::neuraylib::IType_color> atomicType(typeFactory->create_color());
            if (atomicType)
            {
                mi::base::Handle<const mi::neuraylib::IType_array> arrayType(typeFactory->create_immediate_sized_array(atomicType.get(), value.size()));
                mi::base::Handle<mi::neuraylib::IValue_array> mdlValueArray(valueFactory->create_array(arrayType.get()));
                mdlValueArray->set_size(value.size());
                for (mi::Sint32 index = 0; index < value.size(); index++)
                {
                    GfVec3f usdValue(value[index]);
                    mi::base::Handle<mi::neuraylib::IValue_color> mdlValue(valueFactory->create_color(usdValue[0], usdValue[1], usdValue[2]));
                    mdlValueArray->set_value(index, mdlValue.get());
                }
                expression = expressionFactory->create_constant(mdlValueArray.get());
                return true;
            }
        }
        return false;
    }
};

template <class InputType> class ConvertMatrix : public BaseConverter
{
public:
    virtual bool Convert(
          const UsdShadeInput & input
        , mi::base::Handle<mi::neuraylib::IExpression> & expression
        , mi::neuraylib::ITransaction * transaction
        , mi::neuraylib::IMdl_factory * factory
        , mi::neuraylib::IValue_factory * valueFactory
        , mi::neuraylib::IExpression_factory * expressionFactory
    )
    {
        InputType value; // e.g. GfMatrix2d
        size_t vectorElementSize = value.numRows;
        size_t columns = value.numColumns;
        if (input.Get(&value))
        {
            mi::base::Handle<mi::neuraylib::IType_factory> typeFactory(factory->create_type_factory(transaction));
            mi::base::Handle<const mi::neuraylib::IType_atomic> atomicType(typeFactory->create_double());
            mi::base::Handle<const mi::neuraylib::IType_vector> vectorType(typeFactory->create_vector(atomicType.get(), vectorElementSize));
            mi::base::Handle<const mi::neuraylib::IType_matrix> matrixType(typeFactory->create_matrix(vectorType.get(), columns));
            mi::base::Handle<mi::neuraylib::IValue_matrix> mdlValueMatrix(valueFactory->create_matrix(matrixType.get()));
            mi::base::Handle<mi::neuraylib::IValue_vector> mdlValueVector(valueFactory->create_vector(vectorType.get()));
            mi::base::Handle<mi::neuraylib::IValue_double> atomicValue(valueFactory->create_double());
            if (atomicType)
            {
                for (mi::Sint32 index = 0; index < columns; index++)
                {
                    for (mi::Sint32 indexV = 0; indexV < vectorElementSize; indexV++)
                    {
                        // Construct vector
                        atomicValue->set_value(value[index][indexV]);
                        mdlValueVector->set_value(indexV, atomicValue.get());
                    }

                    mi::Sint32 rtn(mdlValueMatrix->set_value(index, mdlValueVector.get()));
                }
                expression = expressionFactory->create_constant(mdlValueMatrix.get());
                return true;
            }
        }
        return false;
    }
};

class ConvertTexture : public BaseConverter
{
    bool Convert(
          const UsdShadeInput & input
        , mi::base::Handle<mi::neuraylib::IExpression> & expression
        , mi::neuraylib::ITransaction * transaction
        , mi::neuraylib::IMdl_factory * factory
        , mi::neuraylib::IValue_factory * valueFactory
        , mi::neuraylib::IExpression_factory * expressionFactory
    ) override
    {
        SdfAssetPath token;
        input.Get<SdfAssetPath>(&token);
        std::string filename(token.GetAssetPath());

        const std::string textureName = MdlUtils::GetUniqueName(transaction, "texture");
        mi::base::Handle<mi::neuraylib::ITexture> texture(transaction->create<mi::neuraylib::ITexture>("Texture"));
        texture->set_image(filename.c_str());
        transaction->store(texture.get(), textureName.c_str());
        mi::base::Handle<mi::neuraylib::IType_factory> typeFactory(factory->create_type_factory(transaction));
        mi::base::Handle<const mi::neuraylib::IType_texture> tex_type(typeFactory->create_texture(mi::neuraylib::IType_texture::TS_2D));
        mi::base::Handle<mi::neuraylib::IValue_texture> mdlvalue(valueFactory->create_texture(tex_type.get(), textureName.c_str()));
        if (mdlvalue)
        {
            expression = expressionFactory->create_constant(mdlvalue.get());
            return true;
        }
        return false;
    }
};

bool UsdToMdlConverter::ConvertValue(
      mi::neuraylib::ITransaction * transaction
    , const UsdShadeInput & input
    , const mi::neuraylib::IType * inputType
    , mi::base::Handle<mi::neuraylib::IExpression> & expression
)
{
    mi::base::Handle<mi::neuraylib::IExpression_factory> expressionFactory(
        m_factory->create_expression_factory(transaction));
    assert(expressionFactory.is_valid_interface());

    mi::base::Handle<mi::neuraylib::IValue_factory> valueFactory(
        m_factory->create_value_factory(transaction));

    if (input.IsDefined())
    {
        TfToken name(input.GetBaseName());
        std::string namestr(name.GetString());
        {
            /////////////////////////////////////////////////////////////
            // Connection
            /////////////////////////////////////////////////////////////
            if (input.HasConnectedSource())
            {
                UsdShadeConnectableAPI source;
                TfToken sourceName;
                UsdShadeAttributeType sourceType;
                if (input.GetConnectedSource(&source, &sourceName, &sourceType))
                {
                    if (source.IsShader())
                    {
                        UsdPrim prim(source.GetPrim());
                        UsdShadeShader shader(prim);

                        Mdl::LogInfo("[ConvertValue] connected parm (name;source;shader): " + name.GetString() + ";" + sourceName.GetString() + ";" + shader.GetPath().GetString());

                        std::string elementName("shader");
                        if (CreateMdlElementFromUsdShader(transaction, elementName, shader))
                        {
                            std::string elementDBName(elementName);
                            expression = expressionFactory->create_call(elementDBName.c_str());
                            return true;
                        }
                    }
                    else
                    {
                        Mdl::LogError("[ConvertValue] connected parm not a shader (name;source): " + name.GetString() + "/" + sourceName.GetString());
                    }
                }
                else
                {
                    Mdl::LogError("[ConvertValue] connected parm failed to get connected source: " + name.GetString());
                }
            }
            else
            {
                /////////////////////////////////////////////////////////////
                // No Connection
                /////////////////////////////////////////////////////////////
                //SdfValueTypeName UChar, UInt, Int64, UInt64;
                //SdfValueTypeName Half;
                //SdfValueTypeName Token;
                //SdfValueTypeName Asset                        /// Support for texture assets only
                //SdfValueTypeName Half2, Half3, Half4;
                //SdfValueTypeName Point3h, Point3f, Point3d;
                //SdfValueTypeName Vector3h, Vector3f, Vector3d;
                //SdfValueTypeName Normal3h, Normal3f, Normal3d;
                //SdfValueTypeName Color3h, Color3d;
                //SdfValueTypeName Color4h, Color4f, Color4d;
                //SdfValueTypeName Quath, Quatf, Quatd;
                //SdfValueTypeName Frame4d;
                //SdfValueTypeName TexCoord2h, TexCoord2f, TexCoord2d;
                //SdfValueTypeName TexCoord3h, TexCoord3f, TexCoord3d;
                //SdfValueTypeName UCharArray, UIntArray, Int64Array, UInt64Array;
                //SdfValueTypeName HalfArray
                //SdfValueTypeName TokenArray, AssetArray;
                //SdfValueTypeName Half2Array, Half3Array, Half4Array;
                //SdfValueTypeName Point3hArray, Point3fArray, Point3dArray;
                //SdfValueTypeName Vector3hArray, Vector3fArray, Vector3dArray;
                //SdfValueTypeName Normal3hArray, Normal3fArray, Normal3dArray;
                //SdfValueTypeName Color3hArray, Color3dArray;
                //SdfValueTypeName Color4hArray, Color4fArray, Color4dArray;
                //SdfValueTypeName QuathArray, QuatfArray, QuatdArray;
                //SdfValueTypeName Matrix2dArray, Matrix3dArray, Matrix4dArray;
                //SdfValueTypeName Frame4dArray;
                //SdfValueTypeName TexCoord2hArray, TexCoord2fArray, TexCoord2dArray;
                //SdfValueTypeName TexCoord3hArray, TexCoord3fArray, TexCoord3dArray;
                {
                    BaseConverter * converter(NULL);
                    if (input.GetTypeName() == SdfValueTypeNames->Int)
                    {
                        converter = new SimpleIntConverter;
                    }
                    else if (input.GetTypeName() == SdfValueTypeNames->Float)
                    {
                        converter = new SimpleFloatConverter;
                    }
                    else if (input.GetTypeName() == SdfValueTypeNames->Bool)
                    {
                        converter = new SimpleBoolConverter;
                    }
                    else if (input.GetTypeName() == SdfValueTypeNames->Double)
                    {
                        converter = new SimpleDoubleConverter;
                    }
                    else if (input.GetTypeName() == SdfValueTypeNames->String)
                    {
                        converter = new SimpleStringConverter;
                    }
                    else if (input.GetTypeName() == SdfValueTypeNames->Color3f)
                    {
                        converter = new SimpleColorConverter;
                    }
                    else if (
                           input.GetTypeName() == SdfValueTypeNames->Float2
                        || input.GetTypeName() == SdfValueTypeNames->Float2Array
                        )
                    {
                        converter = new FloatArrayConverter<GfVec2f>;
                    }
                    else if (
                           input.GetTypeName() == SdfValueTypeNames->Float3
                        || input.GetTypeName() == SdfValueTypeNames->Float3Array
                        )
                    {
                        converter = new FloatArrayConverter<GfVec3f>;
                    }
                    else if (
                           input.GetTypeName() == SdfValueTypeNames->Float4
                        || input.GetTypeName() == SdfValueTypeNames->Float4Array
                        )
                    {
                        converter = new FloatArrayConverter<GfVec4f>;
                    }
                    else if (
                           input.GetTypeName() == SdfValueTypeNames->Int2
                        || input.GetTypeName() == SdfValueTypeNames->Int2Array
                        )
                    {
                        converter = new IntArrayConverter<GfVec2i>;
                    }
                    else if (
                           input.GetTypeName() == SdfValueTypeNames->Int3
                        || input.GetTypeName() == SdfValueTypeNames->Int3Array
                        )
                    {
                        converter = new IntArrayConverter<GfVec3i>;
                    }
                    else if (
                           input.GetTypeName() == SdfValueTypeNames->Int4
                        || input.GetTypeName() == SdfValueTypeNames->Int4Array
                        )
                    {
                        converter = new IntArrayConverter<GfVec4i>;
                    }
                    else if (
                           input.GetTypeName() == SdfValueTypeNames->Double2
                        || input.GetTypeName() == SdfValueTypeNames->Double2Array
                        )
                    {
                        converter = new DoubleArrayConverter<GfVec2d>;
                    }
                    else if (
                           input.GetTypeName() == SdfValueTypeNames->Double3
                        || input.GetTypeName() == SdfValueTypeNames->Double3Array
                        )
                    {
                        converter = new DoubleArrayConverter<GfVec3d>;
                    }
                    else if (
                           input.GetTypeName() == SdfValueTypeNames->Double4
                        || input.GetTypeName() == SdfValueTypeNames->Double4Array
                        )
                    {
                        converter = new DoubleArrayConverter<GfVec4d>;
                    }
                    else if (input.GetTypeName() == SdfValueTypeNames->IntArray)
                    {
                        converter = new ConvertIntArray;
                    }
                    else if (input.GetTypeName() == SdfValueTypeNames->FloatArray)
                    {
                        converter = new ConvertFloatArray;
                    }
                    else if (input.GetTypeName() == SdfValueTypeNames->StringArray)
                    {
                        converter = new ConvertStringArray;
                    }
                    else if (input.GetTypeName() == SdfValueTypeNames->BoolArray)
                    {
                        converter = new ConvertBoolArray;
                    }
                    else if (input.GetTypeName() == SdfValueTypeNames->Color3fArray)
                    {
                        converter = new ConvertColorArray;
                    }
                    else if (input.GetTypeName() == SdfValueTypeNames->DoubleArray)
                    {
                        converter = new ConvertDoubleArray;
                    }
                    else if (input.GetTypeName() == SdfValueTypeNames->Matrix2d)
                    {
                        converter = new ConvertMatrix<GfMatrix2d>;
                    }
                    else if (input.GetTypeName() == SdfValueTypeNames->Matrix3d)
                    {
                        converter = new ConvertMatrix<GfMatrix3d>;
                    }
                    else if (input.GetTypeName() == SdfValueTypeNames->Matrix4d)
                    {
                        converter = new ConvertMatrix<GfMatrix4d>;
                    }
                    else if (input.GetTypeName() == SdfValueTypeNames->Asset)
                    {
                        bool isTexture(inputType->get_kind() == mi::neuraylib::IType::TK_TEXTURE);
                        if (!isTexture && inputType->get_kind() == mi::neuraylib::IType::TK_ALIAS)
                        {
                            mi::base::Handle<const IType_alias> type_alias(inputType->get_interface<IType_alias>());
                            mi::base::Handle<const IType> aliased_type(type_alias->get_aliased_type());
                            isTexture = (aliased_type->get_kind() == mi::neuraylib::IType::TK_TEXTURE);
                        }
                        if (isTexture)
                        {
                            converter = new ConvertTexture;
                        }
                    }

                    if (converter)
                    {
                        bool rtn(converter->Convert(input, expression, m_transaction.get(), m_factory.get(), valueFactory.get(), expressionFactory.get()));
                        delete converter;
                        if (rtn)
                        {
                            return true;
                        }
                    }
                }

                Mdl::LogWarning("[ConvertValue] unsupported type: " + input.GetTypeName().GetAsToken().GetString());
            }
        }
    }
    return false;
}

template <class MaterialInstanceOrFunctionCall> bool UsdToMdlConverter::SetMdlObjectValue(
      mi::neuraylib::ITransaction * transaction
    , MaterialInstanceOrFunctionCall * object
    , const UsdShadeInput & input
)
{
    mi::base::Handle<mi::neuraylib::IExpression> expression;
    std::string name(input.GetBaseName().GetString());
    mi::Size index(object->get_parameter_index(name.c_str()));
    if (index >= 0)
    {
        mi::base::Handle<const mi::neuraylib::IType_list> types(object->get_parameter_types());
        if (types)
        {
            mi::base::Handle<const IType> type(types->get_type(index));
            if (type)
            {
                if (UsdToMdlConverter::ConvertValue(transaction, input, type.get(), expression) && expression)
                {
                    object->set_argument(name.c_str(), expression.get());
                    return true;
                }
                else
                {
                    Mdl::LogError("[SetMdlObjectValue] Unable to convert parameter: " + name);
                }
            }
            else
            {
                Mdl::LogError("[SetMdlObjectValue] Unknown parameter: " + name);
            }
        }
        else
        {
            Mdl::LogError("[SetMdlObjectValue] Invalid parameter types for parameter: " + name);
        }
    }
    else
    {
        Mdl::LogError("[SetMdlObjectValue] Parameter not found: " + name);
    }
    return false;
}

bool UsdToMdlConverter::CreateMdlMaterialInstanceFromUSDMaterial(
      mi::neuraylib::ITransaction * transaction
    , const std::string & materialInstanceName
    , const UsdShadeMaterial & material
)
{
    std::string moduleName(GetMdlModuleName(material));
    if (moduleName.empty())
    {
        m_returnCode = CanNotGetModuleFromMaterial;
        return false;
    }
    mi::neuraylib::ModuleLoader loader;
    loader.SetModuleName(moduleName);
    if( !loader.LoadModule(transaction, m_neuray.get()))
    {
        m_returnCode = CanNotLoadModule;
        return false;
    }

    std::string materialDefinition(MdlUtils::GetDBName(GetMdlMdlElementQualifiedName(material)));
    if (materialDefinition.empty())
    {
        m_returnCode = CanNotGetMaterialDefinitionFromMaterial;
        return false;
    }
    bool rtn(false);
    {
        mi::base::Handle<const mi::neuraylib::IMaterial_definition> materialDefinitionDB(transaction->access<mi::neuraylib::IMaterial_definition>(materialDefinition.c_str()));
        if (materialDefinitionDB)
        {
            mi::neuraylib::Definition_wrapper defWrapper(transaction, materialDefinition.c_str(), m_factory.get());

            const mi::neuraylib::IExpression_list* arguments(NULL);
            mi::Sint32 errors;
            mi::base::Handle<mi::neuraylib::IScene_element> sceneElement(defWrapper.create_instance(arguments, &errors));
            mi::base::Handle<mi::neuraylib::IMaterial_instance> materialInstance(sceneElement->get_interface<mi::neuraylib::IMaterial_instance>());
            if (materialInstance)
            {
                if (m_DBHelper.store(transaction, materialInstance.get(), materialInstanceName.c_str()) != 0)
                {
                    m_returnCode = CanNotCreateMaterialInstance;
                }
                else
                { 
                    m_returnCode = Success;
                }
            }
            else
            {
                m_returnCode = CanNotCreateMaterialInstance;
            }
        }
        else
        {
            m_returnCode = CanNotGetMaterialDefinitionFromDB;
        }
    }
    return m_returnCode == Success;
}

bool UsdToMdlConverter::CreateMdlElementFromUsdShader(
      mi::neuraylib::ITransaction * transaction
    , std::string & elementName
    , const UsdShadeShader & shader
)
{
    std::string fct(UsdToMdlConverter::GetAttributeStringValue(shader.GetPrim(), "info:id"));
    if (!fct.empty())
    {
        std::string functionDefinition(MdlUtils::GetDBName(fct));
        if (functionDefinition.empty())
        {
            return false;
        }
        mi::base::Handle<const mi::neuraylib::IFunction_definition> functionDefinitionDB(transaction->access<mi::neuraylib::IFunction_definition>(functionDefinition.c_str()));
        if (functionDefinitionDB)
        {
            mi::neuraylib::Definition_wrapper defWrapper(transaction, functionDefinition.c_str(), m_factory.get());
            const mi::neuraylib::IExpression_list* arguments(NULL);
            mi::Sint32 errors;
            mi::base::Handle<mi::neuraylib::IScene_element> sceneElement(defWrapper.create_instance(arguments, &errors));
            mi::base::Handle<mi::neuraylib::IFunction_call> fctCall(sceneElement->get_interface<mi::neuraylib::IFunction_call>());
            if (fctCall)
            {
                int converted(0);
                int failed(0);
                for (auto & input : shader.GetInputs())
                {
                    if (SetMdlObjectValue(transaction, fctCall.get(), input))
                    {
                        converted++;
                    }
                    else
                    {
                        failed++;
                    }
                }
                std::stringstream str;
                str << converted << " converted parms (" << failed << " failed)";
                Mdl::LogInfo("[CreateMdlElementFromUsdShader] converted material (" + shader.GetPrim().GetName().GetString() + "): " + str.str());

                elementName = MdlUtils::GetUniqueName(transaction, elementName);

                m_DBHelper.store(transaction, fctCall.get(), elementName.c_str());

                fctCall.reset();
            }
            else
            {
                Mdl::LogError("[CreateMdlElementFromUsdShader] Can not create function for shader: " + shader.GetPrim().GetName().GetString());
            }
        }
        return true;
    }
    return false;
}

bool UsdToMdlConverter::UpdateMdlMaterialFromUSDMaterial(
      mi::neuraylib::ITransaction * transaction
    , const std::string & storedInstanceName
    , const UsdShadeMaterial & material
)
{
    mi::base::Handle<mi::neuraylib::IMaterial_instance> instance(transaction->edit<mi::neuraylib::IMaterial_instance>(storedInstanceName.c_str()));
    assert(instance);
    if (instance)
    {
        int converted(0);
        int failed(0);
        for (auto & input : material.GetInputs())
        {
            if (SetMdlObjectValue(transaction, instance.get(), input))
            {
                converted++;
            }
            else
            {
                failed++;
            }
        }
        std::stringstream str;
        str << converted << " converted parms (" << failed << " failed)";
        Mdl::LogInfo("[UpdateMdlMaterialFromUSDMaterial] converted material (" + material.GetPrim().GetName().GetString() + "): " + str.str());
    }
    else
    {
        m_returnCode = CanNotGetMaterialInstance;
    }

    return m_returnCode == Success;
}

bool UsdToMdlConverter::setup(mi::neuraylib::INeuray * neuray)
{
    if(!neuray)
    {
        return false;
    }
    m_neuray = mi::base::make_handle_dup< mi::neuraylib::INeuray>(neuray);
    m_factory = m_neuray->get_api_component<mi::neuraylib::IMdl_factory>();

    // Access the database and create a transaction.
    mi::base::Handle<mi::neuraylib::IDatabase> database(
        neuray->get_api_component<mi::neuraylib::IDatabase>());
    mi::base::Handle<mi::neuraylib::IScope> scope(database->get_global_scope());
    m_transaction = scope->create_transaction();
    return true;
}

bool UsdToMdlConverter::finalize()
{
    if (m_cleanupDB)
    {
        m_DBHelper.cleanup(m_transaction.get());
    }
    m_transaction->commit();
    return true;
}

UsdShadeShader UsdToMdlConverter::GetUsdSurfaceShadeShader(const UsdShadeMaterial & material)
{
    UsdShadeOutput surface(material.GetOutput(TfToken(MdlSurfaceOutputAttributeName)));
    if (surface.IsDefined())
    {
        UsdPrim shader(surface.GetPrim());
        if (shader.IsDefined())
        {
            UsdShadeConnectableAPI source;
            TfToken sourceName;
            UsdShadeAttributeType sourceType;
            if (UsdShadeConnectableAPI::GetConnectedSource(
                surface, &source, &sourceName, &sourceType))
            {
                UsdPrim prim(source.GetPrim());
                bool isShader(prim.IsA<UsdShadeShader>());
                if (isShader)
                {
                    return UsdShadeShader(prim);
                }
            }
        }
    }
    return UsdShadeShader();
}

std::string UsdToMdlConverter::GetAttributeStringValue(const UsdPrim & prim, std::string attributeName)
{
    UsdAttribute attribute(prim.GetAttribute(TfToken(attributeName)));
    if (attribute.IsDefined())
    {
        VtValue val;
        attribute.Get(&val);
        TfToken token(val.Get<TfToken>());
        std::string tokenString(token.GetString());
        return tokenString;
    }
    return "";
}

std::string UsdToMdlConverter::GetMdlMdlElementQualifiedName(const UsdShadeMaterial & material)
{
    UsdShadeShader shader(GetUsdSurfaceShadeShader(material));
    return GetMdlMdlElementQualifiedName(shader);
}

std::string UsdToMdlConverter::GetMdlMdlElementQualifiedName(const UsdShadeShader & shader)
{
    UsdAttribute subIdentifier(shader.GetPrim().GetAttribute(TfToken(MdlElementName)));
    if (subIdentifier.IsDefined())
    {
        VtValue val;
        subIdentifier.Get(&val);
        TfToken token(val.Get<TfToken>());
        std::string matdef(token.GetString());
        std::string moduleName(GetMdlModuleName(shader));
        matdef = moduleName + "::" + matdef;
        Mdl::LogInfo("[GetMdlMdlElementQualifiedName] material definition: " + matdef);
        return matdef;
    }
    return "";
}

std::string UsdToMdlConverter::GetMdlModuleName(const UsdShadeMaterial & material)
{
    UsdShadeShader shader(GetUsdSurfaceShadeShader(material));
    return GetMdlModuleName(shader);
}

std::string UsdToMdlConverter::GetMdlModuleName(const UsdShadeShader & shader)
{
    UsdAttribute module(shader.GetPrim().GetAttribute(TfToken(MdlModuleAttributeName)));
    if (module.IsDefined())
    {
        VtValue val;
        module.Get(&val);
        SdfAssetPath token(val.Get<SdfAssetPath>());
        std::string moduleFileName(token.GetAssetPath());
        Mdl::LogInfo("[GetMdlModule] shader module file: " + moduleFileName);
        std::string moduleName(MdlUtils::GetModuleNameFromModuleFilename(moduleFileName));
        Mdl::LogInfo("[GetMdlModule] shader module: " + moduleName);
        return moduleName;
    }
    return "";
}

/// class UsdToMdlConverter
/////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/// class CreateModule

class CreateModule
{
public:
    /// ctor
    CreateModule(mi::neuraylib::INeuray * neuray)
    {
        setup(neuray);
    }

    ~CreateModule()
    {
        finalize();
    }

    bool CreateAndSaveModule(
          const std::string & materialName
        , const std::string & storedInstanceName
        , const std::string & inputModuleName
        , const std::string & filename
    );

private:
    /// CreateModuleFromMaterialInstance()
    ///
    /// Create a module containing a single material instance
    ///
    /// storedInstanceName: Name of the instance stored in DB (e.g. "material_001")
    /// inputModuleName: Name of the module (e.g. "::materials_module_01")
    ///
    bool CreateModuleFromMaterialInstance(
        mi::neuraylib::ITransaction * transaction
        , const std::string & materialName
        , const std::string & storedInstanceName
        , const std::string & inputModuleName
    ) const;

    /// SaveModuleToFile()
    ///
    /// Save a module to a file
    ///
    /// inputModuleName: Name of the module (e.g. "::materials_module_01")
    /// filename: Filename to save the module (e.g. "materials_module_01.mdl")
    ///
    bool SaveModuleToFile(
        mi::neuraylib::ITransaction * transaction
        , const std::string & inputModuleName
        , const std::string & filename
    ) const;

private:
    bool setup(mi::neuraylib::INeuray * neuray);
    bool finalize();
    DBHelper m_DBHelper;

private:
    mi::base::Handle<mi::neuraylib::INeuray> m_neuray;
    mi::base::Handle<mi::neuraylib::IMdl_factory> m_factory;
    mi::base::Handle<mi::neuraylib::ITransaction> m_transaction;
};

bool CreateModule::CreateModuleFromMaterialInstance(
    mi::neuraylib::ITransaction * transaction
    , const std::string & materialName
    , const std::string & storedInstanceName
    , const std::string & inputModuleName
) const
{
    mi::base::Handle<mi::neuraylib::IExpression_factory> expressionFactory(
        m_factory->create_expression_factory(transaction));
    assert(expressionFactory.is_valid_interface());

    mi::base::Handle<mi::IArray> array_materials(transaction->create<mi::IArray>("Material_data[1]"));
    assert(array_materials);
    {/// Variant_data[0]
        mi::base::Handle<mi::IStructure> material_data(array_materials->get_element<mi::IStructure>(mi::Size{ 0 }));
        assert(material_data);
        {/// material name
            mi::base::Handle<mi::IString> name(material_data->get_value<mi::IString>("material_name"));
            assert(name);
            name->set_c_str(materialName.c_str());
        }
        {/// prototype name
            mi::base::Handle<mi::IString> prototype_name(material_data->get_value<mi::IString>("prototype_name"));
            assert(prototype_name);
            prototype_name->set_c_str(storedInstanceName.c_str());
        }
        {/// parameters
            mi::base::Handle<mi::IArray> array_parameters(transaction->create<mi::IArray>("Parameter_data[]"));
            assert(array_parameters);
            auto result = material_data->set_value("parameters", array_parameters.get());
            assert(result == 0);
        }
        {/// annotations
            mi::base::Handle<mi::neuraylib::IAnnotation_block> annotations(expressionFactory->create_annotation_block());
            assert(annotations);
            auto result = material_data->set_value("annotations", annotations.get());
            assert(result == 0);
        }
    }

    auto result = m_factory->create_materials(transaction, inputModuleName.c_str(), array_materials.get());
    assert(result == 0);

    mi::base::Handle<const mi::neuraylib::IModule> module(
        transaction->access<mi::neuraylib::IModule>(MdlUtils::GetDBName(inputModuleName).c_str()));
    assert(module.is_valid_interface());

    return true;
}

bool CreateModule::SaveModuleToFile(
    mi::neuraylib::ITransaction * transaction
    , const std::string & inputModuleName
    , const std::string & filename
) const
{
    mi::base::Handle<mi::neuraylib::IMdl_compiler> compiler(
        m_neuray->get_api_component<mi::neuraylib::IMdl_compiler>()
    );
    auto result = compiler->export_module(transaction, MdlUtils::GetDBName(inputModuleName).c_str(), filename.c_str());

    return (0 == result);
}

bool CreateModule::setup(mi::neuraylib::INeuray * neuray)
{
    if (!neuray)
    {
        return false;
    }
    m_neuray = mi::base::make_handle_dup< mi::neuraylib::INeuray>(neuray);
    m_factory = m_neuray->get_api_component<mi::neuraylib::IMdl_factory>();

    // Access the database and create a transaction.
    mi::base::Handle<mi::neuraylib::IDatabase> database(
        neuray->get_api_component<mi::neuraylib::IDatabase>());
    mi::base::Handle<mi::neuraylib::IScope> scope(database->get_global_scope());
    m_transaction = scope->create_transaction();
    return true;
}

bool CreateModule::finalize()
{
    m_DBHelper.cleanup(m_transaction.get());
    m_transaction->commit();
    return true;
}

bool CreateModule::CreateAndSaveModule(
      const std::string & materialName
    , const std::string & storedInstanceName
    , const std::string & inputModuleName
    , const std::string & filename
)
{
    if (!CreateModuleFromMaterialInstance(m_transaction.get(), materialName, storedInstanceName, inputModuleName))
    {
        return false;
    }

    if (!SaveModuleToFile(m_transaction.get(), inputModuleName, filename))
    {
        return false;
    }

    return true;
}

/// class CreateModule
////////////////////////////////////////////////////

std::map<RETURN_CODE, std::string> g_returnMessages =
{
      { Success, "Success"}
    , { UnknownError, "Unknown error" }
    , { CanNotGetModuleFromMaterial, "Can not get module from material" }
    , { CanNotLoadModule, "Can not load module" }
    , { CanNotGetMaterialDefinitionFromMaterial, "Can not get material definition from material" }
    , { CanNotGetMaterialDefinitionFromDB, "Can not get material definition from database" }
    , { CanNotCreateMaterialInstance, "Can not create material instance" }
    , { CanNotGetMaterialInstance, "Can not get material instance" }
};

USDMDL_API int ConvertUsdMaterialToMdl(const UsdShadeMaterial & usdMaterial, mi::neuraylib::INeuray * neuray, const char * mdlMaterialInstanceName)
{
    if (!mdlMaterialInstanceName)
    {
        return CanNotCreateMaterialInstance;
    }
    struct UsdToMdlConverter::Settings settings;
    settings.m_cleanupDB = false;
    settings.m_materialInstanceName = mdlMaterialInstanceName;

    int rtnCode(UnknownError);
    {
        UsdToMdlConverter converter(neuray);
        rtnCode = converter.Convert(usdMaterial, settings);
    }
    if (MdlUtils::GetUsdToMdlSaveToModule())
    {
        if (rtnCode == Success)
        {
            std::string materialName(usdMaterial.GetPrim().GetName().GetString());
            std::string moduleFilename("test_module.mdl");
            /// Arbitrary name for the new module
            std::string savedModuleName("::test_module");

            CreateModule createModule(neuray);
            createModule.CreateAndSaveModule(materialName, settings.m_materialInstanceName, savedModuleName, moduleFilename);
        }
    }
    if (rtnCode != Success)
    {
        Mdl::LogError("[UsdToMdl::Convert] Error: " + g_returnMessages[RETURN_CODE(rtnCode)]);
    }
    return rtnCode;
}

PXR_NAMESPACE_CLOSE_SCOPE
