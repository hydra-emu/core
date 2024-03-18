/**
    The hydra core API

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

//// Frequently Asked Questions

/// 1. What is hydra-emu?

// hydra-emu is a multi-system emulator frontend. The design philosophy of hydra is based around abstraction, it aims
// to define an interface that emulators from all generations can use and seemlessly integrate.

/// 2. Why use the hydra core API, as a developer?

// By using hydra you get access to emulator frontends for multiple systems, which allows you to focus on more important things such
// as emulator development.
// hydra is designed by emulator developers, for emulator developers, and aims to solve problems that similar multi-system emulator frontends
// have faced in the past, and is forever free and open source.

/// 3. Why use the hydra emulator frontend, as a user?

// We aim to provide a pleasant, free, and portable emulation experience to users.

/* ============================================================ */

//// Developer reference

/// 1. Decisions you have to make

/// When using the hydra core API, you have to make a few fundamental decisions about your core.

// Decision 1: Am I gonna let the frontend drive the emulation at a framerate that I specify, or am I going to drive the emulation loop myself?
// Depending on your choice, do the following:

// If you want the frontend to drive the emulation loop, set HcDriveMode to HC_DRIVE_MODE_FRONTEND_DRIVEN in your HcEmulatorInfo,
// and run hcSetCallbacks to setup the functions the frontend will run to control your emulator.
// Synchronization is handled by the frontend in this case. This means that as long as you're inside the runFrame function you
// set using hcSetCallbacks, you can use HcPushVideo/HcPushAudio and the graphics API primitives as you wish.

// If you want your core to drive the emulation loop, set HcDriveMode to HC_DRIVE_MODE_SELF_DRIVEN in your HcEmulatorInfo,
// and do the following:
// - Run your emulation loop as normal, starting at when hcSetEmulatorRunState sets the emulator to running
// - When you want to render a frame, issue a HcLockRequest with name = HC_LOCK_NAME_VIDEO and lock = true to sync the video output
// and proceed as you would in frontend-driven cores (either by calling hcPushVideoFrame if you're software rendering, or by using your selected graphics API)
// After you're done, unlock the video semaphore with another HcLockRequest, this time with lock = false
// - Similarly, when you want to render audio, issue a HcLockRequest with name = HC_LOCK_NAME_AUDIO and lock = true, use hcPushAudioFrame
// then unlock the audio semaphore with another HcLockRequest, this time with lock = false
// - For input, run hcGetInputsSync with an array of requests and an appropriately sized array of responses which will
// lock the emulation thread until the input is ready and write responses to the response array you passed.
// - The frontend has the right to pause or stop emulation at any moment by using hcSetEmulatorRunState. Because of this,
// make sure to issue a HcLockRequest with name = HC_LOCK_NAME_RUN_STATE every time the current emulator run state is read or written.

// Warning: Locks are supposed to be locked for as little time as possible! So for example, for the run state lock, do this:
/*
```c
void run() {
    while (true) {
        hcLockRequest(&lockRunState);
        if (!running) break;
        hcLockRequest(&unlockRunState);

        heavyEmulatorStuff();
    }
}
```
*/
// do NOT do this:
/*
```c
void run() {
    while (true) {
        hcLockRequest(&lockRunState);
        if (!running) break;

        heavyEmulatorStuff();
        hcLockRequest(&unlockRunState);
    }
}
```
*/
// because then the frontend will wait for the entire frame and the frontend/emulator thread separation will be detrimental

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define HYDRA_CORE_MAJOR 0
#define HYDRA_CORE_MINOR 1
#define HYDRA_CORE_PATCH 0

#ifdef _WIN32
#define HYDRA_API_IMPORT __declspec(dllimport)
#define HYDRA_API_EXPORT __declspec(dllexport)
#else
#define HYDRA_API_IMPORT __attribute__((visibility("default")))
#define HYDRA_API_EXPORT __attribute__((visibility("default")))
#endif

#define HYDRA_API_ATTR
#define HYDRA_API_CALL

#ifdef __cplusplus
extern "C" {
#endif

typedef enum HcResult {
    HC_SUCCESS = 0,
    HC_ERROR = -1,
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
    HC_ARCHITECTURE_NULL = 0,
    HC_ARCHITECTURE_X86_64 = 1,
    HC_ARCHITECTURE_AARCH64 = 2,
    HC_ARCHITECTURE_WASM = 3,
} HcArchitecture;

typedef enum HcOperatingSystem {
    HC_OPERATING_SYSTEM_NULL = 0,
    HC_OPERATING_SYSTEM_LINUX = 1,
    HC_OPERATING_SYSTEM_WINDOWS = 2,
    HC_OPERATING_SYSTEM_MACOS = 3,
    HC_OPERATING_SYSTEM_FREEBSD = 4,
    HC_OPERATING_SYSTEM_ANDROID = 5,
    HC_OPERATING_SYSTEM_IOS = 6,
    HC_OPERATING_SYSTEM_WEB = 7,
} HcOperatingSystem;

typedef enum HcDriveMode {
    HC_DRIVE_MODE_NULL = 0,
    HC_DRIVE_MODE_SELF_DRIVEN = 1,
    HC_DRIVE_MODE_FRONTEND_DRIVEN = 2,
} HcDriveMode;

typedef enum HcStructureType {
    HC_STRUCTURE_TYPE_NULL,
    HC_STRUCTURE_TYPE_EMULATOR_INFO,
    HC_STRUCTURE_TYPE_HOST_INFO,
    HC_STRUCTURE_TYPE_VIDEO_INFO,
    HC_STRUCTURE_TYPE_AUDIO_INFO,
    HC_STRUCTURE_TYPE_IMAGE_DATA,
    HC_STRUCTURE_TYPE_AUDIO_DATA,
    HC_STRUCTURE_TYPE_EMULATOR_CREATE_INFO,
    HC_STRUCTURE_TYPE_EMULATOR_DESTROY_INFO,
    HC_STRUCTURE_TYPE_EMULATOR_RESET_INFO,
    HC_STRUCTURE_TYPE_GET_INPUT_REQUEST,
    HC_STRUCTURE_TYPE_LOCK_REQUEST,
    HC_STRUCTURE_TYPE_EMULATOR_RUN_STATE_INFO,
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
} HcMetalVersion;

typedef enum HcDirect3DVersion {
    HC_DIRECT3D_NOT_SUPPORTED = 0,
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
    HC_RESET_TYPE_NULL = 0,
    HC_RESET_TYPE_SOFT = 1,
    HC_RESET_TYPE_HARD = 2,
} HcResetType;

typedef enum HcInputType {
    HC_INPUT_TYPE_NULL = 0,
    // TODO: add input types (eg. HC_INPUT_TYPE_ANALOG1_HORIZONTAL or HC_INPUT_TYPE_BUTTON_X)
} HcInputType;

typedef enum HcLockName {
    HC_LOCK_NAME_NULL = 0,
    HC_LOCK_NAME_AUDIO = 1,
    HC_LOCK_NAME_VIDEO = 2,
    HC_LOCK_NAME_RUN_STATE = 3,
} HcLockName;

typedef enum HcEmulatorRunState {
    HC_EMULATOR_RUN_STATE_NULL = 0,
    HC_EMULATOR_RUN_STATE_RUNNING = 1,
    HC_EMULATOR_RUN_STATE_PAUSED = 2,
    HC_EMULATOR_RUN_STATE_STOPPED = 3,
} HcEmulatorRunState;

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
} HcIconInfo;

typedef struct HcAudioData {
    HcStructureType type;
    void* next;
    uint8_t* data;
    HcAudioInfo info;
} HcAudioData;

typedef struct HcEmulatorInfo {
    HcStructureType type;
    void* next;
    HcDriveMode driveMode;
    const char* coreName;
    const char* coreVersion;
    const char* systemName;
    const char* author;
    const char* description;
    const char* website;
    const char* settings;
    const char* license;
    const char** loadableContentTypes;
    HcIconInfo icon;
} HcEmulatorInfo;

typedef struct HcEmulatorCreateInfo {
    HcStructureType type;
    void* next;
    HcVideoInfo video;
    HcAudioInfo audio;
} HcEmulatorCreateInfo;

typedef struct HcEmulatorDestroyInfo {
    HcStructureType type;
    void* next;
} HcEmulatorDestroyInfo;

typedef struct HcEmulatorResetInfo {
    HcStructureType type;
    void* next;
    HcResetType resetType;
} HcEmulatorResetInfo;

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

typedef struct HcLockRequest {
    HcStructureType type;
    void* next;
    HcLockName lockName;
    bool lock;
} HcLockRequest;

typedef struct HcEmulatorRunStateInfo {
    HcStructureType type;
    void* next;
    HcEmulatorRunState runState;
} HcEmulatorRunStateInfo;

typedef struct HcContentInfo {
    HcStructureType type;
    void* next;
    const char* name;
    const char* path;
} HcContentInfo;

typedef struct HcCallbacks {
    HcStructureType type;
    void* next;
    void (*runFrame)(void* emulator);
} HcCallbacks;

/// Imported functions, these are defined by the frontend
HYDRA_API_IMPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcGetHostInfo(HcHostInfo* info);
HYDRA_API_IMPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcGetInputsSync(const HcInputRequest* const* requests, int64_t* const* const values);
HYDRA_API_IMPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcLockRequest(const HcLockRequest* request);
HYDRA_API_IMPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcPushAudioFrame(const HcAudioData* audio);

// For software-rendered emulator cores
HYDRA_API_IMPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcPushVideoFrame(const HcImageData* image);

// For frontend-driven emulator cores
HYDRA_API_IMPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcSetCallbacks(const HcCallbacks* callbacks);

/// Exported functions, these need to be defined by the core
HYDRA_API_EXPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcGetEmulatorInfo(HcEmulatorInfo* info);
HYDRA_API_EXPORT HYDRA_API_ATTR void* HYDRA_API_CALL hcCreateEmulator(const HcEmulatorCreateInfo* info);
HYDRA_API_EXPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcDestroyEmulator(const HcEmulatorDestroyInfo* info, void* emulator);
HYDRA_API_EXPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcResetEmulator(const HcEmulatorResetInfo* info, void* emulator);
HYDRA_API_EXPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcSetEmulatorRunState(const HcEmulatorRunStateInfo* info, void* emulator);
HYDRA_API_EXPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcLoadContent(const HcContentInfo* info, /* optional */ void* emulator);

#ifdef __cplusplus
}
#endif
