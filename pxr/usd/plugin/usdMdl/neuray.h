/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#pragma once
#include <mi/mdl_sdk.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <mutex>

#ifdef MI_PLATFORM_WINDOWS
#include <mi/base/miwindows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

#define check_success(expr) (expr) 

namespace UsdMdl
{
    static bool SetNeuray(mi::neuraylib::INeuray * neuray);

} // namespace UsdMdl

namespace mi {

    namespace neuraylib {

        class INeuray;

        /// Loads the shared library and calls the main factory function at construction time,
        /// and releases the SDK at the end of the scope in the destructor.
        ///
        class Neuray_factory
        {
        public:
            /// Enum type to encode possible error results of the INeuray interface creation.
            enum Result {
                /// The INeuray interface was successfully created.
                RESULT_SUCCESS = 0,
                /// The shared library failed to load.
                RESULT_LOAD_FAILURE,
                /// The shared library does not contain the expected \c mi_factory symbol.
                RESULT_SYMBOL_LOOKUP_FAILURE,
                /// The requested INeuray interface has a different IID than the ones 
                /// that can be served by the \c mi_factory function.
                RESULT_VERSION_MISMATCH,
                /// The requested INeuray interface cannot be served by the \c mi_factory 
                /// function and neither can the IVersion interface for better diagnostics.
                RESULT_INCOMPATIBLE_LIBRARY,
                //  Undocumented, for alignment only
                RESULT_FORCE_32_BIT = 0xffffffffU
            };

            /// The constructor loads the shared library, locates and calls the 
            /// #mi_factory() function. It store an instance of the main 
            /// #mi::neuraylib::INeuray interface for later access. 
            ///
            /// \param filename    The file name of the DSO. If \c NULL, the built-in
            ///                    default name of the SDK library is used.
            /// \param logger      Interface to report any errors during construction as well
            ///                    as during destruction. The logger interface needs to have 
            ///                    a suitable lifetime. If \c NULL, no error diagnostic will 
            ///                    be reported. The result code can be used for a diagnostic 
            ///                    after the construction.
            Neuray_factory(mi::base::ILogger* logger = 0,
                const char*        filename = 0);

            bool SetNeuray(mi::neuraylib::INeuray* neuray);

            /// Returns the result code of loading the shared library. If the return value
            /// is one of #RESULT_LOAD_FAILURE or #RESULT_SYMBOL_LOOKUP_FAILURE on a Windows
            /// operating system, a call to \c GetLastError can provide more detail.
            Result get_result_code() const { return m_result_code; }

            /// Returns the pointer to an instance of the main #mi::neuraylib::INeuray 
            /// interface if loading the shared library was successful, or \c NULL otherwise.
            /// Does not retain the interface.
            mi::neuraylib::INeuray* get() const {
                return m_neuray.get();
            }

            /// Releases the #mi::neuraylib::INeuray interface and unloads the shared library.
            ~Neuray_factory();

        private:
            /// Releases the #mi::neuraylib::INeuray interface and unloads the shared library.
            void UnloadNeuray();

        private:
            Result                                   m_result_code;
            const char*                              m_filename;
            void*                                    m_dso_handle;
            // MDL SDK
            mi::base::Handle<mi::neuraylib::INeuray> m_neuray;
        };

        inline Neuray_factory::~Neuray_factory()
        {
            UnloadNeuray();
        }
        
        inline void Neuray_factory::UnloadNeuray()
        {
            // destruct neuray before unloading the shared library
            m_neuray = 0;

            if (m_dso_handle)
            {
#ifdef MI_PLATFORM_WINDOWS
                int result = FreeLibrary((HMODULE)m_dso_handle);
#else
                int result = dlclose(m_dso_handle);
#endif
            }
        }

        /// Wrapper for MDL SDK configuration and logging
        class Mdl
        {
        public: 
            static Mdl & Get()
            {
                static Mdl singleton;
                std::call_once(g_onceFlagLoad, [] 
                {
                    singleton.Initialize();
                });
                return singleton;
            }
            
            /// Utility routines for logging messages
            /// These levels are controlled by the verbosity
            static void LogFatal(const std::string & msg);
            static void LogError(const std::string & msg);
            static void LogWarning(const std::string & msg);
            static void LogInfo(const std::string & msg);
            static void LogVerbose(const std::string & msg);
            static void LogDebug(const std::string & msg);
            static void SetVerbosity(int level)
            {
                g_verbosity = level;
            }
        public:
            mi::neuraylib::INeuray* GetNeuray() const
            {
                return m_factory->get();
            }

            /// Use the given interface to MDL SDK
            bool SetNeuray(mi::neuraylib::INeuray* neuray)
            {
                return m_factory->SetNeuray(neuray);
            }

        private:
            void Configuration(mi::neuraylib::INeuray* neuray, mi::base::ILogger* logger);
            void Initialize();
            void LogInternal(const std::string & msg, const mi::base::Message_severity & level);

        private:
            mi::neuraylib::Neuray_factory * m_factory = NULL;
            mi::base::Handle<mi::base::ILogger> m_logger;
        private:
            static int g_verbosity;
            static std::once_flag g_onceFlagLoad;
        };

        class NamedElement
        {
        public:
            NamedElement(const std::string & name)
                : m_name(name)
            {}
            std::string GetName() const
            {
                return m_name;
            }
        protected:
            std::string m_name;
        };

        class Module : public NamedElement
        {
        public:
            Module(const std::string & moduleName)
                : NamedElement(moduleName)
            {}

            std::string GetModuleName() const
            {
                return GetName();
            }
            std::string GetModuleDBName() const;
        };

        class ModuleLoader
        {
        public:
            void SetModuleName(const std::string & moduleName)
            {
                m_moduleName = moduleName;
            }

            // Load the module
            bool LoadModule(mi::neuraylib::ITransaction * transactionInput = NULL);
            bool LoadModule(mi::neuraylib::ITransaction * transactionInput, mi::neuraylib::INeuray * neurayIn);

            void SetEnumFunctions(bool enumFunctions)
            {
                m_enumFunctions = enumFunctions;
            }

            void SetEnumMaterials(bool enumMaterials)
            {
                m_enumMaterials = enumMaterials;
            }

            std::string GetModuleName() const
            {
                return m_moduleName;
            }

            std::string GetModuleFileName() const
            {
                return m_moduleFileName;
            }
        protected:
            virtual void FunctionCallback(const mi::neuraylib::IFunction_definition * function_definition) {}
            virtual void MaterialCallback(const mi::neuraylib::IMaterial_definition * material_definition) {}
        private:
            std::string m_moduleName;
            std::string m_moduleFileName;
            bool m_enumFunctions = false;
            bool m_enumMaterials = false;
        };

        // Keep it simple for the time being
        class Shader : public NamedElement
        {
        public:
            Shader(const std::string & name)
                : NamedElement(name)
            {}
        };

        class Material : public NamedElement
        {
        public:
            Material(const std::string & name)
                : NamedElement(name)
            {}

            std::string GetShortName() const;
            std::string GetModuleName() const;
        };

        // Function has the same interface as Material currently
        class Function : public Material 
        {
        public:
            Function(const std::string & name)
                : Material(name)
            {}
        };

        /// Wrapper/Helper base class for the MDL discovery API
        /// Traverse all modules and packages in MDL Search Paths
        /// Implement callbacks:
        ///     found_module()
        ///     found_package()
        /// in derived classes
        class DiscoveryHelper
        {
        public:
            /// Start traversal
            bool Discover();
        protected:
            bool TraverseInternal(const mi::neuraylib::IMdl_package_info * package);
        protected:
            virtual bool Initialize();
            virtual void FoundModule(const mi::neuraylib::IMdl_module_info * elem);
            virtual void FoundPackage(const mi::neuraylib::IMdl_package_info * elem);
        private:
            mi::base::Handle<const mi::neuraylib::IMdl_package_info> m_root;
        };

        /// Custom logger to re-direct log output.
        ///
        class Logger : public mi::base::Interface_implement<mi::base::ILogger>
        {
        public:
            /// Logger where only messages of level lower than the \p level parameter
            /// are written to stderr, i.e., \p level = 0 will disable all logging, and
            /// \p level = 1 logs only fatal messages, \p level = 2 logs errors, 
            /// \p level = 3 logs warnings, \p level = 4 logs info, \p level = 5 logs debug.
            Logger(int level);
            ~Logger();

            /// Callback function logging a message.
            void message(
                mi::base::Message_severity level,
                const char* module_category,
                const mi::base::Message_details&,
                const char* message);

            void SetVerbosity(int level)
            {
                m_level = level;
            }
        private:
            int  m_level;           ///< Logging level up to which messages are reported
                                    /// Returns a string label corresponding to the log level severity.
            std::ofstream m_file;
            bool m_to_file = true;
        };

    } // namespace neuraylib

} // namespace mi
