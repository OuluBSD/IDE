#ifndef _DevEnv_Ctrl_Core_Core_h_
#define _DevEnv_Ctrl_Core_Core_h_

#include <DevEnv/Ctrl/Ctrl.h>

namespace Upp {
namespace DevEnv {

class BuildController {
public:
	BuildEvents events;
	BuildController() = default;
};

}
}

#endif
