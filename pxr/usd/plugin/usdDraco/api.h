//
// Copyright 2017 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_USD_PLUGIN_USD_DRACO_API_H
#define PXR_USD_PLUGIN_USD_DRACO_API_H

#include "pxr/base/arch/export.h"

#if defined(PXR_STATIC)
#   define USDDRACO_API
#   define USDDRACO_API_TEMPLATE_CLASS(...)
#   define USDDRACO_API_TEMPLATE_STRUCT(...)
#   define USDDRACO_LOCAL
#else
#   if defined(USDDRACO_EXPORTS)
#       define USDDRACO_API ARCH_EXPORT
#       define USDDRACO_API_TEMPLATE_CLASS(...) ARCH_EXPORT_TEMPLATE(class, __VA_ARGS__)
#       define USDDRACO_API_TEMPLATE_STRUCT(...) ARCH_EXPORT_TEMPLATE(struct, __VA_ARGS__)
#   else
#       define USDDRACO_API ARCH_IMPORT
#       define USDDRACO_API_TEMPLATE_CLASS(...) ARCH_IMPORT_TEMPLATE(class, __VA_ARGS__)
#       define USDDRACO_API_TEMPLATE_STRUCT(...) ARCH_IMPORT_TEMPLATE(struct, __VA_ARGS__)
#   endif
#   define USDDRACO_LOCAL ARCH_HIDDEN
#endif

#endif
