#ifndef _DevEnv_Core_BuildEvents_h_
#define _DevEnv_Core_BuildEvents_h_

#include "CompilerMessage.h"

namespace Upp {
namespace DevEnv {

struct BuildEvents {
	Function<void(int done, int total, const String& pkg)> WhenProgress;
	Function<void(const CompilerMessage& msg)>             WhenMessage;
	Function<void(bool success, int error_count)>         WhenFinished;
};

}
}

#endif
