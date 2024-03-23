/**
    @file

    @brief The hydra core API

    The hydra core API is a C API that allows for the creation of 'cores' that can be run in a frontend.
    A core is a piece of software that provides some sort of video and/or audio output,
    while also taking controller input from the frontend.
    What this software does is up to the core, but in the case of hydra-emu, cores are emulators for various game consoles.

    The frontend is responsible for creating a window and setting up the renderer API (OpenGL, Vulkan, etc.).
    It then initializes the core. The core can then decide whether it wants to be self-driven or frontend-driven.
    Self-driven cores are responsible for running their own main loop, while frontend-driven cores have their main loop driven by the frontend
    at a configurable rate.
    For self-driven cores, the frontends job is minimal, providing input when requested, swapping buffers when needed, optionally providing audio output,
    and setting the run state of the core.


    MIT License

    Copyright (c) 2024 hydra-emu

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

/* ============================================================ */

/// Structures in the hydra core API follow Vulkan's style of struct chaining, where you can provide extensions to structs.
/// This means that each struct a pointer `next` to the next struct in the chain.
/// and a `type` field to identify the type of the next struct.
/// This is useful for future-proofing the API, as it allows for adding new fields to existing structs without breaking compatibility.
/// If a structure allows for extensions, it will explicitly say so in the documentation for that structure.

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define HYDRA_CORE_MAJOR 0
#define HYDRA_CORE_MINOR 1
#define HYDRA_CORE_PATCH 0

#ifndef HYDRA_API_EXPORT
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#ifdef __GNUC__
#define HYDRA_API_EXPORT __attribute__((__dllexport__))
#else
#define HYDRA_API_EXPORT __declspec(dllexport)
#endif
#else
#define HYDRA_API_EXPORT __attribute__((visibility("default")))
#endif
#endif

#ifndef HYDRA_API_CALL
#if defined(__GNUC__) && defined(__i386__) && !defined(__x86_64__)
#define HYDRA_API_CALL __attribute__((cdecl))
#elif defined(_MSC_VER) && defined(_M_X86) && !defined(_M_X64)
#define HYDRA_API_CALL __cdecl
#else
#define HYDRA_API_CALL
#endif
#endif

#ifndef HYDRA_API_ATTR
#define HYDRA_API_ATTR
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum HcResult {
    HC_SUCCESS = 0, ///< The operation was successful.
    HC_ERROR_CORE = -1001,  ///< An error occurred by the core. Core should provide hcGetError with more information.
    HC_ERROR_NOT_ALL_CALLBACKS_SET = -2001, ///< Not all callbacks were set.
    HC_ERROR_WRONG_DRIVE_MODE, ///< The drive mode conflicts with what the core tries to do. Eg. setting frontend-driven callbacks in a self-driven core.
    HC_ERROR_NULL_DATA_PASSED, ///< A null pointer was passed to a function that doesn't accept null pointers or a member pointer was null.
    HC_ERROR_BAD_RENDERER_VERSION, ///< The renderer version is not supported.
    HC_ERROR_BAD_CONTENT, ///< The content is not valid.
    HC_ERROR_BAD_INPUT_REQUEST, ///< The input request is not valid.
    HC_ERROR_BAD_ENVIRONMENT_INFO, ///< The environment info is not valid.
    HC_ERROR_BAD_AUDIO_DATA_WANT, ///< The audio data's want field is not valid.
    HC_ERROR_BAD_AUDIO_DATA_HAVE, ///< The audio data's have field is not valid.
    HC_ERROR_AUDIO_OVERRUN, ///< The audio buffer is full and cannot accept more audio data.
    HC_ERROR_AUDIO_FULLY_SELF_DRIVEN, ///< The core should be fully self-driven so the frontend can't accept audio data.
    HC_ERROR_NOT_SOFTWARE_RENDERED, ///< The core is not software rendered.
    HC_ERROR_NOT_OPENGL_RENDERED, ///< The core is not OpenGL rendered.
    HC_ERROR_NOT_VULKAN_RENDERED, ///< The core is not Vulkan rendered.
    HC_ERROR_NOT_METAL_RENDERED, ///< The core is not Metal rendered.
    HC_ERROR_NOT_DIRECT3D_RENDERED, ///< The core is not Direct3D rendered.
    HC_INTERNAL_ERROR_BAD_LOADFUNCTIONPTR = -5001, ///< The load function pointer is not valid during hcInternalLoadFunctions.
    HC_INTERNAL_ERROR_MISSING_FUNCTION = -5002, ///< A function is missing during hcInternalLoadFunctions.
    HC_INTERNAL_ERROR_WRAPPER_NOT_INITIALIZED = -5003, ///< The wrapper is not initialized for whatever reason.
} HcResult;

typedef enum HcPixelFormat {
    HC_PIXEL_FORMAT_NULL = 0,
    HC_PIXEL_FORMAT_RGBA32 = 1,
    HC_PIXEL_FORMAT_BGRA32 = 2,
    HC_PIXEL_FORMAT_ARGB32 = 3,
    HC_PIXEL_FORMAT_ABGR32 = 4,
    HC_PIXEL_FORMAT_RGB24 = 5,
    HC_PIXEL_FORMAT_BGR24 = 6,
    HC_PIXEL_FORMAT_RGB565 = 7,
    HC_PIXEL_FORMAT_BGR565 = 8,
    HC_PIXEL_FORMAT_RGBA5551 = 9,
    HC_PIXEL_FORMAT_BGRA5551 = 10,
    HC_PIXEL_FORMAT_ARGB1555 = 11,
    HC_PIXEL_FORMAT_ABGR1555 = 12,
} HcPixelFormat;

typedef enum HcArchitecture {
    HC_ARCHITECTURE_UNKNOWN,
    HC_ARCHITECTURE_X86,
    HC_ARCHITECTURE_X86_64,
    HC_ARCHITECTURE_AARCH32,
    HC_ARCHITECTURE_AARCH64,
    HC_ARCHITECTURE_WASM,
    HC_ARCHITECTURE_OTHER,
} HcArchitecture;

typedef enum HcOperatingSystem {
    HC_OPERATING_SYSTEM_UNKNOWN,
    HC_OPERATING_SYSTEM_LINUX,
    HC_OPERATING_SYSTEM_WINDOWS,
    HC_OPERATING_SYSTEM_MACOS,
    HC_OPERATING_SYSTEM_FREEBSD,
    HC_OPERATING_SYSTEM_ANDROID,
    HC_OPERATING_SYSTEM_IOS,
    HC_OPERATING_SYSTEM_WEB,
    HC_OPERATING_SYSTEM_OTHER,
} HcOperatingSystem;

typedef enum HcDriveMode {
    HC_DRIVE_MODE_NULL = 0, ///< The drive mode is not yet set.
    HC_DRIVE_MODE_SELF_DRIVEN = 1, ///< The core is responsible for doing everything itself, except for input which is provided by the frontend.
    HC_DRIVE_MODE_SELF_DRIVEN_EXCEPT_AUDIO = 2, ///< The core is responsible for doing everything itself, except for input which is provided by the frontend, and audio that is played by pushing audio frames to the frontend.
    HC_DRIVE_MODE_FRONTEND_DRIVEN = 3, ///< The frontend drives the core loop. The frontend is responsible for calling the core's runFrame function.
} HcDriveMode;

typedef enum HcStructureType {
    HC_STRUCTURE_TYPE_CORE_INFO = 1,
    HC_STRUCTURE_TYPE_HOST_INFO,
    HC_STRUCTURE_TYPE_VIDEO_INFO,
    HC_STRUCTURE_TYPE_AUDIO_INFO,
    HC_STRUCTURE_TYPE_IMAGE_DATA,
    HC_STRUCTURE_TYPE_AUDIO_DATA,
    HC_STRUCTURE_TYPE_CORE_DESTROY_INFO,
    HC_STRUCTURE_TYPE_CORE_RESET_INFO,
    HC_STRUCTURE_TYPE_GET_INPUT_REQUEST,
    HC_STRUCTURE_TYPE_LOCK_REQUEST,
    HC_STRUCTURE_TYPE_CORE_RUN_STATE_INFO,
    HC_STRUCTURE_TYPE_CONTENT_INFO,
    HC_STRUCTURE_TYPE_CALLBACKS,
    HC_STRUCTURE_TYPE_CONTENT_LOAD_INFO,
    HC_STRUCTURE_TYPE_ENVIRONMENT_INFO,
} HcStructureType;

typedef enum HcOpenGlVersion {
    HC_OPENGL_NOT_SUPPORTED = 0,
    HC_OPENGL_VERSION_1_0 = (1 << 16) | 0,
    HC_OPENGL_VERSION_1_1 = (1 << 16) | 1,
    HC_OPENGL_VERSION_1_2 = (1 << 16) | 2,
    HC_OPENGL_VERSION_1_3 = (1 << 16) | 3,
    HC_OPENGL_VERSION_1_4 = (1 << 16) | 4,
    HC_OPENGL_VERSION_1_5 = (1 << 16) | 5,
    HC_OPENGL_VERSION_2_0 = (2 << 16) | 0,
    HC_OPENGL_VERSION_2_1 = (2 << 16) | 1,
    HC_OPENGL_VERSION_3_0 = (3 << 16) | 0,
    HC_OPENGL_VERSION_3_1 = (3 << 16) | 1,
    HC_OPENGL_VERSION_3_2 = (3 << 16) | 2,
    HC_OPENGL_VERSION_3_3 = (3 << 16) | 3,
    HC_OPENGL_VERSION_4_0 = (4 << 16) | 0,
    HC_OPENGL_VERSION_4_1 = (4 << 16) | 1,
    HC_OPENGL_VERSION_4_2 = (4 << 16) | 2,
    HC_OPENGL_VERSION_4_3 = (4 << 16) | 3,
    HC_OPENGL_VERSION_4_4 = (4 << 16) | 4,
    HC_OPENGL_VERSION_4_5 = (4 << 16) | 5,
    HC_OPENGL_VERSION_4_6 = (4 << 16) | 6,
} HcOpenGlVersion;

typedef enum HcOpenGlEsVersion {
    HC_OPENGL_ES_NOT_SUPPORTED = 0,
    HC_OPENGL_ES_VERSION_1_0 = (1 << 16) | 0,
    HC_OPENGL_ES_VERSION_1_1 = (1 << 16) | 1,
    HC_OPENGL_ES_VERSION_2_0 = (2 << 16) | 0,
    HC_OPENGL_ES_VERSION_3_0 = (3 << 16) | 0,
    HC_OPENGL_ES_VERSION_3_1 = (3 << 16) | 1,
    HC_OPENGL_ES_VERSION_3_2 = (3 << 16) | 2,
} HcOpenGlEsVersion;

typedef enum HcWebGlVersion {
    HC_WEBGL_NOT_SUPPORTED = 0,
    HC_WEBGL_VERSION_1_0 = 1,
    HC_WEBGL_VERSION_2_0 = 2,
} HcWebGlVersion;

typedef enum HcVulkanVersion {
    HC_VULKAN_NOT_SUPPORTED = 0,
    HC_VULKAN_VERSION_1_0 = (1 << 16) | 0,
    HC_VULKAN_VERSION_1_1 = (1 << 16) | 1,
    HC_VULKAN_VERSION_1_2 = (1 << 16) | 2,
    HC_VULKAN_VERSION_1_3 = (1 << 16) | 3,
} HcVulkanVersion;

typedef enum HcMetalVersion {
    HC_METAL_NOT_SUPPORTED = 0,
    HC_METAL_VERSION_1_0 = (1 << 16) | 0,
    HC_METAL_VERSION_2_0 = (2 << 16) | 0,
    HC_METAL_VERSION_3_0 = (3 << 16) | 0,
} HcMetalVersion;

typedef enum HcDirect3DVersion {
    HC_DIRECT3D_NOT_SUPPORTED,
    HC_DIRECT3D_VERSION_7_0,
    HC_DIRECT3D_VERSION_8_0,
    HC_DIRECT3D_VERSION_9_0,
    HC_DIRECT3D_VERSION_10_0,
    HC_DIRECT3D_VERSION_11_0,
    HC_DIRECT3D_VERSION_12_0,
} HcDirect3DVersion;

typedef enum HcRendererType {
    HC_RENDERER_TYPE_NULL = 0,
    HC_RENDERER_TYPE_SOFTWARE = 1,
    HC_RENDERER_TYPE_OPENGL = 2,
    HC_RENDERER_TYPE_OPENGL_ES = 3,
    HC_RENDERER_TYPE_WEBGL = 4,
    HC_RENDERER_TYPE_VULKAN = 5,
    HC_RENDERER_TYPE_METAL = 6,
    HC_RENDERER_TYPE_DIRECT3D = 7,
} HcRendererType;

typedef enum HcAudioFormat {
    HC_AUDIO_FORMAT_NULL = 0,
    HC_AUDIO_FORMAT_U8_PCM = 1,
    HC_AUDIO_FORMAT_S8_PCM = 2,
    HC_AUDIO_FORMAT_S16_PCM = 3,
    HC_AUDIO_FORMAT_S24_PCM = 4,
    HC_AUDIO_FORMAT_S32_PCM = 5,
    HC_AUDIO_FORMAT_FLOAT32 = 6,
    HC_AUDIO_FORMAT_FLOAT64 = 7,
} HcAudioFormat;

typedef enum HcAudioChannels {
    HC_AUDIO_CHANNELS_NULL = 0,
    HC_AUDIO_CHANNELS_MONO = 1,
    HC_AUDIO_CHANNELS_STEREO = 2,
    HC_AUDIO_CHANNELS_3_1_SURROUND = 4,
    HC_AUDIO_CHANNELS_5_1_SURROUND = 6,
    HC_AUDIO_CHANNELS_7_1_SURROUND = 8,
} HcAudioChannels;

typedef enum HcResetType {
    HC_RESET_TYPE_SOFT = 1, ///< Whatever the core considers a soft reset, usually equivalent to pressing the reset button on the console.
    HC_RESET_TYPE_HARD = 2, ///< Whatever the core considers a hard reset, usually equivalent to turning the console off and on again.
} HcResetType;

typedef enum HcInputType {
    HC_INPUT_TYPE_NULL = 0,
    // TODO: add input types (eg. HC_INPUT_TYPE_ANALOG1_HORIZONTAL or HC_INPUT_TYPE_BUTTON_X)
} HcInputType;

typedef enum HcRunState {
    HC_RUN_STATE_NULL = 0,    ///< The core run state is not yet set.
    HC_RUN_STATE_RUNNING = 1, ///< The core is running.
    HC_RUN_STATE_PAUSED = 2,  ///< The core is paused.
    HC_RUN_STATE_QUIT = 3,    ///< The core is stopped and will not be resumed.
} HcRunState;

typedef struct HcVideoInfo {
    HcStructureType type;
    void* next;
    HcRendererType rendererType; ///< The type of renderer the core wants to use.
    uint32_t rendererVersion; ///< The version of the renderer the core wants to use.
    uint32_t width; ///< The width of the video output.
    uint32_t height; ///< The height of the video output.
    uint32_t frameRate; ///< The frame rate of the video output, in frames per second.
    HcPixelFormat format; ///< The pixel format of the video output.
} HcVideoInfo;

typedef struct HcAudioInfo {
    HcStructureType type;
    void* next;
    HcAudioFormat format; ///< The audio sample format.
    HcAudioChannels channels; ///< The number of audio channels.
    uint32_t sampleRate; ///< The sample rate of the audio output, in samples per second.
} HcAudioInfo;

typedef struct HcImageData {
    HcStructureType type;
    void* next;
    uint8_t* data; ///< The image bytes. Should be sized stride * height.
    uint32_t width; ///< The width of the image.
    uint32_t height; ///< The height of the image.
    uint32_t channels; ///< The number of channels in the image. Eg. 3 for RGB, 4 for RGBA.
    uint32_t stride; ///< The number of bytes in a row of the image. Should be at least width * channels.
    HcPixelFormat format;
} HcImageData;

typedef struct HcAudioData {
    HcStructureType type;
    void* next;
    uint8_t* data;
    uint32_t sampleCount;
    HcAudioInfo want;
    HcAudioInfo have;
} HcAudioData;

typedef struct HcContentInfo {
    HcStructureType type;
    void* next;
    const char* name;
    const char* description;
    const char* extensions;
} HcContentInfo;

typedef struct HcCoreInfo {
    HcStructureType type;
    void* next;
    const char* coreName;
    const char* coreVersion;
    const char* systemName;
    const char* author;
    const char* description;
    const char* website;
    const char* settings;
    const char* license;
    HcContentInfo* loadableContentInfo;
    int loadableContentInfoCount;
    HcImageData* icon;
} HcCoreInfo;

typedef struct HcEnvironmentInfo {
    HcStructureType type;
    void* next;
    HcDriveMode driveMode;
    HcVideoInfo* video;
    HcAudioInfo* audio;
} HcEnvironmentInfo;

typedef struct HcDestroyInfo {
    HcStructureType type;
    void* next;
} HcDestroyInfo;

typedef struct HcResetInfo {
    HcStructureType type;
    void* next;
    HcResetType resetType;
} HcResetInfo;

typedef struct HcHostInfo {
    HcStructureType type;
    void* next;
    HcArchitecture architecture;
    HcOperatingSystem operatingSystem;
    const char* gpuVendor;
    HcOpenGlVersion openGlVersion;
    HcOpenGlEsVersion openGlEsVersion;
    HcWebGlVersion webGlVersion;
    HcVulkanVersion vulkanVersion;
    HcMetalVersion metalVersion;
    HcDirect3DVersion direct3DVersion;
} HcHostInfo;

typedef struct HcInputRequest {
    HcStructureType type;
    void* next;
    uint32_t port;
    HcInputType inputType;
} HcInputRequest;

typedef struct HcRunStateInfo {
    HcStructureType type;
    void* next;
    HcRunState runState;
} HcRunStateInfo;

typedef struct HcContentLoadInfo {
    HcStructureType type;
    void* next;
    const char* name;
    const char* path;
} HcContentLoadInfo;

typedef struct HcFrontendDrivenCallbacks {
    HcStructureType type;
    void* next;
    void (*runFrame)();
} HcFrontendDrivenCallbacks;

typedef struct HcSelfDrivenCallbacks {
    HcStructureType type;
    void* next;
    void (*entryPoint)();
} HcSelfDrivenCallbacks;

typedef struct HcCallbacks {
    HcStructureType type;
    void* next;
    HcFrontendDrivenCallbacks* frontendDrivenCallbacks;
    HcSelfDrivenCallbacks* selfDrivenCallbacks;
} HcCallbacks;

/// Imported functions, these are defined by the frontend

/**
    Get information about the host system.
    @param hostInfo Will be filled with information about the host system.
*/
typedef void (HYDRA_API_CALL *HcGetHostInfoPtr)(HcHostInfo* hostInfo);
extern HcGetHostInfoPtr hcGetHostInfo;

/**
    Request input state from the frontend.
    @param requests An array of pointers to HcInputRequest structs, each containing information about the input to request.
    @param requestCount The number of input requests in the requests array.
    @param values An array of pointers to int64_t, each containing the value of the input requested.
    @return ::HC_SUCCESS
    @return ::HC_ERROR_BAD_INPUT_REQUEST
*/
typedef HcResult (HYDRA_API_CALL *HcGetInputsSyncPtr)(const HcInputRequest* const* requests, int requestCount, const int64_t* const* values);
extern HcGetInputsSyncPtr hcGetInputsSync;

/**
    Reconfigure the environment the core is running in, for example when the window is resized or the frame rate changes in frontend-driven cores.
    @param environmentInfo Information about the new environment. Renderer changes are ignored.
    @return ::HC_SUCCESS
    @return ::HC_ERROR_BAD_ENVIRONMENT_INFO
*/
typedef HcResult (HYDRA_API_CALL *HcReconfigureEnvironmentPtr)(const HcEnvironmentInfo* environmentInfo);
extern HcReconfigureEnvironmentPtr hcReconfigureEnvironment;

/**
    For not fully self-driven cores (so cores that use ::HC_DRIVE_MODE_SELF_DRIVEN_EXCEPT_AUDIO or ::HC_DRIVE_MODE_FRONTEND_DRIVEN),
    this function is called by the core to push an arbitrary amount of audio samples to the frontend.
    @param audioData The audio data to push to the frontend.
    @return ::HC_SUCCESS
    @return ::HC_ERROR_BAD_AUDIO_DATA_WANT
    @return ::HC_ERROR_BAD_AUDIO_DATA_HAVE
    @return ::HC_ERROR_AUDIO_OVERRUN
    @return ::HC_ERROR_FULLY_SELF_DRIVEN
*/
typedef HcResult (HYDRA_API_CALL *HcPushSamplesPtr)(const HcAudioData* audioData);
extern HcPushSamplesPtr hcPushSamples;

/**
    For software rendered cores, this function is called by the core to push a video frame to the frontend.
    @param image The image data to push to the frontend.
    @return ::HC_SUCCESS
    @return ::HC_ERROR_NOT_SOFTWARE_RENDERED
*/
typedef HcResult (HYDRA_API_CALL *HcSwPushVideoFramePtr)(const HcImageData* image);
extern HcSwPushVideoFramePtr hcSwPushVideoFrame;

/**
    For OpenGL rendered cores, this function is called by the core to make the current thread owner of the OpenGL context.
    Should be called before the core starts rendering.
    @return ::HC_SUCCESS
    @return ::HC_ERROR_NOT_OPENGL_RENDERED
*/
typedef HcResult (HYDRA_API_CALL *HcGlMakeCurrentPtr)();
extern HcGlMakeCurrentPtr hcGlMakeCurrent;

/**
    For OpenGL rendered cores, this function is called by the core to swap buffers and render any overlays.
    Should be called after the core has finished rendering a frame.
    @return ::HC_SUCCESS
    @return ::HC_ERROR_NOT_OPENGL_RENDERED
*/
typedef HcResult (HYDRA_API_CALL *HcGlSwapBuffersPtr)();
extern HcGlSwapBuffersPtr hcGlSwapBuffers;

/**
    For OpenGL rendered cores, this function is called by the core to get a function pointer to an OpenGL function.
    You can use it with GLAD or similar functions to load the OpenGL functions, as you normally would.
    @param name The name of the OpenGL function to get a pointer to.
    @return A pointer to the OpenGL function, or nullptr if the function is not found.
*/
typedef void* (HYDRA_API_CALL *HcGlGetProcAddressPtr)(const char* name);
extern HcGlGetProcAddressPtr hcGlGetProcAddress;

/**
    For frontend-driven cores, this function sets the callbacks that the frontend will call to drive the core's main loop.
    @param callbacks The callbacks to set.
    @return ::HC_SUCCESS
    @return ::HC_ERROR_NOT_ALL_CALLBACKS_SET
*/
typedef HcResult (HYDRA_API_CALL *HcSetCallbacksPtr)(const HcCallbacks* callbacks);
extern HcSetCallbacksPtr hcSetCallbacks;

/**
    Get information about the core. Will be called one time, once the core is loaded.
    @param coreInfo The HcCoreInfo struct to fill with information about the core.
*/
HYDRA_API_EXPORT HYDRA_API_ATTR void HYDRA_API_CALL hcGetCoreInfo(HcCoreInfo* coreInfo);

/**
    Initialize the core.
    Each core is tied to a window and a particular renderer API, configured from environmentInfo.
    @param[out] environmentInfo Information about the environment the core is running in. This needs to be populated by the core.
    @return ::HC_SUCCESS
    @return ::HC_ERROR_CORE
*/
HYDRA_API_EXPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcCreate(HcEnvironmentInfo* environmentInfo);

/**
    Destroy the core, freeing any resources it has allocated.
    @param[in] destroyInfo Information about the destruction of the core. Currently unused.
    @return ::HC_SUCCESS
    @return ::HC_ERROR_CORE
*/
HYDRA_API_EXPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcDestroy(const HcDestroyInfo* destroyInfo);

/**
    Reset the core.
    @param[in] resetInfo Information about the reset of the core.
    @return ::HC_SUCCESS
    @return ::HC_ERROR_CORE
*/
HYDRA_API_EXPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcReset(const HcResetInfo* resetInfo);

/**
    Set the run state of the core, as in running, paused, or quit.
    @param[in] runInfo Information about the desired run state of the core.
*/
HYDRA_API_EXPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcSetRunState(const HcRunStateInfo* runInfo);

/**
    Load content into the core.
    @param[in] info Information about the content to load.
    @return ::HC_SUCCESS
    @return ::HC_ERROR_BAD_CONTENT
    @return ::HC_ERROR_CORE
*/
HYDRA_API_EXPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcLoadContent(const HcContentLoadInfo* info);

/**
    Return the error message of the last ::HC_ERROR_CORE that occurred.
    @return A string containing the error message, or nullptr if no error occurred.
*/
HYDRA_API_EXPORT HYDRA_API_ATTR const char* HYDRA_API_CALL hcGetError();

#ifdef __cplusplus
}
#endif
