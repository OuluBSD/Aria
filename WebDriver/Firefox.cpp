#include "WebDriver.h"

NAMESPACE_UPP

void Firefox::ApplyStealthSettings(bool silence_audio) {
	options.SetPreference("dom.webdriver.enabled", false);
	options.SetPreference("media.navigator.enabled", true);
	options.SetPreference("media.navigator.video.enabled", true);
	options.SetPreference("media.ondevicechange.enabled", true);
	options.SetPreference("media.peerconnection.enabled", true);
	options.SetPreference("media.peerconnection.video.enabled", true);
	options.SetPreference("media.peerconnection.identity.enabled", true);
	options.SetPreference("media.peerconnection.simulcast", true);
	options.SetPreference("media.peerconnection.bundle", true);
	options.SetPreference("media.peerconnection.default_iceservers", "[{\"urls\":[\"stun:stun.services.mozilla.com\"]}]");
	options.SetPreference("media.peerconnection.use_document_iceservers", true);
	options.SetPreference("media.peerconnection.turn.disable", false);
	options.SetPreference("media.peerconnection.ice.tcp", true);
	options.SetPreference("media.peerconnection.ice.obfuscate_host_addresses", true);
	options.SetPreference("media.peerconnection.ice.obfuscate_host_addresses_blacklist", "[]");
	options.SetPreference("media.peerconnection.ice.penalize_link_local", false);
	options.SetPreference("media.peerconnection.ice.proxy_only_if_tcp", false);
	options.SetPreference("media.peerconnection.ice.proxy_only", false);
	options.SetPreference("media.peerconnection.ice.loopback", false);
	options.SetPreference("media.peerconnection.ice.websocket", true);
	options.SetPreference("device.sensors.enabled", true);
	options.SetPreference("dom.battery.enabled", true);
	options.SetPreference("geo.enabled", true);
	
	options.SetPreference("browser.safebrowsing.enabled", true);
	options.SetPreference("privacy.trackingprotection.enabled", true);
	options.SetPreference("webgl.disabled", false);
	options.SetPreference("webgl.enable-webgl", true);
	options.SetPreference("webgl.enable-debug-renderer-info", true);
	
	options.SetPreference("browser.webdriver.privileged", false);
	options.SetPreference("devtools.debugger.prompt-connection", false);
	
	options.SetPreference("media.autoplay.default", 0);
	options.SetPreference("media.autoplay.blocking_policy", 0);
	
	options.AddArgument("--disable-infobars");
	options.AddArgument("--disable-extensions");
	options.AddArgument("--no-sandbox");
	
	if (silence_audio)
		options.SetPreference("media.volume_scale", "0.0");
	else
		options.SetPreference("media.volume_scale", "1.0");
		
	// Add more as needed from driver.py
}

END_UPP_NAMESPACE
