#ifndef _DevEnv_App_App_h_
#define _DevEnv_App_App_h_

#include <DevEnv/Ctrl/Lib/Lib.h>

namespace Upp {
namespace DevEnv {

class AppWindow : public TopWindow {
public:
	AppWindow();
	virtual ~AppWindow() = default;

private:
	AiAssistantCtrl ai_assistant;
};

}
}

#endif
