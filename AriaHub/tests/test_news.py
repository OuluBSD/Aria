print("Switching to News tab...")
news_tab = find("Main/Tabs/News")
if not news_tab:
    news_tab = find("News")
if not news_tab:
    print("Error: News tab not found.")
    _exit(1)
news_tab.click()
wait_time(2.0)

print("Triggering News Scrape via Service Menu...")
# Shift+F5 is Refresh Active Service
send_key(K_SHIFT_F5)

print("Waiting for scrape to finish...")
wait_time(30.0) # Visiting 7 sites takes time

print("Checking NewsList...")
ui = dump_ui()
if "NewsList" in ui and "row =" in ui:
    print("✓ Success: News items detected.")
else:
    print("? No news items detected yet.")

_exit(0)
