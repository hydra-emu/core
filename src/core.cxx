#include <hydra/core.hxx>

namespace hydra
{
#define X_HYDRA_INTERFACE(name) name::~name() {}
    X_HYDRA_INTERFACES
#undef X_HYDRA_INTERFACE
}