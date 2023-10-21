# hydra core API documentation

The hydra header exposes interfaces that an implementation class can inherit from to create an emulator. For example, an emulator that uses or can use OpenGL for rendering should inherit the `GlEmulatorInterface` interface which provides functions for hooking the emulator's GL renderer with the frontend.
You can inherit as many or as few of these interfaces as you want, except for `IBase` which must always be inherited.

Currently available interfaces:
- BaseEmulatorInterface: Provides functions for most basic functionality such as loading a file. Cores should typically implement this.
- GlEmulatorInterface: Provides functions for communicating between an emulator's OpenGL renderer and the frontend. Cores that can use OpenGL for rendering should implement this.
- FrontendDrivenEmulatorInterface: Provides functions for emulators that expect to be driven by the frontend. Cores that work like this should inherit from it

Your class will then need to use the `HYDRA_CLASS` macro inside its definition (much like Qts `Q_OBJECT`) which will define a bunch of getter functions
such as `asIAudioInterface()`. The definition of these functions will check if you inherit from them, and either return a casted `this` or `nullptr`.

Example implementation of a Hydra core using OpenGL for rendering:
```cpp
class HC_GLOBAL HydraCore final : public hydra::BaseEmulatorInterface, public hydra::GlEmulatorInterface, public hydra::FrontendDrivenEmulatorInterface
{
    HYDRA_CLASS
public:
    HydraCore();
    virtual ~HydraCore();
private:
    bool loadFile(const char* type, const char* path) override;
    void reset() override;
    hydra::Size getNativeSize() override;
    void setOutputSize(hydra::Size size) override;
    void setFbo(unsigned handle) override;
    void setContext(void* context) override;
    void setGetProcAddress(void* function) override;
    void runFrame() override;

    std::unique_ptr<Emulator> emulator;
    RendererGL* renderer;
};
```

### Why not dynamic_cast?
I was unable to get `dynamic_cast` working on android, and it seems that it's way less well defined (a lot of implementation specific things) than the way we do things.

### Why not use STL?
Using STL may break stuff if the STL version between the core and the frontend is different.

### Why both a hasInterface function and a asInterface function?
While asInterface will return null if an interface is not supported, running the hasInterface function first is vital for the frontend so that if more interfaces are added previous core builds don't break

In order to make a hydra core, you need 3 functions defined and a class

`HC_API IBase* createEmulator();`
`HC_API void destroyEmulator(IBase*)`
`HC_API const char* getInfo(InfoType info)`

