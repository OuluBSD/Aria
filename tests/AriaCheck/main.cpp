#include <Core/Core.h>
#include <WebDriver/WebDriver.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	Cout() << "========================================\n";
	Cout() << "Running AriaCheck Unit & Lifecycle Tests\n";
	Cout() << "========================================\n\n";

	// 1. Test WebDriver By selector methods
	Cout() << "[1] Testing WebDriver By selectors...\n";
	By byId = By::Id("test_element");
	ASSERT(byId.GetMethod() == "id");
	ASSERT(byId.GetSelector() == "test_element");
	Cout() << "PASS: By::Id selector verified\n";

	By byCss = By::CssSelector("div.main-content > p");
	ASSERT(byCss.GetMethod() == "css selector");
	ASSERT(byCss.GetSelector() == "div.main-content > p");
	Cout() << "PASS: By::CssSelector selector verified\n";

	By byXpath = By::Xpath("//button[@type='submit']");
	ASSERT(byXpath.GetMethod() == "xpath");
	ASSERT(byXpath.GetSelector() == "//button[@type='submit']");
	Cout() << "PASS: By::Xpath selector verified\n";

	// 2. Test Capabilities browser constants
	Cout() << "\n[2] Testing WebDriver Capabilities constants...\n";
	ASSERT(String(browser::K_CHROME) == "chrome");
	ASSERT(String(browser::K_FIREFOX) == "firefox");
	Cout() << "PASS: Browser constants verified\n";

	Cout() << "\n========================================\n";
	Cout() << "ALL AriaCheck TESTS PASSED SUCCESSFULLY!\n";
	Cout() << "========================================\n";
}
