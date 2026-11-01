#ifndef _DevEnv_Core_CompilerMessage_h_
#define _DevEnv_Core_CompilerMessage_h_

#include <Core/Core.h>

namespace Upp {
namespace DevEnv {

enum class MessageKind {
	Info,
	Warning,
	Error,
	Fatal
};

struct CompilerMessage : Moveable<CompilerMessage> {
	MessageKind kind = MessageKind::Info;
	String file;
	int line = 0;
	int column = 0;
	String text;
	String raw_output;

	CompilerMessage() = default;
	CompilerMessage(MessageKind k, const String& f, int l, int col, const String& txt)
		: kind(k), file(f), line(l), column(col), text(txt) {}
};

}
}

#endif
