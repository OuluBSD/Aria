#include "Aria.h"

NAMESPACE_UPP

const char* GoogleMessagesScraper::URL = "https://messages.google.com/web/conversations";

GoogleMessagesScraper::GoogleMessagesScraper(AriaNavigator& navigator, SiteManager& sm)
	: navigator(navigator)
	, sm(sm)
{
	site_name = "google_messages";
}

bool GoogleMessagesScraper::Navigate() {
	GetAriaLogger("google_messages").Info("Navigating to Google Messages...");
	try {
		navigator.Navigate(URL);
		navigator.WaitForElement("mws-conversations-list", "tag name", 30);
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("google_messages").Error("Failed to load Google Messages: " + e);
		return false;
	}
}

bool GoogleMessagesScraper::Refresh(bool deep) {
	if (!Navigate()) return false;
	ValueArray conversations = ScrapeAllConversations();
	sm.SetSiteData(site_name, "conversations", conversations);
	return true;
}

ValueArray GoogleMessagesScraper::ScrapeAllConversations() {
	ValueArray all_data;
	try {
		GetAriaLogger("google_messages").Info("Scraping conversations...");
		Vector<Element> convs = navigator.FindElements(By::TagName("mws-conversation-list-item"));
		GetAriaLogger("google_messages").Info(Format("Found %d conversations", convs.GetCount()));
		
		for (int i = 0; i < convs.GetCount(); i++) {
			try {
				ValueMap conv;
				Element& e = convs[i];
				
				String sender = e.FindElement(By::CssSelector(".name-container")).GetText();
				conv.Set("sender", sender);
				
				try {
					conv.Set("snippet", e.FindElement(By::CssSelector(".snippet-text")).GetText());
				} catch(...) {}
				
				try {
					conv.Set("timestamp", e.FindElement(By::CssSelector(".timestamp")).GetText());
				} catch(...) {}
				
				// Unread check (often a bold class or a specific dot)
				try {
					String cls = e.GetAttribute("class");
					conv.Set("unread", cls.Find("unread") >= 0);
				} catch(...) {}
				
				all_data.Add(conv);
			} catch (...) {}
		}
	} catch (const Exc& e) {
		GetAriaLogger("google_messages").Error("Error scraping: " + e);
	}
	return all_data;
}

ValueArray GoogleMessagesScraper::ExtractVisibleMessages() {
	ValueArray messages;
	try {
		Vector<Element> msg_els = navigator.FindElements(By::TagName("mws-message-wrapper"));
		for (const auto& e : msg_els) {
			try {
				ValueMap msg;
				msg.Set("text", e.FindElement(By::CssSelector(".text-msg")).GetText());
				messages.Add(msg);
			} catch (...) {}
		}
	} catch (...) {}
	return messages;
}

END_UPP_NAMESPACE