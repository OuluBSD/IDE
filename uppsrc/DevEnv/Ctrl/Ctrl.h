#ifndef _DevEnv_Ctrl_Ctrl_h_
#define _DevEnv_Ctrl_Ctrl_h_

#include <CtrlLib/CtrlLib.h>
#include <DevEnv/Draw/Draw.h>

#include "AiAssistantCtrl.h"

namespace Upp {
namespace DevEnv {

class OutputConsole : public Ctrl {
public:
	OutputConsole() = default;
	virtual ~OutputConsole() = default;
};

}
}

#endif
