#ifndef _WebDriver_Undetected_h_
#define _WebDriver_Undetected_h_

#include <Core/Core.h>

NAMESPACE_UPP

namespace detail {

String GetFirefoxInstallationPath();
String GetUndetectedFirefoxPath();
String PatchFirefoxBinary();
String GetFirefoxDefaultProfilePath();

} // namespace detail

END_UPP_NAMESPACE

#endif
