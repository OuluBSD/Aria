# AriaHub GUI Test: Menu and Shortcuts
import sys

print("Checking Menu structure...")
ui = dump_ui()
print("Full UI Dump:")
print(ui)

if "Service/Refresh Active Sub-tab" in ui:
    print("✓ Success: Menu item 'Refresh Active Sub-tab' found.")
else:
    print("Error: Menu item 'Refresh Active Sub-tab' NOT found.")
    sys.exit(1)

print("Triggering Refresh Active Sub-tab via F5 shortcut...")
send_key(K_F5)
wait_time(1.0)

print("Triggering Refresh Active Service via Shift+F5 shortcut...")
send_key(K_SHIFT_F5)
wait_time(1.0)

print("Triggering All Services refresh via Ctrl+Shift+F5 shortcut...")
send_key(K_CTRL_SHIFT_F5)
wait_time(1.0)

print("Test finished.")
_exit(0)
