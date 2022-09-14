/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#pragma once

#include "pxr/base/arch/export.h"

#if defined(PXR_STATIC)
#   define USDMDL_API
#   define USDMDL_API_TEMPLATE_CLASS(...)
#   define USDMDL_API_TEMPLATE_STRUCT(...)
#   define USDMDL_LOCAL
#else
#   if defined(USDMDL_EXPORTS)
#       define USDMDL_API ARCH_EXPORT
#       define USDMDL_API_TEMPLATE_CLASS(...) ARCH_EXPORT_TEMPLATE(class, __VA_ARGS__)
#       define USDMDL_API_TEMPLATE_STRUCT(...) ARCH_EXPORT_TEMPLATE(struct, __VA_ARGS__)
#   else
#       define USDMDL_API ARCH_IMPORT
#       define USDMDL_API_TEMPLATE_CLASS(...) ARCH_IMPORT_TEMPLATE(class, __VA_ARGS__)
#       define USDMDL_API_TEMPLATE_STRUCT(...) ARCH_IMPORT_TEMPLATE(struct, __VA_ARGS__)
#   endif
#   define USDMDL_LOCAL ARCH_HIDDEN
#endif
