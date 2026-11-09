print("--- AriaHub News Dump Test ---")
wait_time(2.0)
tab = find("Main/Tabs/News")
if not tab:
    tab = find("News")
if tab:
    print("Clicking News Tab...")
    tab.click()
    wait_time(2.0)
btn = find("BtnRefresh")
if not btn:
    btn = find("Refresh")
if btn:
    print("Triggering Global Refresh...")
    btn.click()
loop = 0
while loop < 30:
    wait_time(10.0)
    print("Heartbeat - " + str(loop))
    ui = dump_ui()
    # Using chr(10) for newline splitting
    lines = ui.split(chr(10))
    l_idx = 0
    while l_idx < len(lines):
        line = lines[l_idx]
        if "NewsList/Rows/Row" in line:
            print("DATA: " + line)
        l_idx = l_idx + 1
    loop = loop + 1
print("Test Finished.")
_exit(0)
