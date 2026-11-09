print("Selecting Threads main tab...")
# The path seems to be Main/Tabs/Threads (or just 'Threads' if we find by label)
threads_main_tab = find("Threads")
if not threads_main_tab:
    print("Error: Threads tab not found.")
    _exit(1)
threads_main_tab.click()
wait_time(2.0)

# After clicking the main tab, we dump the UI to see the exact path of the sub-tabs
# which should now be visible.
print("Current UI State:")
ui = dump_ui()
# print(ui)

print("Selecting Feed sub-tab...")
# From dump: Main/Tabs/SubTabs/Feed
# If find() is smart it should find it.
feed_sub_tab = find("Main/Tabs/SubTabs/Feed")
if not feed_sub_tab:
    # Look for any 'Feed' that is not hidden
    for line in ui.split('\n'):
        if "Feed =" in line and "(hidden)" not in line:
            p = line.split('=')[0].strip()
            feed_sub_tab = find(p)
            break

if not feed_sub_tab:
    print("Error: Visible Feed sub-tab not found.")
    _exit(1)

feed_sub_tab.click()
wait_time(1.0)

print("Finding row in FeedTree...")
row = find("FeedTree/Rows/Row 0")
if not row:
    # Search for visible row in dump
    ui = dump_ui()
    for line in ui.split('\n'):
        if "FeedTree/Rows/Row" in line and "(hidden)" not in line:
            p = line.split('=')[0].strip()
            row = find(p)
            break

if not row:
    print("Error: Could not find visible row in FeedTree.")
    _exit(1)

print("Clicking row to expand: " + row.path)
row.click() # Select
wait_time(1.0)
row.click() # Double click simulation

print("Waiting for response...")
wait_time(15.0)
print("Finished.")
_exit(0)
