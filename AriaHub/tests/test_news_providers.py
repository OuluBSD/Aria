print("--- News Extraction Full Dump ---")
wait_time(2.0)
tab = find("Main/Tabs/News")
if not tab:
    tab = find("News")
tab.click()
wait_time(3.0)
btn = find("BtnRefresh")
if not btn:
    btn = find("Refresh")
btn.click()
loop = 0
while loop < 10:
    wait_time(60.0)
    print("UI DUMP AT " + str(loop) + " MINUTES:")
    print(dump_ui())
    print("---------------------------------")
    loop = loop + 1
_exit(0)
