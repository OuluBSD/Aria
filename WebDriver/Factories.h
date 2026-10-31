#ifndef _WebDriver_Factories_h_
#define _WebDriver_Factories_h_

#include <Core/Core.h>


NAMESPACE_UPP

namespace detail {

struct IFinder_factory : public SharedObjectBase {
	virtual ~IFinder_factory() {}
	virtual Shared<Resource> operator()(const Shared<Resource>& resource) const = 0;
};

struct Element_factory : IFinder_factory {
	virtual Shared<Resource> operator()(const Shared<Resource>& resource) const override;
};

struct Session_factory : IFinder_factory {
	virtual Shared<Resource> operator()(const Shared<Resource>& resource) const override;
};

} // namespace detail

END_UPP_NAMESPACE

#endif