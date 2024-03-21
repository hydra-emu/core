#include <hydra/core.h>

HcGetHostInfoPtr hcGetHostInfo = NULL;
HcGetInputsSyncPtr hcGetInputsSync = NULL;
HcReconfigureEnvironmentPtr hcReconfigureEnvironment = NULL;
HcPushSamplesPtr hcPushSamples = NULL;
HcSwPushVideoFramePtr hcSwPushVideoFrame = NULL;
HcGlMakeCurrentPtr hcGlMakeCurrent = NULL;
HcGlSwapBuffersPtr hcGlSwapBuffers = NULL;
HcGlGetProcAddressPtr hcGlGetProcAddress = NULL;
HcSetCallbacksPtr hcSetCallbacks = NULL;

/**
    The frontend will call this function with a pointer that can load the function pointers.
    This is pretty much what happens when you use a loader in OpenGL to load the function pointers,
    for example gladLoadGLLoader where you pass something like SDL_GL_GetProcAddress or glfwGetProcAddress etc.
*/
HYDRA_API_EXPORT HYDRA_API_ATTR HcResult HYDRA_API_CALL hcInternalLoadFunctions(void*(*loadFunctionPtr)(const char*)) {
    if (!loadFunctionPtr) {
        return HC_INTERNAL_ERROR_BAD_LOADFUNCTIONPTR;
    }

    hcGetHostInfo = (HcGetHostInfoPtr)loadFunctionPtr("hcGetHostInfo");
    if (!hcGetHostInfo) {
        return HC_INTERNAL_ERROR_MISSING_FUNCTION;
    }

    hcGetInputsSync = (HcGetInputsSyncPtr)loadFunctionPtr("hcGetInputsSync");
    if (!hcGetInputsSync) {
        return HC_INTERNAL_ERROR_MISSING_FUNCTION;
    }

    hcReconfigureEnvironment = (HcReconfigureEnvironmentPtr)loadFunctionPtr("hcReconfigureEnvironment");
    if (!hcReconfigureEnvironment) {
        return HC_INTERNAL_ERROR_MISSING_FUNCTION;
    }

    hcPushSamples = (HcPushSamplesPtr)loadFunctionPtr("hcPushSamples");
    if (!hcPushSamples) {
        return HC_INTERNAL_ERROR_MISSING_FUNCTION;
    }

    hcSwPushVideoFrame = (HcSwPushVideoFramePtr)loadFunctionPtr("hcSwPushVideoFrame");
    if (!hcSwPushVideoFrame) {
        return HC_INTERNAL_ERROR_MISSING_FUNCTION;
    }

    hcGlMakeCurrent = (HcGlMakeCurrentPtr)loadFunctionPtr("hcGlMakeCurrent");
    if (!hcGlMakeCurrent) {
        return HC_INTERNAL_ERROR_MISSING_FUNCTION;
    }

    hcGlSwapBuffers = (HcGlSwapBuffersPtr)loadFunctionPtr("hcGlSwapBuffers");
    if (!hcGlSwapBuffers) {
        return HC_INTERNAL_ERROR_MISSING_FUNCTION;
    }

    hcGlGetProcAddress = (HcGlGetProcAddressPtr)loadFunctionPtr("hcGlGetProcAddress");
    if (!hcGlGetProcAddress) {
        return HC_INTERNAL_ERROR_MISSING_FUNCTION;
    }

    hcSetCallbacks = (HcSetCallbacksPtr)loadFunctionPtr("hcSetCallbacks");
    if (!hcSetCallbacks) {
        return HC_INTERNAL_ERROR_MISSING_FUNCTION;
    }

    return HC_SUCCESS;
}