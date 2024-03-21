/**
    @file

    The hydra core API

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

#ifndef HYDRA_API_IMPORT
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#ifdef __GNUC__
#define HYDRA_API_IMPORT __attribute__((__dllimport__))
#else
#define HYDRA_API_IMPORT __declspec(dllimport)
#endif
#else
#define HYDRA_API_IMPORT 
#endif
#endif

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
    HC_SUCCESS = 0, ///< The operation was successful
    HC_ERROR_OTHER = -1,  ///< An error occurred
    HC_ERROR_TOO_MANY_INSTANCES = -2, ///< An instance of the core cannot be created because there are too many instances of the core running
    HC_ERROR_NO_SUCH_INSTANCE = -3, ///< This instance of the core does not exist
    HC_ERROR_BAD_CONTENT = -4, ///< The content is not valid
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
    HC_ARCHITECTURE_X86_64 = 1,
    HC_ARCHITECTURE_AARCH64 = 2,
    HC_ARCHITECTURE_WASM = 3,
    HC_ARCHITECTURE_OTHER = 1000,
} HcArchitecture;

typedef enum HcOperatingSystem {
    HC_OPERATING_SYSTEM_LINUX = 1,
    HC_OPERATING_SYSTEM_WINDOWS = 2,
    HC_OPERATING_SYSTEM_MACOS = 3,
    HC_OPERATING_SYSTEM_FREEBSD = 4,
    HC_OPERATING_SYSTEM_ANDROID = 5,
    HC_OPERATING_SYSTEM_IOS = 6,
    HC_OPERATING_SYSTEM_WEB = 7,
    HC_OPERATING_SYSTEM_OTHER = 1000,
} HcOperatingSystem;

typedef enum HcDriveMode {
    HC_DRIVE_MODE_SELF_DRIVEN = 1, ///< The core is responsible for doing everything itself, except for input which is provided by the frontend
    HC_DRIVE_MODE_SELF_DRIVEN_EXCEPT_AUDIO = 2, ///< The core is responsible for doing everything itself, except for input which is provided by the frontend, and audio that is played by pushing audio frames to the frontend
    HC_DRIVE_MODE_FRONTEND_DRIVEN = 3, ///< The frontend drives the core loop. The frontend is responsible for calling the core's runFrame function.
} HcDriveMode;

typedef enum HcStructureType {
    HC_STRUCTURE_TYPE_CORE_INFO = 1,
    HC_STRUCTURE_TYPE_HOST_INFO,
    HC_STRUCTURE_TYPE_VIDEO_INFO,
    HC_STRUCTURE_TYPE_AUDIO_INFO,
    HC_STRUCTURE_TYPE_IMAGE_DATA,
    HC_STRUCTURE_TYPE_AUDIO_DATA,
    HC_STRUCTURE_TYPE_CORE_CREATE_INFO,
    HC_STRUCTURE_TYPE_CORE_DESTROY_INFO,
    HC_STRUCTURE_TYPE_CORE_RESET_INFO,
    HC_STRUCTURE_TYPE_GET_INPUT_REQUEST,
    HC_STRUCTURE_TYPE_LOCK_REQUEST,
    HC_STRUCTURE_TYPE_CORE_RUN_STATE_INFO,
    HC_STRUCTURE_TYPE_CONTENT_INFO,
    HC_STRUCTURE_TYPE_CALLBACKS,
} HcStructureType;

typedef enum HcOpenGlVersion {
    HC_OPENGL_NOT_SUPPORTED = 0,
    HC_OPENGL_VERSION_1_0 = 1,
    HC_OPENGL_VERSION_1_1 = 2,
    HC_OPENGL_VERSION_1_2 = 3,
    HC_OPENGL_VERSION_1_3 = 4,
    HC_OPENGL_VERSION_1_4 = 5,
    HC_OPENGL_VERSION_1_5 = 6,
    HC_OPENGL_VERSION_2_0 = 7,
    HC_OPENGL_VERSION_2_1 = 8,
    HC_OPENGL_VERSION_3_0 = 9,
    HC_OPENGL_VERSION_3_1 = 10,
    HC_OPENGL_VERSION_3_2 = 11,
    HC_OPENGL_VERSION_3_3 = 12,
    HC_OPENGL_VERSION_4_0 = 13,
    HC_OPENGL_VERSION_4_1 = 14,
    HC_OPENGL_VERSION_4_2 = 15,
    HC_OPENGL_VERSION_4_3 = 16,
    HC_OPENGL_VERSION_4_4 = 17,
    HC_OPENGL_VERSION_4_5 = 18,
    HC_OPENGL_VERSION_4_6 = 19,
} HcOpenGlVersion;

typedef enum HcOpenGlEsVersion {
    HC_OPENGL_ES_NOT_SUPPORTED = 0,
    HC_OPENGL_ES_VERSION_1_0 = 1,
    HC_OPENGL_ES_VERSION_1_1 = 2,
    HC_OPENGL_ES_VERSION_2_0 = 3,
    HC_OPENGL_ES_VERSION_3_0 = 4,
    HC_OPENGL_ES_VERSION_3_1 = 5,
    HC_OPENGL_ES_VERSION_3_2 = 6,
} HcOpenGlEsVersion;

typedef enum HcWebGlVersion {
    HC_WEBGL_NOT_SUPPORTED = 0,
    HC_WEBGL_VERSION_1_0 = 1,
    HC_WEBGL_VERSION_2_0 = 2,
} HcWebGlVersion;

typedef enum HcVulkanVersion {
    HC_VULKAN_NOT_SUPPORTED = 0,
    HC_VULKAN_VERSION_1_0 = 1,
    HC_VULKAN_VERSION_1_1 = 2,
    HC_VULKAN_VERSION_1_2 = 3,
    HC_VULKAN_VERSION_1_3 = 4,
} HcVulkanVersion;

typedef enum HcMetalVersion {
    HC_METAL_NOT_SUPPORTED = 0,
    HC_METAL_VERSION_1_0 = 1,
    HC_METAL_VERSION_2_0 = 1,
    HC_METAL_VERSION_3_0 = 1,
} HcMetalVersion;

typedef enum HcDirect3DVersion {
    HC_DIRECT3D_NOT_SUPPORTED = 0,
    HC_DIRECT3D_VERSION_11_0 = 1,
    HC_DIRECT3D_VERSION_12_0 = 2,
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
    HC_RESET_TYPE_SOFT = 1, ///< Whatever the core considers a soft reset, usually equivalent to pressing the reset button on the console
    HC_RESET_TYPE_HARD = 2, ///< Whatever the core considers a hard reset, usually equivalent to turning the console off and on again
} HcResetType;

typedef enum HcInputType {
    HC_INPUT_TYPE_NULL = 0,
    // TODO: add input types (eg. HC_INPUT_TYPE_ANALOG1_HORIZONTAL or HC_INPUT_TYPE_BUTTON_X)
} HcInputType;

typedef enum HcRunState {
    HC_RUN_STATE_NULL = 0,    ///< The core run state is not yet set
    HC_RUN_STATE_RUNNING = 1, ///< The core is running
    HC_RUN_STATE_PAUSED = 2,  ///< The core is paused
    HC_RUN_STATE_QUIT = 3,    ///< The core is stopped and will not be resumed
} HcRunState;

typedef struct HcVideoInfo {
    HcStructureType type;
    void* next;
    HcRendererType renderer;
    uint32_t rendererVersion;
    uint32_t width;
    uint32_t height;
    uint32_t frameRate;
    HcPixelFormat format;
} HcVideoInfo;

typedef struct HcAudioInfo {
    HcStructureType type;
    void* next;
    HcAudioFormat format;
    HcAudioChannels channels;
    uint32_t sampleRate;
} HcAudioInfo;

typedef struct HcImageData {
    HcStructureType type;
    void* next;
    uint8_t* data;
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    uint32_t stride;
    HcPixelFormat format;
} HcImageData;

typedef struct HcAudioData {
    HcStructureType type;
    void* next;
    uint8_t* data;
    HcAudioInfo info;
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
    HcImageData icon;
} HcCoreInfo;

typedef struct HcEnvironmentInfo {
    HcStructureType type;
    void* next;
    HcDriveMode driveMode;
    HcVideoInfo video;
    HcAudioInfo audio;
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

typedef struct HcCallbacks {
    HcStructureType type;
    void* next;
    void* userdata;
    void (*runFrame)(void* userdata);
} HcCallbacks;

/// Imported functions, these are defined by the frontend
HYDRA_API_IMPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcGetHostInfo(HcHostInfo* info);
HYDRA_API_IMPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcGetInputsSync(const HcInputRequest* const* requests, const int64_t* const* const values);
HYDRA_API_IMPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcReconfigureEnvironment(const HcEnvironmentInfo* info, void* instance);

// For software-rendered cores
HYDRA_API_IMPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcSwPushVideoFrame(const HcImageData* image);

// For OpenGL-rendered cores
HYDRA_API_IMPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcGlSwapBuffers();

// For frontend-driven cores
HYDRA_API_IMPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcSetCallbacks(const HcCallbacks* callbacks);

/// Exported functions, these need to be defined by the core

/**
    Get information about the core. Will be called one time, once the core is loaded.
    @param info The HcCoreInfo struct to fill with information about the core.
*/
HYDRA_API_EXPORT HYDRA_API_ATTR void HYDRA_API_CALL hcGetCoreInfo(HcCoreInfo* info);

/**
    Create an instance of the core, and point the instance pointer to it.
    Each core is tied to a window and a particular renderer API, configured from environmentInfo.
    @param[out] environmentInfo Information about the environment the core is running in. This needs to be populated by the core.
    @param[out] instance A pointer to a void* that will be set to the instance of the core.
    @return ::HC_SUCCESS
    @return ::HC_ERROR_TOO_MANY_INSTANCES
    @return ::HC_ERROR_OTHER
*/
HYDRA_API_EXPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcCreate(HcEnvironmentInfo* environmentInfo, void** instance);

/**
    Destroy an instance of the core.
    @param[in] destroyInfo Information about the destruction of the core. Currently unused.
    @param[in] instance The instance of the core to destroy.
    @return ::HC_SUCCESS
    @return ::HC_ERROR_NO_SUCH_INSTANCE
    @return ::HC_ERROR_OTHER
*/
HYDRA_API_EXPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcDestroy(const HcDestroyInfo* destroyInfo, void* instance);

/**
    Reset the core.
    @param[in] resetInfo Information about the reset of the core.
    @param[in] instance The instance of the core to reset.
    @return ::HC_SUCCESS
    @return ::HC_ERROR_NO_SUCH_INSTANCE
    @return ::HC_ERROR_OTHER
*/
HYDRA_API_EXPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcReset(const HcResetInfo* resetInfo, void* instance);

/**
    Set the run state of the core, as in running, paused, or quit.
    @param[in] runInfo Information about the desired run state of the core.
    @param[in] instance The instance of the core to set the run state of.
*/
HYDRA_API_EXPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcSetRunState(const HcRunStateInfo* runInfo, void* instance);

/**
    Load content into the core.
    @param[in] info Information about the content to load.
    @param[in] instance (optional) The instance of the core to load the content into. If this content is not instance-specific, this can be nullptr.
    @return ::HC_SUCCESS
    @return ::HC_ERROR_NO_SUCH_INSTANCE
    @return ::HC_ERROR_BAD_CONTENT
    @return ::HC_ERROR_OTHER
*/
HYDRA_API_EXPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcLoadContent(const HcContentLoadInfo* info, void* instance);

/**
    Return the error message of the last ::HC_ERROR_OTHER that occurred.
    @return A string containing the error message, or nullptr if no error occurred.
*/
HYDRA_API_EXPORT HYDRA_API_ATTR const char* HYDRA_API_CALL hcGetError();

#ifdef __cplusplus
}
#endif
