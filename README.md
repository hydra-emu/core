# hydra core API documentation

The hydra header exposes interfaces that an implementation class can inherit to create an emulator.
You can inherit as many or as few of these interfaces as you want, except for `IBase` which must always be inherited.

Your class will then need to use the `HYDRA_CLASS` macro inside its definition (much like Qts `Q_OBJECT`) which will define a bunch of getter functions
such as `getIAudioInterface()`. The definition of these functions will check if you inherit from them, and either return a casted `this` or `nullptr`.

Example implementation:
```cpp
class HC_GLOBAL HydraCore final : public hydra::BaseEmulatorInterface, public hydra::GlEmulatorInterface, public hydra::FrontendDrivenEmulatorInterface
{
    HYDRA_CLASS
public:
    HydraCore();
    virtual ~HydraCore();
private:
    // 
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

In order to make a hydra core, you need 3 functions defined and a class

`HC_API IBase* createEmulator();`
`HC_API void destroyEmulator(IBase*)`
`HC_API const char* getInfo(InfoType info)`

