#include <ccore/event.h>

char ccEventKeyToChar(int key)
{
	if(key >= CC_KEY_0 && key <= CC_KEY_9){
		return '0' + (key - CC_KEY_0);
	}

	if(key >= CC_KEY_NUM0 && key <= CC_KEY_NUM9){
		return '0' + (key - CC_KEY_NUM0);
	}

	if(key >= CC_KEY_A && key <= CC_KEY_Z){
		return 'a' + (key - CC_KEY_A);
	}

	if(key >= CC_KEY_EXCLAM && key <= CC_KEY_SLASH){
		return '!' + (key - CC_KEY_EXCLAM);
	}
	
	if(key >= CC_KEY_COLON && key <= CC_KEY_AT){
		return ':' + (key - CC_KEY_COLON);
	}
	
	if(key >= CC_KEY_BRACKETLEFT && key <= CC_KEY_QUOTELEFT){
		return '[' + (key - CC_KEY_BRACKETLEFT);
	}
	
	if(key >= CC_KEY_BRACELEFT && key <= CC_KEY_ASCIITILDE){
		return '{' + (key - CC_KEY_BRACELEFT);
	}

	switch(key){
		case CC_KEY_TAB:
			return '\t';
		case CC_KEY_RETURN:
			return '\n';
		case CC_KEY_SPACE:
			return ' ';
		default:
			return '\0';
	}
}

char *ccEventKeyToStr(int key)
{
	switch(key) {
	case CC_KEY_0:
		return "0";
	case CC_KEY_1:
		return "1";
	case CC_KEY_2:
		return "2";
	case CC_KEY_3:
		return "3";
	case CC_KEY_4:
		return "4";
	case CC_KEY_5:
		return "5";
	case CC_KEY_6:
		return "6";
	case CC_KEY_7:
		return "7";
	case CC_KEY_8:
		return "8";
	case CC_KEY_9:
		return "9";
	case CC_KEY_F1:
		return "F1";
	case CC_KEY_F2:
		return "F2";
	case CC_KEY_F3:
		return "F3";
	case CC_KEY_F4:
		return "F4";
	case CC_KEY_F5:
		return "F5";
	case CC_KEY_F6:
		return "F6";
	case CC_KEY_F7:
		return "F7";
	case CC_KEY_F8:
		return "F8";
	case CC_KEY_F9:
		return "F9";
	case CC_KEY_F10:
		return "F10";
	case CC_KEY_F11:
		return "F11";
	case CC_KEY_F12:
		return "F12";
	case CC_KEY_NUM0:
		return "Numpad 0";
	case CC_KEY_NUM1:
		return "Numpad 1";
	case CC_KEY_NUM2:
		return "Numpad 2";
	case CC_KEY_NUM3:
		return "Numpad 3";
	case CC_KEY_NUM4:
		return "Numpad 4";
	case CC_KEY_NUM5:
		return "Numpad 5";
	case CC_KEY_NUM6:
		return "Numpad 6";
	case CC_KEY_NUM7:
		return "Numpad 7";
	case CC_KEY_NUM8:
		return "Numpad 8";
	case CC_KEY_NUM9:
		return "Numpad 9";
	case CC_KEY_A:
		return "A";
	case CC_KEY_B:
		return "B";
	case CC_KEY_C:
		return "C";
	case CC_KEY_D:
		return "D";
	case CC_KEY_E:
		return "E";
	case CC_KEY_F:
		return "F";
	case CC_KEY_G:
		return "G";
	case CC_KEY_H:
		return "H";
	case CC_KEY_I:
		return "I";
	case CC_KEY_J:
		return "J";
	case CC_KEY_K:
		return "K";
	case CC_KEY_L:
		return "L";
	case CC_KEY_M:
		return "M";
	case CC_KEY_N:
		return "N";
	case CC_KEY_O:
		return "O";
	case CC_KEY_P:
		return "P";
	case CC_KEY_Q:
		return "Q";
	case CC_KEY_R:
		return "R";
	case CC_KEY_S:
		return "S";
	case CC_KEY_T:
		return "T";
	case CC_KEY_U:
		return "U";
	case CC_KEY_V:
		return "V";
	case CC_KEY_W:
		return "W";
	case CC_KEY_X:
		return "X";
	case CC_KEY_Y:
		return "Y";
	case CC_KEY_Z:
		return "Z";
	case CC_KEY_BACKSPACE:
		return "Backspace";
	case CC_KEY_TAB:
		return "Tab";
	case CC_KEY_RETURN:
		return "Return";
	case CC_KEY_ESCAPE:
		return "Escape";
	case CC_KEY_SPACE:
		return "Space";
	case CC_KEY_CAPSLOCK:
		return "Caps lock";
	case CC_KEY_INSERT:
		return "Insert";
	case CC_KEY_DELETE:
		return "Delete";
	case CC_KEY_HOME:
		return "Home";
	case CC_KEY_END:
		return "End";
	case CC_KEY_PAGEUP:
		return "Page up";
	case CC_KEY_PAGEDOWN:
		return "Page down";
	case CC_KEY_PRINTSCREEN:
		return "Print screen";
	case CC_KEY_SCROLLLOCK:
		return "Scroll lock";
	case CC_KEY_NUMLOCK:
		return "Num lock";
	case CC_KEY_PAUSEBREAK:
		return "Pause";
	case CC_KEY_LSHIFT:
		return "Left shift";
	case CC_KEY_RSHIFT:
		return "Right shift";
	case CC_KEY_LCONTROL:
		return "Left control";
	case CC_KEY_RCONTROL:
		return "Right control";
	case CC_KEY_LEFT:
		return "Left";
	case CC_KEY_UP:
		return "Up";
	case CC_KEY_RIGHT:
		return "Right";
	case CC_KEY_DOWN:
		return "Down";
	default:
		return "Unknown key";
	}
}
