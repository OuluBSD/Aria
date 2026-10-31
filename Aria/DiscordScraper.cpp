#include "Aria.h"

NAMESPACE_UPP

const char* DiscordScraper::URL = "https://discord.com/app";

DiscordScraper::DiscordScraper(AriaNavigator& navigator, SiteManager& sm)
	: navigator(navigator)
	, sm(sm)
{
	site_name = "discord";
	media_dir = GetEnv("ARIA_CONTEXT_MEDIA_DIR");
	if(media_dir.IsEmpty())
		media_dir = GetHomeDirFile(".aria/context/media");
}

bool DiscordScraper::WaitForStableMessages(int timeout_seconds) {
	int previous = -1, stable = 0;
	for(int i = 0; i < timeout_seconds * 2; i++) {
		Sleep(500);
		try {
			Value count = navigator.Eval("return document.querySelectorAll('li[id^=\\\"chat-messages-\\\"], [data-list-item-id^=\\\"chat-messages-\\\"]').length");
			int n = (int)count;
			if(n > 0 && n == previous) {
				if(++stable >= 2) return true;
			}
			else
				stable = 0;
			previous = n;
		}
		catch(...) {}
	}
	return false;
}

bool DiscordScraper::ClickResource(const String& url) {
	int slash = url.ReverseFind('/');
	if (slash < 0) return false;
	String id = url.Mid(slash + 1);
	int query = id.Find('?');
	if (query >= 0) id = id.Left(query);
	if (id.IsEmpty()) return false;
	for (int attempt = 0; attempt < 24; attempt++) {
	Value clicked = navigator.Eval(Format(R"JS(return (function(){
		var id='%s', nodes=document.querySelectorAll('a[href], [data-list-item-id], [data-channel-id]');
		for(var i=0;i<nodes.length;i++) { var n=nodes[i], h=n.getAttribute('href')||'', d=n.getAttribute('data-list-item-id')||n.getAttribute('data-channel-id')||'';
			if(h.indexOf('/channels/')>=0 && (h.split('/').indexOf(id)>=0 || h.replace(/\/$/,'').split('/').pop()===id) || d.indexOf(id)>=0) { n.removeAttribute('target'); n.click(); return true; } }
		var sc=document.querySelector('#channels[class*="scroller"], #channels [class*="scroller"], [data-list-id="channels"] [class*="scroller"], [class*="sidebarList"] [class*="scroller"]'); if(sc) sc.scrollTop = %d == 0 ? 0 : Math.min(sc.scrollHeight, sc.scrollTop + Math.max(200, sc.clientHeight*.8));
		return false;
	})())JS", id, attempt));
	if ((bool)clicked) return true;
	Sleep(500);
	}
	return false;
}

void DiscordScraper::DownloadAttachments(ValueArray& messages) const {
	int max_bytes = 25 * 1024 * 1024;
	String configured = GetEnv("ARIA_DISCORD_MEDIA_MAX_BYTES");
	if(!configured.IsEmpty()) max_bytes = max(1024, StrInt(configured));
	Index<String> hosts;
	hosts.Add("cdn.discordapp.com");
	hosts.Add("media.discordapp.net");
	if(!DirectoryCreate(media_dir)) return;
	for(int i = 0; i < messages.GetCount(); i++) {
		ValueMap message = messages[i];
		ValueArray attachments = message["attachments"];
		for(int j = 0; j < attachments.GetCount(); j++) {
			ValueMap attachment = attachments[j];
			String source = attachment["url"];
			attachment.Set("local_path", "");
			attachment.Set("download_status", "skipped");
			if(!source.StartsWith("https://")) { attachment.Set("download_status", "blocked_url"); attachments.Set(j, attachment); continue; }
			String authority = source.Mid(8);
			int slash = authority.Find('/');
			String host = slash < 0 ? authority : authority.Left(slash);
			int query = host.Find(':'); if(query >= 0) host = host.Left(query);
			if(hosts.Find(host) < 0) { attachment.Set("download_status", "blocked_host"); attachments.Set(j, attachment); continue; }
			HttpRequest request(source);
			// Do not follow a CDN URL onto an attacker-controlled host.
			request.MaxContentSize(max_bytes).MaxRedirect(0).RequestTimeout(30000);
			String body = request.Execute();
			String type = ToLower(request.GetHeader("Content-Type"));
			int semicolon = type.Find(';');
			if(semicolon >= 0) type = type.Left(semicolon);
			type = TrimBoth(type);
			String filename = attachment["filename"];
			String extension = ToLower(GetFileExt(filename));
			String image_type = type.StartsWith("image/") ? type.Mid(6) : String();
			if(image_type == "jpeg") image_type = "jpg";
			bool safe_image = image_type == "png" || image_type == "jpg" || image_type == "gif" || image_type == "webp" || image_type == "bmp";
			bool known_file = extension == ".pdf" || extension == ".zip" || extension == ".txt" || extension == ".csv" ||
				extension == ".json" || extension == ".doc" || extension == ".docx" || extension == ".xls" || extension == ".xlsx";
			bool allowed = safe_image || type == "application/pdf" || type == "text/plain" ||
				type == "application/zip" || (type == "application/octet-stream" && known_file);
			if(request.GetStatusCode() < 200 || request.GetStatusCode() >= 300 || body.GetLength() <= 0 || body.GetLength() > max_bytes || !allowed) {
				attachment.Set("download_status", body.GetLength() > max_bytes ? "too_large" : (!allowed ? "blocked_type" : "failed"));
				attachments.Set(j, attachment); continue;
			}
			String hash = SHA256String(body);
			String ext = type == "application/pdf" ? ".pdf" :
				type == "application/zip" ? ".zip" : type == "text/plain" ? ".txt" : ".bin";
			if(type.StartsWith("image/")) {
				if(safe_image) ext = "." + image_type;
			}
			String path = AppendFileName(media_dir, hash + ext);
			if(!FileExists(path) && !SaveFile(path, body)) { attachment.Set("download_status", "failed"); attachments.Set(j, attachment); continue; }
			attachment.Set("local_path", path);
			attachment.Set("download_status", "downloaded");
			attachment.Set("sha256", hash);
			attachment.Set("content_type", type);
			attachments.Set(j, attachment);
		}
		message.Set("attachments", attachments);
		messages.Set(i, message);
	}
}

bool DiscordScraper::Navigate() {
	throw Exc("Direct Discord navigation is disabled. Aria may only click inside an existing Discord tab.");
}

bool DiscordScraper::Refresh(bool deep) {
	if (!Navigate()) return false;
	GetAriaLogger("discord").Info("Refreshing Discord data...");

	try {
		// Wait for the message list to be visible
		if(!WaitForStableMessages(20))
			GetAriaLogger("discord").Warning("Discord refresh message count did not stabilize before timeout.");
		
		Vector<ValueMap> messages;
		
		// Basic message extraction logic
		// In Discord, messages are often in 'li' elements inside an 'ol'
		Vector<Element> msg_elements = navigator.WaitForElement("ol[class*='messageLog']", "css selector").FindElements(By::TagName("li"));
		
		GetAriaLogger("discord").Info(Format("Found %d message elements", msg_elements.GetCount()));
		
		for (int i = 0; i < msg_elements.GetCount(); i++) {
			try {
				Element& e = msg_elements[i];
				// Skip if not a real message (e.g. date separators)
				if (e.GetAttribute("class").Find("message") < 0) continue;
				
				ValueMap msg;
				
				// Extract author
				try {
					Element author_el = e.FindElement(By::CssSelector("span[class*='username']"));
					msg.Set("author", author_el.GetText());
				} catch (...) { msg.Set("author", "Unknown"); }
				
				// Extract content
				try {
					Element content_el = e.FindElement(By::CssSelector("div[class*='messageContent']"));
					msg.Set("content", content_el.GetText());
				} catch (...) { msg.Set("content", ""); }
				
				// Extract timestamp
				try {
					Element time_el = e.FindElement(By::CssSelector("time"));
					msg.Set("timestamp", time_el.GetAttribute("datetime"));
				} catch (...) { msg.Set("timestamp", ""); }
				
				if (!IsNull(msg["content"]))
					messages.Add(msg);
					
			} catch (...) {}
		}
		
		GetAriaLogger("discord").Info(Format("Extracted %d valid messages", messages.GetCount()));
		
		// Store in site manager
		ValueArray va;
		for (const auto& m : messages) va.Add(m);
		sm.SetSiteData(site_name, "messages", va);
		
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("discord").Error("Error during Discord refresh: " + e);
		return false;
	}
}

ValueMap DiscordScraper::ScrapeChannel(const String& url, bool no_theme) {
	ValueMap envelope;
	int tab_count = navigator.ListTabs().GetCount();
	bool created_discord_tab = false;
	int guild_start = url.Find("/channels/"); int guild_end = guild_start < 0 ? -1 : url.Find('/', guild_start + 10); String guild_marker = guild_start >= 0 ? url.Mid(guild_start, guild_end < 0 ? url.GetCount() - guild_start : guild_end - guild_start) : String();
	{
		ValueArray tabs = navigator.ListTabs();
		bool found = false; for (const Value& tab_value : tabs) { ValueMap tab = tab_value; if (!guild_marker.IsEmpty() && String(tab["url"]).Find(guild_marker) >= 0) { found = true; break; } }
		if (!found) { String root_url = "https://discord.com" + guild_marker; navigator.NewTab(root_url); created_discord_tab = true; Sleep(5000); if (navigator.ListTabs().GetCount() != tab_count + 1) throw Exc("Discord tab creation assertion failed"); tab_count++; ValueArray after = navigator.ListTabs(); for (const Value& tv : after) { ValueMap tm = tv; if (String(tm["url"]).Find(guild_marker) >= 0) { navigator.GotoTab(tm["handle"]); break; } } }
	}
	auto assert_tabs = [&] { if (navigator.ListTabs().GetCount() != tab_count) throw Exc("Discord crawl changed Firefox tab count"); };
	envelope.Set("version", 2);
	envelope.Set("type", "discord.channel");
	envelope.Set("url", url);
	envelope.Set("channel", ValueMap()
		("id", "")
		("name", "")
		("guild_id", ""));
	envelope.Set("messages", ValueArray());
	envelope.Set("sections", ValueArray());
	envelope.Set("categories", ValueArray());
	envelope.Set("thread_panel", ValueArray());
	envelope.Set("message_starters", ValueArray());
	envelope.Set("media", ValueMap()("directory", media_dir)("max_bytes", 25 * 1024 * 1024));
	String previous_url, discord_handle;
	try {
		ValueArray tabs = navigator.ListTabs();
		for (const Value& tab_value : tabs) { ValueMap tab = tab_value; String tab_url = tab["url"]; if (tab_url.Find("discord.com/channels/") >= 0) { discord_handle = tab["handle"]; break; } }
		if (!discord_handle.IsEmpty()) navigator.GotoTab(discord_handle);
		previous_url = navigator.GetCurrentURL();
	} catch (...) {}

	try {
		GetAriaLogger("discord").Info("Navigating to Discord channel...");
		assert_tabs();
		String selected_url = navigator.GetCurrentURL();
		if (selected_url != url && !ClickResource(url)) throw Exc("Discord route is not reachable by clicking an existing channel link");
		Sleep(5000);
		assert_tabs();
		// Discord can keep the app mount in an already-rendered tab while the
		// channel route is loading.  Waiting on the mount element alone therefore
		// produces false timeouts; let the SPA settle and inspect the DOM below.
		if(!WaitForStableMessages())
			GetAriaLogger("discord").Warning("Discord channel message count did not stabilize before timeout.");

		Value result = navigator.Eval(R"JS(
			return (function() {
				var path = window.location.pathname.split('/');
				var messages = [];
				var nodes = document.querySelectorAll('li[id^="chat-messages-"]');
				function text(n) { return n ? (n.innerText || n.textContent || '').trim() : ''; }
				function attr(n, name) { return n ? (n.getAttribute(name) || '') : ''; }
				for (var nodeIndex = 0; nodeIndex < nodes.length; nodeIndex++) {
					var node = nodes[nodeIndex];
					var messageId = attr(node, 'id').replace(/^chat-messages-/, '');
					if (!messageId) continue;
					var duplicate = false;
					for (var seen = 0; seen < messages.length; seen++)
						if (messages[seen].message_id === messageId) duplicate = true;
					if (duplicate) continue;
					var authorNode = node.querySelector('[id^="message-username-"], [class*="username"]');
					var timeNode = node.querySelector('time[datetime], time');
					var contentNode = node.querySelector('[id^="message-content-"], [class*="markup"]');
					var author = text(authorNode);
					var content = text(contentNode);
					var timestamp = attr(timeNode, 'datetime') || attr(timeNode, 'title');
					var attachments = [], attachmentNodes = node.querySelectorAll('a[href], img[src], [data-media-url]');
					for (var ai = 0; ai < attachmentNodes.length; ai++) {
						var an = attachmentNodes[ai], au = attr(an, 'href') || attr(an, 'src') || attr(an, 'data-media-url');
						if (!/^https:\/\/(cdn\.discordapp\.com|media\.discordapp\.net)\//i.test(au)) continue;
						var fn = attr(an, 'download') || (au.split('?')[0].split('/').pop() || 'attachment');
						attachments.push({url:au, filename:fn, kind:/\.(png|jpe?g|gif|webp|bmp|svg)$/i.test(fn) || an.tagName === 'IMG' ? 'image' : 'file'});
					}
					var reactions = [], reactionNodes = node.querySelectorAll('[class*="reaction"]');
					for (var r = 0; r < reactionNodes.length; r++) {
						var rn = reactionNodes[r], label = text(rn.querySelector('[class*="emoji"], img, [aria-label]')) || rn.getAttribute('aria-label') || '';
						var countNode = rn.querySelector('[class*="reactionCount"], [class*="count"]'), count = parseInt(text(countNode), 10);
						if (!label) continue;
						reactions.push({emoji: label.replace(/\s+/g, ' ').trim(), count: isNaN(count) ? 1 : count, me: !!rn.querySelector('[class*="reactionMe"], [aria-pressed="true"]') || /selected|you reacted/i.test(rn.getAttribute('aria-label') || '')});
					}
					if (!content && !author) continue;
					messages.push({
						message_id: messageId,
						channel_id: path.length >= 2 ? path[path.length - 1] : '',
						guild_id: path.length >= 2 ? path[path.length - 2] : '',
						sender_id: attr(node, 'data-author-id') || attr(authorNode, 'data-user-id'),
						sender_name: author,
						timestamp: timestamp,
						content: content,
						reactions: reactions, attachments: attachments
					});
				}
				function display(n) { var values = n ? [text(n), attr(n,'aria-label'), attr(n,'data-name'), attr(n,'title')] : []; for(var i=0;i<values.length;i++) if(values[i] && !/^\d{5,}$/.test(values[i])) return values[i]; return ''; }
				var channelNode = document.querySelector('[data-channel-id] h1, [data-channel-id] [class*="title"], header h1, [role="heading"]');
				var panel = document.querySelector('[class*="channelName"], [class*="threadName"], [class*="threadSidebar"] [class*="title"]');
				var channelTitle = display(panel) || display(channelNode) || display(document.querySelector('[data-channel-id]')) || display(document.querySelector('[aria-label*="channel" i]'));
				var sectionNode = channelNode && channelNode.closest('[class*="container"], [role="group"]');
				var forum_posts = [];
				var forumCards = document.querySelectorAll('li[data-item-role="item"]');
				for (var ci = 0; ci < forumCards.length; ci++) {
					var cardRoot = forumCards[ci], card = cardRoot.querySelector('[data-list-item-id], [data-item-id]') || cardRoot;
					var cardId = attr(card, 'data-item-id') || attr(card, 'data-list-item-id') || ('card-' + ci);
					var titleNode = cardRoot.querySelector('h3.postTitleText, h3[class*="postTitle"], h3');
					var previewNode = cardRoot.querySelector('[class*="postPreview"], [class*="preview"], [class*="messageContent"], [class*="markup"]');
					var tagNodes = cardRoot.querySelectorAll('[class*="tag"], [data-tag-name], [aria-label*="tag" i]');
					var tags = [], seenTags = {};
					for (var tg = 0; tg < tagNodes.length; tg++) { var tv = text(tagNodes[tg]) || attr(tagNodes[tg], 'data-tag-name') || attr(tagNodes[tg], 'aria-label'); if (tv && !seenTags[tv]) { seenTags[tv] = true; tags.push(tv); } }
					var reactionNode = cardRoot.querySelector('[class*="reaction"], [class*="emoji"], [data-reaction], [aria-label*="reaction" i]');
					var commentNode = cardRoot.querySelector('[class*="comment"], [class*="reply"], [aria-label*="comment" i], [aria-label*="reply" i]');
					var dateNode = cardRoot.querySelector('time, [datetime], [class*="timestamp"], [class*="date"], [class*="age"]');
					var post = {id:cardId, card_index:ci, parent_channel_id:path[path.length-1], tag:tags.join(', '), tags:tags,
						title:text(titleNode), preview:text(previewNode), reaction:text(reactionNode), comment_count:text(commentNode), age:text(dateNode),
						name:text(titleNode), display_name:text(titleNode), icon_kind:'forum-post', messages:[]};
					if (!post.title) post.title = 'Forum post ' + cardId;
					post.name = post.title; post.display_name = post.title;
					forum_posts.push(post);
				}
				return {
					channel_id: path.length >= 2 ? path[path.length - 1] : '',
					guild_id: path.length >= 2 ? path[path.length - 2] : '',
					channel_name: channelTitle,
					icon_kind: path.length >= 4 ? 'thread' : (/stage/i.test(channelTitle) ? 'stage' : (/voice/i.test(channelTitle) ? 'voice' : 'text')),
					section: {name: display(sectionNode), kind: sectionNode ? 'section' : ''},
						messages: messages,
						channel: (function(){ var n=document.querySelector('[data-channel-id]'); return {id: n ? attr(n,'data-channel-id') : '', name: text(channelNode)}; })(), forum_posts: forum_posts
				};
			})()
		)JS");

		if (result.Is<ValueMap>()) {
			ValueMap data = result;
			envelope.Set("channel", ValueMap()
				("id", data["channel_id"])
				("name", data["channel_name"])
				("guild_id", data["guild_id"]));
			ValueArray channel_messages = data["messages"];
			DownloadAttachments(channel_messages);
			envelope.Set("messages", channel_messages);
			ValueArray forum_posts = data["forum_posts"];
			ValueArray captured_threads;
			for (int forum_index = 0; forum_index < forum_posts.GetCount(); forum_index++) {
				Value post_value = forum_posts[forum_index];
				ValueMap post = post_value;
				try {
					assert_tabs();
					Value clicked = navigator.Eval(Format(R"JS(return (function(){
						var cards=document.querySelectorAll('li[data-item-role="item"]');
						var card=cards[%d]; if(!card) return false;
						var target=card.querySelector('h3.postTitleText, h3[class*="postTitle"], h3, [role="link"], a') || card;
						target.removeAttribute('target'); card.removeAttribute('target'); target.click(); return true;
					})())JS", forum_index));
					if (!(bool)clicked) throw Exc("Forum card is no longer present");
					Sleep(5000);
					assert_tabs();
					if (!WaitForStableMessages(90))
						GetAriaLogger("discord").Warning("Forum thread messages did not stabilize: " + (String)post["title"]);
					Value thread_value = navigator.Eval(R"JS(return (function(){
						function text(n){return n?(n.innerText||n.textContent||'').trim():'';}
						function attr(n,k){return n?(n.getAttribute(k)||''):'';}
						var out=[], nodes=document.querySelectorAll('li[id^="chat-messages-"], [data-list-item-id^="chat-messages-"]');
						for(var i=0;i<nodes.length;i++){var n=nodes[i], id=attr(n,'id').replace(/^chat-messages-/,'')||attr(n,'data-list-item-id').replace(/^chat-messages-/,'');
							var poster=n.querySelector('[id^="message-username-"], [class*="username"], [class*="header"] [class*="name"]');
							var avatar=n.querySelector('img[src], [class*="avatar"] img, [class*="avatar"]');
							var content=n.querySelector('[id^="message-content-"], [class*="markup"], [class*="messageContent"]');
							var tm=n.querySelector('time[datetime], time'); if(!id || (!text(poster)&&!text(content))) continue;
							var attachments=[], as=n.querySelectorAll('a[href], img[src], [data-media-url]');
							for(var ai=0;ai<as.length;ai++){var an=as[ai], au=attr(an,'href')||attr(an,'src')||attr(an,'data-media-url'); if(!/^https:\/\/(cdn\.discordapp\.com|media\.discordapp\.net)\//i.test(au)) continue; attachments.push({url:au,filename:attr(an,'download')||(au.split('?')[0].split('/').pop()||'attachment'),kind:an.tagName==='IMG'?'image':'file'});}
							out.push({message_id:id, poster:text(poster), avatar:attr(avatar,'src')||text(avatar), sender_name:text(poster), content:text(content), timestamp:attr(tm,'datetime')||attr(tm,'title'), attachments:attachments}); }
						return out;
					})())JS");
					ValueArray thread_messages = thread_value.Is<ValueArray>() ? (ValueArray)thread_value : ValueArray();
					DownloadAttachments(thread_messages);
					for (const Value& message : thread_messages) { ValueMap tm = message; tm.Set("thread_id", post["id"]); tm.Set("thread_name", post["title"]); tm.Set("resource_type", "forum-post"); channel_messages.Add(tm); }
					post.Set("messages", thread_messages);
					captured_threads.Add(post);
					// Return to the forum channel through its visible sidebar link, never by URL navigation.
					if (!ClickResource(url)) throw Exc("Forum channel link is no longer reachable");
					Sleep(5000);
					assert_tabs();
					WaitForStableMessages(30);
				} catch (...) { GetAriaLogger("discord").Warning("Forum post capture failed: " + (String)post["url"]); }
			}
			envelope.Set("messages", channel_messages);
			envelope.Set("threads", captured_threads);
			ValueMap channel = envelope["channel"];
			ValueMap discovered_channel = data["channel"];
			if (!discovered_channel.IsEmpty()) {
				channel.Set("id", discovered_channel["id"]);
				channel.Set("name", discovered_channel["name"]);
			}
			envelope.Set("channel", channel);
			if (!no_theme) {
				Value theme = navigator.Eval(R"JS((function(){
					function safe(v){v=(v||'').trim(); return !v || (v.length < 160 && !/[<>]|javascript:|url\s*\(|https?:\/\//i.test(v));}
					var root=getComputedStyle(document.documentElement), body=getComputedStyle(document.body), out={colors:{}, fonts:{}, vars:{}};
					['--background-primary','--background-secondary','--background-tertiary','--text-normal','--text-muted','--brand-experiment'].forEach(function(k){var v=root.getPropertyValue(k).trim(); if(safe(v)) out.vars[k]=v;});
					[['background',body.backgroundColor],['color',body.color]].forEach(function(p){if(safe(p[1]))out.colors[p[0]]=p[1];});
					if(safe(body.fontFamily))out.fonts.family=body.fontFamily;
					if(safe(body.fontSize))out.fonts.size=body.fontSize;
					return out;
				})())JS");
				envelope.Set("theme_profile", theme);
			}
			ValueArray messages = channel_messages;
			ValueMap section = data["section"];
			ValueArray sections;
			if(!section.IsEmpty()) sections.Add(section);
			envelope.Set("sections", sections);
			channel.Set("icon_kind", data["icon_kind"]);
			envelope.Set("channel", channel);
			sm.SetSiteData(site_name, "messages", messages);
			GetAriaLogger("discord").Info(Format("Extracted %d Discord messages", messages.GetCount()));
			// Discord navigation is click-only; leave the authenticated tab in its current SPA state.
		}
	} catch (const Exc& e) {
		GetAriaLogger("discord").Error("Error scraping Discord channel: " + e);
		envelope.Set("error", (String)e);
		// Do not restore by changing the address bar.
	}
	return envelope;
}

ValueMap DiscordScraper::ScrapeGuild(const String& url, bool no_theme, bool incremental) {
	ValueMap envelope;
	int tab_count = navigator.ListTabs().GetCount();
	bool created_discord_tab = false;
	int guild_start = url.Find("/channels/"); int guild_end = guild_start < 0 ? -1 : url.Find('/', guild_start + 10); String guild_marker = guild_start >= 0 ? url.Mid(guild_start, guild_end < 0 ? url.GetCount() - guild_start : guild_end - guild_start) : String();
	{
		ValueArray tabs = navigator.ListTabs();
		bool found = false; for (const Value& tab_value : tabs) { ValueMap tab = tab_value; if (!guild_marker.IsEmpty() && String(tab["url"]).Find(guild_marker) >= 0) { found = true; break; } }
		if (!found) { navigator.NewTab(url); created_discord_tab = true; Sleep(5000); if (navigator.ListTabs().GetCount() != tab_count + 1) throw Exc("Discord tab creation assertion failed"); tab_count++; ValueArray after = navigator.ListTabs(); for (const Value& tv : after) { ValueMap tm = tv; if (String(tm["url"]).Find("discord.com/") >= 0) { navigator.GotoTab(tm["handle"]); break; } } }
	}
	auto assert_tabs = [&] { if (navigator.ListTabs().GetCount() != tab_count) throw Exc("Discord crawl changed Firefox tab count"); };
	envelope.Set("version", 2);
	envelope.Set("type", "discord.guild");
	envelope.Set("url", url);
	envelope.Set("guild", ValueMap()("id", "")("name", "")("url", url));
	envelope.Set("channels", ValueArray());
	envelope.Set("sections", ValueArray());
	envelope.Set("categories", ValueArray());
	envelope.Set("threads", ValueArray());
	envelope.Set("messages", ValueArray());
	envelope.Set("cursors", ValueArray());
	envelope.Set("unread", ValueArray());
	envelope.Set("diagnostics", ValueMap()("max_messages", 500)("max_scroll_pages", 20)
		("discovered_channels", 0)("discovered_threads", 0)("captured_messages", 0));
	envelope.Set("partial_failures", ValueArray());
	envelope.Set("media", ValueMap()("directory", media_dir)("max_bytes", 25 * 1024 * 1024));
	String previous_url, discord_handle;
	try {
		ValueArray tabs = navigator.ListTabs();
		for (const Value& tab_value : tabs) {
			ValueMap tab = tab_value;
			String tab_url = tab["url"];
			if (tab_url.Find("discord.com/channels/") >= 0) { discord_handle = tab["handle"]; break; }
		}
		if (!discord_handle.IsEmpty()) navigator.GotoTab(discord_handle);
		previous_url = navigator.GetCurrentURL();
	}
	catch (...) {}
	if (!discord_handle.IsEmpty())
		GetAriaLogger("discord").Info("Pinned crawl to existing Discord tab: " + discord_handle);

	try {
		GetAriaLogger("discord").Info("Navigating to Discord guild channel...");
		assert_tabs();
		String current_url = navigator.GetCurrentURL();
		if (!created_discord_tab && current_url.Find("discord.com/channels/") < 0)
			throw Exc("Discord guild crawl requires an already-open Discord guild tab");
		assert_tabs();
		for (int wait = 0; wait < 90; wait++) {
			Sleep(1000);
			try {
				Value ready = navigator.Eval("return document.querySelectorAll('a[href*=\\\"/channels/\\\"]').length");
				if ((int)ready > 0) break;
			} catch (...) {}
		}

		// Discord's React DOM exposes route links for accessible channels and threads.
		// Keep discovery in the authenticated DOM, rather than calling Discord APIs.
		Value discovered = navigator.Eval(R"JS(
			return (function() {
				var out = {guild: {id:'', name:''}, channels:[], sections:[], categories:[], threads:[], thread_panel:[], message_starters:[], current:''};
				var path = location.pathname.split('/');
				if (path[1] === 'channels' && path.length > 2) out.guild.id = path[2] || '';
				function text(n) { return n ? (n.innerText || n.textContent || '').trim() : ''; }
				function add(a) {
					var link = a.matches && a.matches('a[href]') ? a : a.querySelector('a[href]');
					var href = (link ? link.getAttribute('href') : '') || a.getAttribute('href') || '';
					var m = href.match(/^\/channels\/([^/]+)\/([^/?#]+)(?:\/([^/?#]+))?/);
					var dataId = a.getAttribute('data-list-item-id') || a.getAttribute('data-channel-id') || '';
					if (!m && /^channels___\d+$/.test(dataId)) m = ['', out.guild.id, dataId.replace(/^channels___/, ''), ''];
					if (!m || m[1] === '@me' || (out.guild.id && m[1] !== out.guild.id)) return;
					if (!href) href = '/channels/' + m[1] + '/' + m[2] + (m[3] ? '/' + m[3] : '');
					var labelNode = a.querySelector('h3 [class*="overflow"], h3 [class*="name"], [class*="channelName"], [class*="overflow"]');
					var label = text(labelNode) || text(a) || a.getAttribute('aria-label') || a.getAttribute('data-name') || a.getAttribute('title') || '';
					if(/^\d{5,}$/.test(label)) label = a.getAttribute('aria-label') || a.getAttribute('data-name') || a.getAttribute('title') || 'Thread';
					var lower = label.toLowerCase();
					if (!label || lower.indexOf('voice') >= 0 || lower.indexOf('stage') >= 0 || lower.indexOf('archived') >= 0) return;
					var unreadNode = a.querySelector('[class*="unread"], [aria-label*="unread" i]');
					var badge = a.querySelector('[class*="numberBadge"], [class*="unreadBadge"], [aria-label*="unread" i]');
					var badgeCount = parseInt(text(badge), 10);
					var parentNode = a.closest('[data-list-item-id]');
					var icon = lower.indexOf('announcement') >= 0 ? 'hash' : (lower.indexOf('server-rules') >= 0 ? 'book' : (lower.indexOf('for-hire') >= 0 ? 'speech-bubbles' : (m[3] ? 'thread' : 'text')));
					var item = a.closest('li.containerDefault, li[class*="containerDefault"]'), category = item;
					while (category && !category.querySelector('h3')) category = category.previousElementSibling;
					var section_name = category ? text(category.querySelector('h3 [class*="overflow"], h3 [class*="name"], h3')) : '';
					var section_id = category ? (category.getAttribute('data-list-item-id') || '') : '';
					var node = {id:m[3] || m[2], name:label, display_name:label, icon_kind:icon, section:{id:section_id, name:section_name, kind:section_name ? 'category' : ''}, section_id:section_id, url:location.origin + href,
						parent_id:a.getAttribute('data-parent-id') || (parentNode ? parentNode.getAttribute('data-list-item-id') : ''),
						unread:!!unreadNode, unread_count:isNaN(badgeCount) ? (!!unreadNode ? 1 : 0) : badgeCount,
						kind:m[3] ? 'thread' : 'text'};
					var isThread = !!m[3] || !!(a.closest('[class*="thread"], [data-list-item-id*="thread"]')) ||
						!!a.getAttribute('data-thread-id') || lower.indexOf('thread') >= 0;
					var list = isThread ? out.threads : out.channels;
					for (var i=0; i<list.length; i++) if (list[i].id === node.id) return;
					list.push(node);
				}
				var links = document.querySelectorAll('a[href^="/channels/"], nav#channels [data-list-item-id^="channels___"], nav#channels [role="link"]');
				for (var i=0; i<links.length; i++) add(links[i]);
				var groups = document.querySelectorAll('nav#channels ul.content > li.containerDefault, nav#channels ul.content > li[class*="containerDefault"], [class*="categoryContainer"], [role="group"]');
				for (var g=0; g<groups.length; g++) { var gn=groups[g], name=text(gn.querySelector('[class*="name"], h3')); if(name) out.categories.push({id:gn.getAttribute('data-list-item-id')||'', name:name, kind:'category'}); }
				var sections = document.querySelectorAll('[class*="privateChannelsHeaderContainer"], [class*="section"]');
				for (var s=0; s<sections.length; s++) { var sn=sections[s], sectionName=text(sn); if(sectionName && sectionName.length < 100) out.sections.push({name:sectionName, kind:'section'}); }
				var panels=document.querySelectorAll('[class*="threadSidebar"], [class*="threadPanel"], [aria-label*="thread" i]');
				for(var p=0;p<panels.length;p++){var pn=panels[p], pl=text(pn.querySelector('h2,[class*="header"], [class*="title"]'))||text(pn); if(pl&&pl.length<200) out.thread_panel.push({name:pl,kind:'thread_panel'});}
				var starters=document.querySelectorAll('[class*="messageStarter"], [class*="newMemberActionBar"], textarea[placeholder]');
				for(var q=0;q<starters.length;q++){var st=starters[q], ph=st.getAttribute('placeholder')||text(st); if(ph&&ph.length<200) out.message_starters.push({label:ph,kind:'message_starter'});}
				var title = document.querySelector('[class*="guildName"], [class*="headerText"], header h1');
				out.guild.name = text(title) || document.title.replace(/\\s*[-|].*$/, '').trim();
				out.current = location.href;
				return out;
			})()
		)JS");
		if (!discovered.Is<ValueMap>()) throw Exc("Discord guild DOM discovery returned no data");
		ValueMap found = discovered;
		ValueMap guild = found["guild"];
		envelope.Set("guild", guild);
		Vector<String> urls;
		Vector<bool> is_thread;
		ValueArray channels = found["channels"];
		ValueArray sections = found["sections"];
		ValueArray categories = found["categories"];
		ValueArray threads = found["threads"];
		Index<String> known_urls;
		envelope.Set("channels", channels);
		envelope.Set("sections", sections);
		envelope.Set("categories", categories);
		envelope.Set("thread_panel", found["thread_panel"]);
		envelope.Set("message_starters", found["message_starters"]);
		envelope.Set("threads", threads);
		for (const Value& v : channels) { ValueMap m = v; String resource_url = m["url"]; if (known_urls.Find(resource_url) < 0) { known_urls.Add(resource_url); urls.Add(resource_url); is_thread.Add(false); } }
		for (const Value& v : threads) { ValueMap m = v; String resource_url = m["url"]; if (known_urls.Find(resource_url) < 0) { known_urls.Add(resource_url); urls.Add(resource_url); is_thread.Add(true); } }

		ValueArray all_messages, cursors, unread, failures;
		int captured = 0;
		int resource_limit = incremental ? urls.GetCount() : urls.GetCount();
		for (int resource = 0; resource < resource_limit; resource++) {
			String resource_url = urls[resource];
			try {
				int sidebar_unread = 0;
				ValueArray resource_list = is_thread[resource] ? threads : channels;
				for (const Value& rv : resource_list) { ValueMap rm = rv; if ((String)rm["url"] == resource_url) { sidebar_unread = (int)rm["unread_count"]; break; } }
				assert_tabs();
				if (!ClickResource(resource_url)) throw Exc("Discord resource is not reachable by clicking an existing link");
				Sleep(5000);
				assert_tabs();
				if(!WaitForStableMessages())
					GetAriaLogger("discord").Warning("Discord route message count did not stabilize: " + resource_url);
				// Threads are often absent from the initial sidebar DOM and become
				// discoverable only after their parent channel is opened.
				Value discovered_threads = navigator.Eval(R"JS(
					return (function() {
						var out = [], links = document.querySelectorAll('a[href^="/channels/"]');
						for (var i = 0; i < links.length; i++) {
							var a = links[i], href = a.getAttribute('href') || '';
							var m = href.match(/^\/channels\/([^/]+)\/([^/?#]+)\/([^/?#]+)/);
							if (!m) continue;
					var label = (a.innerText || a.textContent || a.getAttribute('aria-label') || a.getAttribute('data-name') || a.getAttribute('title') || '').trim();
					if(/^\d{5,}$/.test(label)) label = '';
							if (!label || /voice|stage|archived/i.test(label)) continue;
							out.push({id:m[3], parent_id:m[2], name:label, url:location.origin + href});
						}
						return out;
					})()
				)JS");
				if (discovered_threads.Is<ValueArray>()) {
					for (const Value& thread_value : (ValueArray)discovered_threads) {
						ValueMap thread = thread_value;
						String thread_url = thread["url"];
						if (thread_url.IsEmpty() || known_urls.Find(thread_url) >= 0) continue;
						known_urls.Add(thread_url);
						threads.Add(thread);
						urls.Add(thread_url);
						is_thread.Add(true);
					}
				}
				ValueMap seen;
				String oldest = "";
				int pages = 0;
				int unread_count = sidebar_unread;
				int resource_captured = 0;
				for (; pages < (incremental ? 1 : 20) && resource_captured < (incremental ? 100 : 500); pages++) {
					Value page = navigator.Eval(R"JS(
						return (function() {
							var path=location.pathname.split('/'), out={messages:[], cursor:'', unread:0, title:'', forum_posts:[]};
							function text(n){return n?(n.innerText||n.textContent||'').trim():'';}
							function attr(n,k){return n?(n.getAttribute(k)||''):'';}
							var nodes=document.querySelectorAll('li[id^="chat-messages-"], [data-list-item-id^="chat-messages-"]');
							for(var i=0;i<nodes.length;i++) { var n=nodes[i], id=attr(n,'id').replace(/^chat-messages-/,'') || attr(n,'data-list-item-id').replace(/^chat-messages-/,''); if(!id) continue;
							var a=n.querySelector('[id^="message-username-"], [class*="username"]'), t=n.querySelector('time[datetime],time'), c=n.querySelector('[id^="message-content-"], [class*="markup"]');
							var content=text(c), author=text(a); if(!content && !author) continue;
							var attachments=[], as=n.querySelectorAll('a[href],img[src],[data-media-url]'); for(var ai=0;ai<as.length;ai++){var an=as[ai], au=attr(an,'href')||attr(an,'src')||attr(an,'data-media-url'); if(!/^https:\/\/(cdn\.discordapp\.com|media\.discordapp\.net)\//i.test(au)) continue; var fn=attr(an,'download')||(au.split('?')[0].split('/').pop()||'attachment'); attachments.push({url:au,filename:fn,kind:/\.(png|jpe?g|gif|webp|bmp|svg)$/i.test(fn)||an.tagName==='IMG'?'image':'file'});}
							var reactions=[], rs=n.querySelectorAll('[class*="reaction"]'); for(var r=0;r<rs.length;r++){var rn=rs[r], emoji=text(rn.querySelector('[class*="emoji"],img,[aria-label]'))||attr(rn,'aria-label'), cn=rn.querySelector('[class*="reactionCount"],[class*="count"]'), count=parseInt(text(cn),10); if(emoji) reactions.push({emoji:emoji,count:isNaN(count)?1:count,me:!!rn.querySelector('[class*="reactionMe"],[aria-pressed="true"]')||/selected|you reacted/i.test(attr(rn,'aria-label'))});}
							out.messages.push({message_id:id, guild_id:path[2]||'', channel_id:path[3]||'', sender_id:attr(n,'data-author-id')||attr(a,'data-user-id'), sender_name:author, timestamp:attr(t,'datetime')||attr(t,'title'), content:content, reactions:reactions, attachments:attachments});
							}
							var first=nodes.length?nodes[0]:null; out.cursor=attr(first,'id').replace(/^chat-messages-/,'');
							var active=document.querySelector('main [aria-current="page"], main [class*="selected"]'); out.unread=active ? (function(){var b=active.querySelector('[class*="numberBadge"],[class*="unreadBadge"]'); var c=parseInt(text(b),10); return isNaN(c)?0:c;})() : 0;
							var heading=document.querySelector('header h1,[class*="channelName"],[class*="title"]'); out.title=text(heading);
							var cards=document.querySelectorAll('li[data-item-role="item"]'); for(var fi=0;fi<cards.length;fi++){var card=cards[fi], title=text(card.querySelector('h3.postTitleText,h3[class*="postTitle"],h3')), link=card.querySelector('a[href*="/channels/"]'), href=attr(link,'href'); if(!title||!href) continue; var id=href.split('/').pop(), preview=text(card.querySelector('[class*="postPreview"],[class*="preview"],[class*="markup"]')); out.forum_posts.push({id:id,thread_id:id,parent_channel_id:path[3]||'',title:title,name:title,display_name:title,preview:preview,url:location.origin+href,icon_kind:'forum-post',messages:[]});}
							return out;
						})()
					)JS");
					if (!page.Is<ValueMap>()) break;
					ValueMap pm = page;
					ValueArray forum_posts = pm["forum_posts"];
					for (const Value& fpv : forum_posts) { ValueMap fp = fpv; bool exists = false; for (const Value& tv : threads) { ValueMap t = tv; if ((String)t["id"] == (String)fp["id"] || (String)t["url"] == (String)fp["url"]) { exists = true; break; } } if (!exists) threads.Add(fp); }
					String route_title = pm["title"];
					if (is_thread[resource] && !route_title.IsEmpty()) {
						for (int ti = 0; ti < threads.GetCount(); ti++) { ValueMap thread = threads[ti]; if ((String)thread["url"] == resource_url) { thread.Set("name", route_title); thread.Set("display_name", route_title); threads.Set(ti, thread); break; } }
					}
					unread_count = (int)pm["unread"];
					ValueArray msgs = pm["messages"];
					ValueArray route_messages = msgs;
					DownloadAttachments(route_messages);
					for (const Value& mv : route_messages) { ValueMap msg = mv; String id = msg["message_id"]; if (id.IsEmpty() || seen.Find(id) >= 0) continue; seen.Add(id, true); msg.Set("resource_type", is_thread[resource] ? "thread" : "channel"); all_messages.Add(msg); captured++; resource_captured++; if (resource_captured >= 500) break; }
					String cursor = pm["cursor"];
					if (!cursor.IsEmpty()) oldest = cursor;
					if (incremental) break;
					Value moved = navigator.Eval(R"JS(return (function(){var n=document.querySelector('main [class*="scroller"]'); if(!n){var a=document.querySelectorAll('[class*="scroller"]'); n=a.length?a[a.length-1]:null;} if(!n)return false; var b=n.scrollTop; n.scrollTop=Math.max(0,b-n.clientHeight*.85); return b!==n.scrollTop;})())JS");
					if (!((bool)moved)) break;
					Sleep(700);
				}
				cursors.Add(ValueMap()("url", resource_url)("oldest_message_id", oldest)("pages", pages)("captured_messages", resource_captured)("complete", pages < 20 && resource_captured < 500));
				unread.Add(ValueMap()("url", resource_url)("count", unread_count));
			} catch (const Exc& e) { failures.Add(ValueMap()("url", resource_url)("error", (String)e)); }
		}
		envelope.Set("messages", all_messages);
		envelope.Set("cursors", cursors);
		envelope.Set("unread", unread);
		envelope.Set("partial_failures", failures);
		ValueMap diagnostics = envelope["diagnostics"];
		diagnostics.Set("discovered_channels", channels.GetCount());
		diagnostics.Set("discovered_threads", threads.GetCount());
		diagnostics.Set("captured_messages", all_messages.GetCount());
		diagnostics.Set("failed_resources", failures.GetCount());
		envelope.Set("diagnostics", diagnostics);
		if (!no_theme) {
			Value theme = navigator.Eval(R"JS(return (function(){
				function safe(v){v=(v||'').trim(); return !v || (v.length < 160 && !/[<>]|javascript:|url\s*\(|https?:\/\//i.test(v));}
				var root=getComputedStyle(document.documentElement), body=getComputedStyle(document.body), out={colors:{},fonts:{},vars:{}};
				['--background-primary','--background-secondary','--background-tertiary','--text-normal','--text-muted','--brand-experiment'].forEach(function(k){var v=root.getPropertyValue(k).trim();if(safe(v))out.vars[k]=v;});
				[['background',body.backgroundColor],['color',body.color]].forEach(function(p){if(safe(p[1]))out.colors[p[0]]=p[1];});
				if(safe(body.fontFamily))out.fonts.family=body.fontFamily; if(safe(body.fontSize))out.fonts.size=body.fontSize; return out;
			})())JS");
			envelope.Set("theme_profile", theme);
		}
		sm.SetSiteData(site_name, "messages", all_messages);
		// Do not restore by changing the address bar; the pinned SPA tab remains authenticated.
		GetAriaLogger("discord").Info(Format("Guild crawl captured %d messages across %d resources", all_messages.GetCount(), urls.GetCount()));
	} catch (const Exc& e) {
		ValueArray root_failure;
		root_failure.Add(ValueMap()("url", url)("error", (String)e));
		envelope.Set("partial_failures", root_failure);
		GetAriaLogger("discord").Error("Error crawling Discord guild: " + e);
		// Do not restore by changing the address bar.
	}
	return envelope;
}

END_UPP_NAMESPACE
