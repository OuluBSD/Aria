#include "Aria.h"

NAMESPACE_UPP

SlackScraper::SlackScraper(AriaNavigator& navigator, SiteManager& sm)
	: navigator(navigator), sm(sm), site_name("slack")
{}

bool SlackScraper::SelectSlackTab(int expected_tabs) {
	ValueArray tabs = navigator.ListTabs();
	if (tabs.GetCount() != expected_tabs)
		return false;
	for (const Value& value : tabs) {
		ValueMap tab = value;
		String url = tab["url"];
		if (url.Find("slack.com") >= 0)
			return navigator.GotoTab(tab["handle"]);
	}
	return false;
}

bool SlackScraper::ClickChannel(const String& channel) {
	String wanted = channel;
	wanted = TrimBoth(wanted);
	if (wanted.IsEmpty()) return false;
	String script = Format(R"JS(
		return (function() {
			var wanted = %s;
			var nodes = document.querySelectorAll('[data-qa^="channel_sidebar_name_"]');
			for (var i = 0; i < nodes.length; i++) {
				var n = nodes[i], name = (n.innerText || n.textContent || '').trim();
				var qa = n.getAttribute('data-qa') || '';
				var suffix = qa.replace(/^channel_sidebar_name_/, '').replace(/_/g, ' ');
				if (name.replace(/^#\s*/, '').toLowerCase() === wanted.toLowerCase() ||
					suffix.toLowerCase() === wanted.toLowerCase()) {
					n.click(); return true;
				}
			}
			return false;
		})()
	)JS", AsJSON(wanted));
	return (bool)navigator.Eval(script);
}

void SlackScraper::LoadOlderMessages() {
	int pages = 0;
	String configured = GetEnv("ARIA_SLACK_PAGE_UPS");
	if (!configured.IsEmpty()) pages = max(0, StrInt(configured));
	if (!pages) return;
	for (int i = 0; i < pages; i++) {
		navigator.Eval(R"JS(
			return (function() {
				var list = document.querySelector('#message-list, [data-qa="message_list"], .c-message_list');
				if (!list) return false;
				list.click();
				var target = document.activeElement || list;
				target.dispatchEvent(new KeyboardEvent('keydown', {key:'PageUp', code:'PageUp', keyCode:33, which:33, bubbles:true}));
				target.dispatchEvent(new KeyboardEvent('keyup', {key:'PageUp', code:'PageUp', keyCode:33, which:33, bubbles:true}));
				return true;
			})()
		)JS");
		Sleep(700);
	}
	// Leave the browser at the newest messages after the capture.
	for (int i = 0; i < pages; i++) {
		navigator.Eval(R"JS(
			return (function() {
				var list = document.querySelector('#message-list, [data-qa="message_list"], .c-message_list');
				if (!list) return false;
				list.click();
				var target = document.activeElement || list;
				target.dispatchEvent(new KeyboardEvent('keydown', {key:'PageDown', code:'PageDown', keyCode:34, which:34, bubbles:true}));
				target.dispatchEvent(new KeyboardEvent('keyup', {key:'PageDown', code:'PageDown', keyCode:34, which:34, bubbles:true}));
				return true;
			})()
		)JS");
		Sleep(400);
	}
}

bool SlackScraper::ClickThreadReplies(const String& message_id) {
	Value clicked = navigator.Eval(Format(R"JS(
		return (function() {
			var nodes = document.querySelectorAll('[data-msg-ts="%s"], [data-qa="message_container"]');
			for (var i = 0; i < nodes.length; i++) {
				var n = nodes[i];
				if (n.getAttribute('data-msg-ts') && n.getAttribute('data-msg-ts') !== '%s') continue;
				var b = n.querySelector('[data-qa="reply_bar_count"], [data-qa="thread_replies"], [data-qa="reply_count"], [aria-label*="repl" i]');
				if (!b) {
					var buttons = n.querySelectorAll('button,[role="button"]');
					for (var j = 0; j < buttons.length; j++) {
						var label = (buttons[j].innerText || buttons[j].getAttribute('aria-label') || '').trim();
						if (/\brepl(?:y|ies)\b/i.test(label)) { b = buttons[j]; break; }
					}
				}
				if (b) { (b.closest('button,[role="button"]') || b).click(); return true; }
			}
			return false;
		})()
	)JS", message_id, message_id));
	return (bool)clicked;
}

ValueMap SlackScraper::ExtractThread(const String& channel_id, const String& parent_message_id) {
	Sleep(800);
	Value result = navigator.Eval(Format(R"JS(
		return (function() {
			function text(n) { return n ? (n.innerText || n.textContent || '').trim() : ''; }
			function attr(n, k) { return n ? (n.getAttribute(k) || '') : ''; }
			var panel = document.querySelector('[data-qa="threads_flexpane"], [role="dialog"] .p-view_contents');
			if (!panel) return {id:'', channel_id:'', parent_message_id:'', messages:[]};
			var nodes = panel.querySelectorAll('[data-qa="message_container"]'), messages = [], active = '';
			for (var i = 0; i < nodes.length; i++) {
				var n = nodes[i], ts = attr(n, 'data-msg-ts') || attr(n, 'data-ts');
				if (!active && ts) active = ts;
				var sender = n.querySelector('[data-qa="message_sender_name"], [data-qa="message_sender"]');
				var body = n.querySelector('[data-qa="message-text"], [data-qa="message_text"], [data-qa="message_content"]');
				var time = n.querySelector('[data-qa="timestamp_label"], time[datetime], time');
				if (!ts && !body) continue;
				messages.push({message_id:ts, sender:text(sender), content:text(body), timestamp:attr(time, 'datetime') || attr(time, 'title') || text(time)});
			}
			return {id:%s + ':' + %s, channel_id:%s, parent_message_id:%s, active_parent_id:active, messages:messages};
		})()
	)JS", AsJSON(channel_id), AsJSON(parent_message_id), AsJSON(channel_id), AsJSON(parent_message_id)));
	return result.Is<ValueMap>() ? (ValueMap)result : ValueMap();
}

ValueMap SlackScraper::ExtractChannel(const String& requested) {
	Value result = navigator.Eval(Format(R"JS(
		return (function() {
			function text(n) { return n ? (n.innerText || n.textContent || '').trim() : ''; }
			function attr(n, k) { return n ? (n.getAttribute(k) || '') : ''; }
			var out = {name:'', id:'', messages:[]};
			var heading = document.querySelector('[data-qa="channel_header_name"], [data-qa="channel_name"], [data-qa="channel_header"]');
			out.name = text(heading);
			var channelNode = document.querySelector('[data-qa="channel_sidebar_name_' +
				(String(%s).replace(/[^a-z0-9_-]/gi, '_')) + '"]');
			out.name = out.name || text(channelNode) || %s;
			var nodes = document.querySelectorAll('[data-qa="message_container"], [data-msg-ts]');
			var seen = {};
			for (var i = 0; i < nodes.length; i++) {
				var n = nodes[i], ts = attr(n, 'data-msg-ts') || attr(n, 'data-ts');
				if (!ts || seen[ts]) continue;
				seen[ts] = true;
				var sender = n.querySelector('[data-qa="message_sender_name"], [data-qa="message_sender"]');
				var body = n.querySelector('[data-qa="message_text"], [data-qa="message_content"]');
				var time = n.querySelector('[data-qa="message_timestamp"], time[datetime], time');
				var reply = n.querySelector('[data-qa="reply_bar_count"], [data-qa="thread_replies"], [data-qa="reply_count"], [aria-label*="repl" i]');
				var messageLink = n.querySelector('a[href*="/archives/"][href*="/p"]');
				var attachments = [], media = n.querySelectorAll('[data-qa="message_attachment"] a[href], [data-qa="message_attachment"] img[src], [data-qa="message_file_image_thumbnail"], [data-qa="file_image_thumbnail_img"], [data-qa="download_action"], a[href*="/files-pri/"], a[href*="/files-tmb/"]');
				for (var a = 0; a < media.length; a++) {
					var m = media[a], url = attr(m, 'href') || attr(m, 'src');
					if (!url || attachments.length >= 32) continue;
					var filename = attr(m, 'download') || attr(m, 'data-filename') || attr(m, 'alt') ||
						(url.split('?')[0].split('/').pop() || '');
					var duplicate = -1;
					for (var d = 0; d < attachments.length; d++)
						if (attachments[d].filename && filename && attachments[d].filename === filename) { duplicate = d; break; }
					if (duplicate >= 0) {
						if (url.indexOf('/files-tmb/') >= 0) attachments[duplicate].thumbnail_url = url;
						else if (url.indexOf('/files-pri/') >= 0) attachments[duplicate].download_url = url;
						continue;
					}
					attachments.push({url:url, filename:filename, kind:m.tagName === 'IMG' ? 'image' : 'file',
						label:attr(m, 'aria-label') || attr(m, 'title') || ''});
				}
				var channelNode = n.closest('[data-channel-id], [data-msg-channel], [data-channel]');
				var channel = attr(n, 'data-channel-id') || attr(n, 'data-msg-channel') || attr(n, 'data-channel') ||
					attr(channelNode, 'data-channel-id') || attr(channelNode, 'data-msg-channel') || attr(channelNode, 'data-channel');
				var timestamp = attr(time, 'datetime') || attr(time, 'title') || text(time);
				if (!timestamp && messageLink) {
					var href = attr(messageLink, 'href'), match = href.match(/\/p(\d{10,})/);
					timestamp = match ? match[1] : '';
				}
				out.messages.push({message_id:ts, channel_id:channel, sender:text(sender), content:text(body),
					timestamp:timestamp,
					reply_count:text(reply), attachments:attachments});
			}
			return out;
		})()
	)JS", AsJSON(requested), AsJSON(requested)));
	return result.Is<ValueMap>() ? (ValueMap)result : ValueMap();
}

ValueMap SlackScraper::Scrape(const Vector<String>& requested_channels) {
	ValueMap envelope;
	envelope.Set("version", 1);
	envelope.Set("type", "slack.snapshot");
	envelope.Set("site", "slack");
	envelope.Set("read_only", true);
	envelope.Set("channels", ValueArray());
	envelope.Set("messages", ValueArray());
	envelope.Set("threads", ValueArray());
	envelope.Set("diagnostics", ValueMap()("requested_channels", 0)("captured_channels", 0)("captured_messages", 0));

	Vector<String> channels;
	for (const String& channel : requested_channels)
		channels.Add(channel);
	if (channels.IsEmpty()) {
		String configured = GetEnv("ARIA_SLACK_CHANNELS");
		if (!configured.IsEmpty()) channels = Split(configured, ',');
	}
	if (channels.IsEmpty()) {
		channels.Add("announcements");
		channels.Add("kernels");
		channels.Add("model-inference-optimization");
		channels.Add("model-inference-optimization-support");
	}
	ValueMap diagnostics = envelope["diagnostics"];
	diagnostics.Set("requested_channels", channels.GetCount());
	envelope.Set("diagnostics", diagnostics);

	try {
		int tab_count = navigator.ListTabs().GetCount();
		if (!SelectSlackTab(tab_count))
			throw Exc("No existing Slack tab found; Slack scraping is read-only and will not create or navigate tabs");
		for (const String& raw_channel : channels) {
			String channel = TrimBoth(raw_channel);
			if (channel.IsEmpty()) continue;
			if (!ClickChannel(channel)) {
				GetAriaLogger("slack").Warning("Slack channel is not visible: " + channel);
				continue;
			}
			Sleep(1200);
			LoadOlderMessages();
			ValueMap data = ExtractChannel(channel);
			if (data.IsEmpty()) continue;
			ValueMap channel_data;
			channel_data.Set("id", data["id"]);
			channel_data.Set("name", data["name"]);
			channel_data.Set("requested_name", channel);
			ValueArray messages = data["messages"];
			channel_data.Set("message_count", messages.GetCount());
			ValueArray channel_list = envelope["channels"];
			channel_list.Add(channel_data);
			envelope.Set("channels", channel_list);
			ValueArray all_messages = envelope["messages"];
			ValueArray all_threads = envelope["threads"];
			for (const Value& value : messages) {
				ValueMap message = value;
				message.Set("channel_name", data["name"]);
				all_messages.Add(message);
				String reply_count = message["reply_count"];
				String message_id = message["message_id"];
				// Slack keeps the flexpane mounted while changing threads. Close any
				// previous pane before selecting the next reply to avoid stale data.
				navigator.Eval("var b=document.querySelector('[data-qa=close_flexpane]'); if(b)b.click(); return true;");
				Sleep(300);
				if (!reply_count.IsEmpty() && !message_id.IsEmpty() && ClickThreadReplies(message_id)) {
					ValueMap thread = ExtractThread(String(data["id"]), message_id);
					if (String(thread["active_parent_id"]) == message_id && !((ValueArray)thread["messages"]).IsEmpty())
						all_threads.Add(thread);
					// Close the thread pane before opening the next one.
					navigator.Eval("var b=document.querySelector('[data-qa=close_flexpane]'); if(b)b.click(); return true;");
				}
			}
			envelope.Set("messages", all_messages);
			envelope.Set("threads", all_threads);
		}
		ValueArray all_messages = envelope["messages"];
		diagnostics = envelope["diagnostics"];
		diagnostics.Set("captured_channels", ((ValueArray)envelope["channels"]).GetCount());
		diagnostics.Set("captured_messages", all_messages.GetCount());
		envelope.Set("diagnostics", diagnostics);
		sm.SetSiteData(site_name, "snapshot", envelope);
		sm.SetSiteData(site_name, "messages", all_messages);
	}
	catch (const Exc& e) {
		envelope.Set("error", (String)e);
	}
	return envelope;
}

bool SlackScraper::Refresh(const Vector<String>& channels) {
	ValueMap result = Scrape(channels);
	return IsNull(result["error"]);
}

END_UPP_NAMESPACE
