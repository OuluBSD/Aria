#include "Aria.h"

NAMESPACE_UPP

const char* WhatsAppScraper::URL = "https://web.whatsapp.com/";

WhatsAppScraper::WhatsAppScraper(AriaNavigator& navigator, SiteManager& sm)
	: navigator(navigator)
	, sm(sm)
{
	site_name = "whatsapp";
}

bool WhatsAppScraper::Navigate() {
	GetAriaLogger("whatsapp").Info("Navigating to WhatsApp Web...");
	try {
		navigator.Navigate(URL);
		navigator.WaitForElement("div[contenteditable='true']", "css selector", 45);
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("whatsapp").Error("Failed to load WhatsApp: " + e);
		return false;
	}
}

bool WhatsAppScraper::Refresh(bool deep) {
	if (!Navigate()) return false;
	
	GetAriaLogger("whatsapp").Info("Refreshing WhatsApp chat list...");
	
	try {
		// Wait for side pane (chat list)
		navigator.WaitForElement("div[id='pane-side']", "css selector", 20);
		
		Vector<ValueMap> chats;
		
		// Find chat items - role="listitem" is common for these
		Vector<Element> chat_elements = navigator.FindElements(By::CssSelector("div[role='listitem']"));
		
		GetAriaLogger("whatsapp").Info(Format("Found %d potential chat elements", chat_elements.GetCount()));
		
		for (int i = 0; i < chat_elements.GetCount(); i++) {
			try {
				Element& e = chat_elements[i];
				ValueMap chat;
				
				// Extract Name from span with title
				try {
					Element name_el = e.FindElement(By::CssSelector("span[title]"));
					chat.Set("name", name_el.GetAttribute("title"));
				} catch(...) {
					chat.Set("name", "Unknown Contact");
				}
				
				// Extract Last Message and Time
				// In WhatsApp Web, these are often sibling spans or in specific nested divs
				try {
					// This is heuristic and might need adjustment as WhatsApp updates
					String full_text = e.GetText();
					Vector<String> parts = Split(full_text, '\n');
					if (parts.GetCount() >= 2) {
						chat.Set("timestamp", parts[1]); // Often the second line is time
						if (parts.GetCount() >= 3) chat.Set("last_message", parts[2]);
					}
				} catch(...) {}
				
				// Online/Typing status (requires more complex DOM inspection)
				chat.Set("status", ""); 
				
				chats.Add(chat);
			} catch (...) {}
		}
		
		GetAriaLogger("whatsapp").Info(Format("Extracted %d valid chats", chats.GetCount()));
		
		ValueArray va;
		for (const auto& c : chats) va.Add(c);
		sm.SetSiteData(site_name, "chats", va);
		
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("whatsapp").Error("Error during WhatsApp refresh: " + e);
		return false;
	}
}

END_UPP_NAMESPACE