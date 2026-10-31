#ifndef _WebDriver_Keys_h_
#define _WebDriver_Keys_h_

#include <Core/Core.h>



NAMESPACE_UPP

namespace keys {

const String Null = "\xEE\x80\x80";
const String Cancel = "\xEE\x80\x81";
const String Help = "\xEE\x80\x82";
const String Backspace = "\xEE\x80\x83";
const String Tab = "\xEE\x80\x84";
const String Clear = "\xEE\x80\x85";
const String Return_key = "\xEE\x80\x86";
const String Enter = "\xEE\x80\x87";
const String Shift = "\xEE\x80\x88";
const String Left_shift = "\xEE\x80\x89";
const String Control = "\xEE\x80\x8A";
const String Left_control = "\xEE\x80\x8B";
const String Alt = "\xEE\x80\x8C";
const String Left_alt = "\xEE\x80\x8D";
const String Pause = "\xEE\x80\x8E";
const String Escape = "\xEE\x80\x8F";
const String Space = "\xEE\x80\x90";
const String Page_up = "\xEE\x80\x91";
const String Page_down = "\xEE\x80\x92";
const String End = "\xEE\x80\x93";
const String Home = "\xEE\x80\x94";
const String Left = "\xEE\x80\x95";
const String Arrow_left = "\xEE\x80\x95";
const String Up = "\xEE\x80\x96";
const String Arrow_up = "\xEE\x80\x96";
const String Right = "\xEE\x80\x97";
const String Arrow_right = "\xEE\x80\x97";
const String Down = "\xEE\x80\x98";
const String Arrow_down = "\xEE\x80\x98";
const String Insert = "\xEE\x80\x99";
const String Delete = "\xEE\x80\x9A";
const String Semicolon = "\xEE\x80\x9B";
const String Equals = "\xEE\x80\x9C";
const String Numpad0 = "\xEE\x80\x9D";
const String Numpad1 = "\xEE\x80\x9E";
const String Numpad2 = "\xEE\x80\x9F";
const String Numpad3 = "\xEE\x80\xA0";
const String Numpad4 = "\xEE\x80\xA1";
const String Numpad5 = "\xEE\x80\xA2";
const String Numpad6 = "\xEE\x80\xA3";
const String Numpad7 = "\xEE\x80\xA4";
const String Numpad8 = "\xEE\x80\xA5";
const String Numpad9 = "\xEE\x80\xA6";
const String Multiply = "\xEE\x80\xA7";
const String Add = "\xEE\x80\xA8";
const String Separator = "\xEE\x80\xA9";
const String Subtract = "\xEE\x80\xAA";
const String Decimal = "\xEE\x80\xAB";
const String Divide = "\xEE\x80\xAC";
const String F1 = "\xEE\x80\xAD";
const String F2 = "\xEE\x80\xAE";
const String F3 = "\xEE\x80\xAF";
const String F4 = "\xEE\x80\xB0";
const String F5 = "\xEE\x80\xB1";
const String F6 = "\xEE\x80\xB2";
const String F7 = "\xEE\x80\xB3";
const String F8 = "\xEE\x80\xB4";
const String F9 = "\xEE\x80\xB5";
const String F10 = "\xEE\x80\xB6";
const String F11 = "\xEE\x80\xB7";
const String F12 = "\xEE\x80\xB8";
const String Meta = "\xEE\x80\xB9";
const String Command = "\xEE\x80\xB9";

} // namespace keys

struct Shortcut {
	Vector<String> keys;
	
	Shortcut(const String& key);
	Shortcut(const Vector<String>& keys);
};

END_UPP_NAMESPACE

#endif