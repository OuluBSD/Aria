#include "GoogleMessagesCtrl.h"
#include "AriaHub.h"

NAMESPACE_UPP

GoogleMessagesCtrl::GoogleMessagesCtrl() {
	LayoutId("GoogleMessagesCtrl");
	Add(otp_banner.HSizePos().TopPos(0, 40));
	otp_banner.Add(lblOtp.LeftPos(10, 200).VSizePos());
	otp_banner.Add(btnCopyOtp.RightPos(10, 100).VSizePos(5, 5));
	otp_banner.Add(auto_copy_otp.RightPos(120, 150).VSizePos());
	
	lblOtp.SetLabel("No OTP detected.");
	btnCopyOtp.SetLabel("Copy OTP");
	btnCopyOtp << [=] { OnCopyOtp(); };
	auto_copy_otp.SetLabel("Auto-copy OTP");
	
	Add(lblSearch.LeftPos(10, 60).TopPos(45, 20));
	lblSearch.SetLabel("Search:");
	Add(search.LeftPos(70, 200).TopPos(45, 20));
	
	Add(list.VSizePos(70, 40).HSizePos());
	Add(btnRefresh.BottomPos(5, 30).RightPos(5, 150));
	
	list.AddColumn("Sender", 3);
	list.AddColumn("Snippet", 7);
	list.AddColumn("Time", 2);
	
	btnRefresh.SetLabel("Sync Messages");
	btnRefresh << [=] { Scrape(); };
	
	search << [=] { LoadData(); };
}

void GoogleMessagesCtrl::OnCopyOtp() {
	if (!last_otp.IsEmpty()) {
		WriteClipboardText(last_otp);
		AriaAlert("OTP " + last_otp + " copied to clipboard.");
	}
}

void GoogleMessagesCtrl::Scrape() {
	if (!navigator || !site_manager) return;
	
	GoogleMessagesScraper scraper(*navigator, *site_manager);
	try {
		if (scraper.Refresh()) {
			LoadData();
		} else {
			AriaAlert("Google Messages sync failed. Check login.");
		}
	} catch (const Exc& e) {
		AriaAlert("Error syncing Google Messages: " + e);
	}
}

void GoogleMessagesCtrl::LoadData() {
	if (!site_manager) return;
	list.Clear();
	
	String q = ToLower((String)search.GetData());
	
	Value data = site_manager->GetSiteData("google_messages", "conversations");
	if (data.Is<ValueArray>()) {
		String newest_otp;
		
		for (const Value& conv : (ValueArray)data) {
			String sender = conv["sender"];
			String snippet = conv["snippet"];
			
			// Extract OTP from snippet if it's recent (heuristic: first few conversations)
			if (newest_otp.IsEmpty()) {
				newest_otp = OtpManager::ExtractOtp(snippet);
			}
			
			if (q.IsEmpty() || ToLower(sender).Find(q) >= 0 || ToLower(snippet).Find(q) >= 0) {
				list.Add(sender, snippet, conv["timestamp"]);
				if (conv["unread"]) {
					int row = list.GetCount() - 1;
					list.SetLineColor(row, LtYellow());
				}
			}
		}
		
		if (!newest_otp.IsEmpty() && newest_otp != last_otp) {
			last_otp = newest_otp;
			lblOtp.SetLabel("Latest OTP: " + last_otp);
			if (auto_copy_otp) {
				WriteClipboardText(last_otp);
				// AriaAlert("OTP " + last_otp + " auto-copied.");
			}
		}
	}
}

END_UPP_NAMESPACE
