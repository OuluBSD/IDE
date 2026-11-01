#ifndef _DevEnv_Draw_Draw_h_
#define _DevEnv_Draw_Draw_h_

#include <Draw/Draw.h>
#include <DevEnv/Core/Core.h>

namespace Upp {
namespace DevEnv {

struct SyntaxTheme {
	Color background = SColorPaper();
	Color text = SColorText();
	Color keyword = LtBlue();
	Color string_literal = LtRed();
	Color comment = Gray();
	Color number = LtGreen();
	Color preprocessor = LtMagenta();
};

}
}

#endif
