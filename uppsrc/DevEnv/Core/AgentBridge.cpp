#include "AgentBridge.h"
#include <chrono>

namespace Upp {
namespace DevEnv {

String AgentMessage::ToString() const
{
	String s;
	s << "[" << type << "] target=" << target << " status=" << status
	  << " (" << duration_ms << "ms)";
	if (!message.IsEmpty()) s << " msg=" << message;
	return s;
}

String AgentMessage::ToJson() const
{
	Json json;
	json("type", type)
	    ("target", target)
	    ("status", status)
	    ("duration_ms", duration_ms)
	    ("message", message)
	    ("time", Format(timestamp));
	return json.ToString();
}

AgentMessage AgentMessage::FromJson(const String& json_str)
{
	AgentMessage msg;
	Value v = ParseJSON(json_str);
	if (IsValueMap(v)) {
		ValueMap m = v;
		msg.type = m["type"];
		msg.target = m["target"];
		msg.status = m["status"];
		msg.duration_ms = (int)m["duration_ms"];
		msg.message = m["message"];
	}
	return msg;
}

AgentBridge::AgentBridge()
{
}

AgentBridge::~AgentBridge()
{
	Disconnect();
}

bool AgentBridge::Connect(const String& host, int port)
{
	control_host = host;
	control_port = port;

	TcpSocket sock;
	sock.Timeout(500);
	if (sock.Connect(host, port)) {
		connected = true;
		sock.Close();
		return true;
	}
	// Fallback to loopback readiness flag if daemon is offline
	connected = false;
	return false;
}

void AgentBridge::Disconnect()
{
	connected = false;
}

bool AgentBridge::SendEvent(const AgentMessage& msg)
{
	String frame = msg.ToJson() + "\n";
	TcpSocket sock;
	sock.Timeout(500);
	if (sock.Connect(control_host, control_port)) {
		sock.Put(frame);
		sock.Close();
		return true;
	}
	// If direct socket is not listening, invoke local callback directly
	if (WhenMessageReceived) {
		WhenMessageReceived(msg);
	}
	return true;
}

bool AgentBridge::TriggerBackgroundTest(const String& atom)
{
	auto t0 = std::chrono::steady_clock::now();
	String cmd = "uppbuilder test " + atom + " 2>&1";
	String log = Sys(cmd);
	auto t1 = std::chrono::steady_clock::now();
	int ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

	bool ok = (log.Find("[PASS]") >= 0 || log.Find("0 failed") >= 0);

	AgentMessage msg;
	msg.type = "test_result";
	msg.target = atom;
	msg.status = ok ? "pass" : "fail";
	msg.duration_ms = ms;
	msg.message = ok ? "Test suite passed" : "Test suite failed";

	if (WhenTestResult) {
		WhenTestResult(atom, ok, ms);
	}

	SendEvent(msg);
	return ok;
}

String AgentBridge::FormatFrame(const String& type, const ValueMap& payload)
{
	Json json;
	json("type", type);
	for (int i = 0; i < payload.GetCount(); i++) {
		json(payload.GetKey(i).ToString(), payload.GetValue(i));
	}
	return json.ToString();
}

bool AgentBridge::ParseFrame(const String& frame_str, String& out_type, ValueMap& out_payload)
{
	Value v = ParseJSON(frame_str);
	if (!IsValueMap(v)) return false;
	out_payload = v;
	out_type = out_payload["type"];
	return !out_type.IsEmpty();
}

}
}
