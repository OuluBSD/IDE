#ifndef _DevEnv_Core_AgentBridge_h_
#define _DevEnv_Core_AgentBridge_h_

#include <Core/Core.h>

namespace Upp {
namespace DevEnv {

struct AgentMessage : Moveable<AgentMessage> {
	String type;        // e.g. "build_event", "test_result", "file_changed"
	String target;      // e.g. "autotest/Bits", "DevCpp/BasicHtmlRenderer"
	String status;      // "pass", "fail", "building"
	int    duration_ms = 0;
	String message;
	Time   timestamp;

	AgentMessage() : timestamp(GetSysTime()) {}

	String ToString() const;
	String ToJson() const;
	static AgentMessage FromJson(const String& json_str);
};

class AgentBridge {
public:
	AgentBridge();
	~AgentBridge();

	bool Connect(const String& host = "127.0.0.1", int port = 7421);
	void Disconnect();
	bool IsConnected() const { return connected; }

	bool SendEvent(const AgentMessage& msg);
	bool TriggerBackgroundTest(const String& atom);

	Event<AgentMessage> WhenMessageReceived;
	Event<String, bool, int> WhenTestResult; // atom, success, duration_ms

	static String FormatFrame(const String& type, const ValueMap& payload);
	static bool ParseFrame(const String& frame_str, String& out_type, ValueMap& out_payload);

private:
	bool   connected = false;
	int    control_port = 7421;
	String control_host = "127.0.0.1";
};

}
}

#endif
