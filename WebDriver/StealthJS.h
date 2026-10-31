#ifndef _WebDriver_StealthJS_h_
#define _WebDriver_StealthJS_h_

#include <Core/Core.h>

NAMESPACE_UPP

inline const char* GetStealthJS() {
	return R"(
		try {
			Object.defineProperty(navigator, 'webdriver', {
				get: () => undefined,
				set: () => {},
				configurable: true
			});
		} catch (e) {}
		if (window.chrome) delete window.chrome;
		if (window.cdc_adoQtmx083120120533123) delete window.cdc_adoQtmx083120120533123;
		const mockPlugins = [
			{ name: 'PDF Viewer', filename: 'internal-pdf-viewer', description: 'Portable Document Format' },
			{ name: 'Chrome PDF Viewer', filename: 'mhjfbmdgcfjbbpaeojofohoefgiehjai', description: '' },
			{ name: 'Native Client', filename: 'internal-nacl-plugin', description: '' }
		];
		mockPlugins.length = 3;
		try { Object.defineProperty(navigator, 'plugins', { get: () => mockPlugins }); } catch (e) {}
		try { Object.defineProperty(navigator, 'languages', { get: () => ['en-US', 'en'] }); } catch (e) {}
		try { Object.defineProperty(navigator, 'hardwareConcurrency', { get: () => 4 }); } catch (e) {}
		
		// Mock Page Visibility API and Focus
		try {
			Object.defineProperty(document, 'hidden', { get: () => false, configurable: true });
			Object.defineProperty(document, 'visibilityState', { get: () => 'visible', configurable: true });
			Object.defineProperty(document, 'webkitHidden', { get: () => false, configurable: true });
			document.hasFocus = () => true;
		} catch (e) {}

		// Block blur events and visibility changes that might throttle the page
		const blockEvent = (e) => {
			e.stopImmediatePropagation();
			if (e.type === 'visibilitychange' || e.type === 'webkitvisibilitychange') {
				// Re-dispatch as visible if needed? Usually overriding properties is enough.
			}
		};
		window.addEventListener('visibilitychange', blockEvent, true);
		window.addEventListener('webkitvisibilitychange', blockEvent, true);
		window.addEventListener('blur', blockEvent, true);
		window.addEventListener('focusout', blockEvent, true);

		const getParameter = WebGLRenderingContext.prototype.getParameter;
		WebGLRenderingContext.prototype.getParameter = function(parameter) {
			if (parameter === 37445) return 'Intel Inc.';
			if (parameter === 37446) return 'Intel Iris OpenGL Engine';
			return getParameter(parameter);
		};
		const toString = Function.prototype.toString;
		Function.prototype.toString = function() {
			if (this === Function.prototype.toString) return 'function toString() { [native code] }';
			if (this === navigator.webdriver) return 'function get webdriver() { [native code] }';
			return toString.call(this);
		};
	)";
}

END_UPP_NAMESPACE

#endif
