/**
    The hydra core API
*/
#pragma once

#include <cstddef>
#include <cstdint>

#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
#define HC_GLOBAL __attribute__((visibility("default")))
#elif defined(_WIN32)
#ifdef HC_WINDOWS_IMPORT
#define HC_GLOBAL __declspec(dllimport)
#else
#define HC_GLOBAL __declspec(dllexport)
#endif
#else
#define HC_GLOBAL
#pragma message("WARNING: Unknown platform when building hydra core")
#endif
#define HC_API extern "C" HC_GLOBAL

namespace hydra
{

    /// Some things we want from type_traits for compile-time type checking, but we don't want to include the whole thing
    namespace type_traits
    {
        struct true_type {
            static constexpr bool value = true;
            operator bool() const noexcept { return value; }
        };
        
        struct false_type {
            static constexpr bool value = false;
            operator bool() const noexcept { return value; }
        };

        template<typename B>
        ::hydra::type_traits::true_type test_ptr_conv(const volatile B*);
        template<typename>
        ::hydra::type_traits::false_type test_ptr_conv(const volatile void*);
    
        template<typename B, typename D>
        auto test_is_base_of(int) -> decltype(test_ptr_conv<B>(static_cast<D*>(nullptr)));
        template<typename, typename>
        auto test_is_base_of(...) -> ::hydra::type_traits::true_type;

        template<typename Base, typename Derived>
        struct is_base_of
        {
            static constexpr bool value = decltype(::hydra::type_traits::test_is_base_of<Base, Derived>(0))::value;
        };

        template<class T> struct remove_pointer { typedef T type; };
        template<class T> struct remove_pointer<T*> { typedef T type; };
    }

    typedef struct
    {
        uint32_t width;
        uint32_t height;
    } Size;

    typedef struct
    {
        uint8_t* data;
        size_t size;
    } SaveState;

    enum struct LogTarget
    {
        Warning,
        Error,
        Debug,
        Info,
        VerboseI,
        VerboseII,
        VerboseIII,
        VerboseIV,
    };

    enum struct SampleType
    {
        Int16,
        Float,
    };

    enum struct ChannelType
    {
        Mono,
        Stereo,
    };

    enum struct ButtonType
    {
        Keypad1Up,
        Keypad1Down,
        Keypad1Left,
        Keypad1Right,
        Keypad2Up,
        Keypad2Down,
        Keypad2Left,
        Keypad2Right,
        A,
        B,
        X,
        Y,
        Z,
        L1,
        R1,
        L2,
        R2,
        L3,
        R3,
        Start,
        Select,
        Touch,
        Analog1Up,
        Analog1Down,
        Analog1Left,
        Analog1Right,
        Analog2Up,
        Analog2Down,
        Analog2Left,
        Analog2Right,
    };

    enum class InfoType
    {
        CoreName,
        SystemName,
        Description,
        Author,
        Version,
        License,
        Website,
        Extensions,
        Firmware,
    };

    #define X_HYDRA_INTERFACES \
        X_HYDRA_INTERFACE(IBase) \
        X_HYDRA_INTERFACE(IFrontendDriven) \
        X_HYDRA_INTERFACE(ISelfDriven) \
        X_HYDRA_INTERFACE(ISoftwareRendered) \
        X_HYDRA_INTERFACE(IOpenGlRendered) \
        X_HYDRA_INTERFACE(IAudio) \
        X_HYDRA_INTERFACE(IInput) \
        X_HYDRA_INTERFACE(ISaveState) \
        X_HYDRA_INTERFACE(IMultiplayer) \
        X_HYDRA_INTERFACE(ILog)
    
    #define X_HYDRA_INTERFACE(name) class name;
        X_HYDRA_INTERFACES
    #undef X_HYDRA_INTERFACE

    enum class InterfaceType
    {
    #define X_HYDRA_INTERFACE(name) name,
        X_HYDRA_INTERFACES
    #undef X_HYDRA_INTERFACE
        Size
    };

    // The base emulator interface, every emulator core inherits this
    struct HC_GLOBAL IBase
    {
        virtual ~IBase();
        virtual bool loadFile(const char* type, const char* path) = 0;
        virtual void reset() = 0;
        virtual Size getNativeSize() = 0;
        virtual void setOutputSize(Size size) = 0;
        virtual bool hasInterface(InterfaceType interface) = 0;
    #define X_HYDRA_INTERFACE(name) virtual name* get##name() = 0;
        X_HYDRA_INTERFACES
    #undef X_HYDRA_INTERFACE

    private:
        /// This function is defined by HYDRA_CORE macro, so when it's not defined the compiler will throw an error
        virtual void youForgotToAddHydraCoreMacroToYourClass() = 0;
    };

    /// The frontend driven emulator interface, the frontend is responsible for calling runFrame() every frame
    struct HC_GLOBAL IFrontendDriven
    {
        virtual ~IFrontendDriven();
        virtual void runFrame() = 0;
    };

    struct HC_GLOBAL ISelfDriven
    {
        virtual ~ISelfDriven();
        virtual void runLoop() = 0;
        virtual void setUiRefreshCallback(void(*callback)()) = 0;
    };

    // The software rendered emulator interface, software rendered emulators inherit this
    struct HC_GLOBAL ISoftwareRendered
    {
        virtual ~ISoftwareRendered();
        virtual void setVideoCallback(void(*callback)(void* data, Size size)) = 0;
    };

    // The OpenGL rendered emulator interface, OpenGL rendered emulators inherit this
    struct HC_GLOBAL IOpenGlRendered
    {
        virtual ~IOpenGlRendered();
        virtual void setFbo(unsigned handle) = 0;
        virtual void setContext(void* context) = 0;
        virtual void setGetProcAddress(void* function) = 0;
    };

    // The audio interface, emulators that support audio inherit this
    struct HC_GLOBAL IAudio
    {
        virtual ~IAudio();
        virtual SampleType getSampleType() { return SampleType::Int16; }
        virtual ChannelType getChannelType() { return ChannelType::Stereo; }
        virtual void setSampleRate(uint32_t sampleRate) = 0;
        virtual void setAudioCallback(void(*callback)(void* data, size_t size)) = 0;
    };

    // The input interface, emulators that support input inherit this
    struct HC_GLOBAL IInput
    {
        virtual ~IInput();
        virtual void setPollInputCallback(void(*callback)()) = 0;
        virtual void setCheckButtonCallback(int32_t(*callback)(uint32_t player, ButtonType button)) = 0;
    };

    // Save state interface, emulators that support save states inherit this
    struct HC_GLOBAL ISaveState
    {
        virtual ~ISaveState();
        virtual SaveState saveState() = 0;
        virtual bool loadState(SaveState state) = 0;
    };

    // Multiple player interface, emulators that support multiple players inherit this
    struct HC_GLOBAL IMultiplayer
    {
        virtual ~IMultiplayer();
        virtual void activatePlayer(uint32_t player) = 0;
        virtual void deactivatePlayer(uint32_t player) = 0;
        virtual uint32_t getMinimumPlayerCount() = 0;
        virtual uint32_t getMaximumPlayerCount() = 0;
    };

    // Log interface, emulators that support logging through the frontend inherit this
    struct HC_GLOBAL ILog
    {
        virtual ~ILog();
        virtual void setLogCallback(LogTarget target, void(*callback)(const char* message)) = 0;
    };
    
    /// Create an emulator and return a base interface pointer
    HC_API IBase* createEmulator();
    /// Destroy an emulator using a base interface pointer
    HC_API void destroyEmulator(IBase* emulator);
    /// Get info about the emulator, used if the info is not already cached
    HC_API const char* getInfo(InfoType type);

    #define HYDRA_CLASS \
        public: \
            bool hasInterface(::hydra::InterfaceType interface) override { \
                switch (interface) { \
                    case ::hydra::InterfaceType::IBase: { return ::hydra::type_traits::is_base_of<hydra::IBase, ::hydra::type_traits::remove_pointer<decltype(this)>::type>::value; } case ::hydra::InterfaceType::IFrontendDriven: { return ::hydra::type_traits::is_base_of<hydra::IFrontendDriven, ::hydra::type_traits::remove_pointer<decltype(this)>::type>::value; } case ::hydra::InterfaceType::ISelfDriven: { return ::hydra::type_traits::is_base_of<hydra::ISelfDriven, ::hydra::type_traits::remove_pointer<decltype(this)>::type>::value; } case ::hydra::InterfaceType::ISoftwareRendered: { return ::hydra::type_traits::is_base_of<hydra::ISoftwareRendered, ::hydra::type_traits::remove_pointer<decltype(this)>::type>::value; } case ::hydra::InterfaceType::IOpenGlRendered: { return ::hydra::type_traits::is_base_of<hydra::IOpenGlRendered, ::hydra::type_traits::remove_pointer<decltype(this)>::type>::value; } case ::hydra::InterfaceType::IAudio: { return ::hydra::type_traits::is_base_of<hydra::IAudio, ::hydra::type_traits::remove_pointer<decltype(this)>::type>::value; } case ::hydra::InterfaceType::IInput: { return ::hydra::type_traits::is_base_of<hydra::IInput, ::hydra::type_traits::remove_pointer<decltype(this)>::type>::value; } case ::hydra::InterfaceType::ISaveState: { return ::hydra::type_traits::is_base_of<hydra::ISaveState, ::hydra::type_traits::remove_pointer<decltype(this)>::type>::value; } case ::hydra::InterfaceType::IMultiplayer: { return ::hydra::type_traits::is_base_of<hydra::IMultiplayer, ::hydra::type_traits::remove_pointer<decltype(this)>::type>::value; } case ::hydra::InterfaceType::ILog: { return ::hydra::type_traits::is_base_of<hydra::ILog, ::hydra::type_traits::remove_pointer<decltype(this)>::type>::value; } default: return false; \
                } \
            } \
            ::hydra::IBase* getIBase() override { if (hasInterface(::hydra::InterfaceType::IBase)) { return (::hydra::IBase*)(this); } return nullptr; } ::hydra::IFrontendDriven* getIFrontendDriven() override { if (hasInterface(::hydra::InterfaceType::IFrontendDriven)) { return (::hydra::IFrontendDriven*)(this); } return nullptr; } ::hydra::ISelfDriven* getISelfDriven() override { if (hasInterface(::hydra::InterfaceType::ISelfDriven)) { return (::hydra::ISelfDriven*)(this); } return nullptr; } ::hydra::ISoftwareRendered* getISoftwareRendered() override { if (hasInterface(::hydra::InterfaceType::ISoftwareRendered)) { return (::hydra::ISoftwareRendered*)(this); } return nullptr; } ::hydra::IOpenGlRendered* getIOpenGlRendered() override { if (hasInterface(::hydra::InterfaceType::IOpenGlRendered)) { return (::hydra::IOpenGlRendered*)(this); } return nullptr; } ::hydra::IAudio* getIAudio() override { if (hasInterface(::hydra::InterfaceType::IAudio)) { return (::hydra::IAudio*)(this); } return nullptr; } ::hydra::IInput* getIInput() override { if (hasInterface(::hydra::InterfaceType::IInput)) { return (::hydra::IInput*)(this); } return nullptr; } ::hydra::ISaveState* getISaveState() override { if (hasInterface(::hydra::InterfaceType::ISaveState)) { return (::hydra::ISaveState*)(this); } return nullptr; } ::hydra::IMultiplayer* getIMultiplayer() override { if (hasInterface(::hydra::InterfaceType::IMultiplayer)) { return (::hydra::IMultiplayer*)(this); } return nullptr; } ::hydra::ILog* getILog() override { if (hasInterface(::hydra::InterfaceType::ILog)) { return (::hydra::ILog*)(this); } return nullptr; } \
        private: \
            void youForgotToAddHydraCoreMacroToYourClass() override {}

    
    // Macros to generate HYDRA_CLASS if new interfaces are added
    // #define X_HYDRA_INTERFACE(name) case ::hydra::InterfaceType::name: { return ::hydra::type_traits::is_base_of<hydra::name, ::hydra::type_traits::remove_pointer<decltype(this)>::type>::value; }
    // public:
    //     bool hasInterface(::hydra::InterfaceType interface) override {
    //         switch (interface) {
    //             X_HYDRA_INTERFACES
    //         }
    //     }
    // private:
    //     void youForgotToAddHydraCoreMacroToYourClass() override {}
    // #define X_HYDRA_INTERFACE(name) ::hydra::name* get##name() override { if (hasInterface(::hydra::InterfaceType::name)) { return (::hydra::name*)(this); } return nullptr; }
    // X_HYDRA_INTERFACES
}
