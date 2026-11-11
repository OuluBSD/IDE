#include <Core/Core.h>
#include <DevEnv/Core/Core.h>

using namespace Upp;
using namespace Upp::DevEnv;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	Cout() << "========================================\n";
	Cout() << "Running DevEnvCheck Unit & Lifecycle Tests\n";
	Cout() << "========================================\n\n";

	// 1. Test CompilerMessage Data Model
	Cout() << "[1] Testing CompilerMessage structure and kinds...\n";
	{
		CompilerMessage msg(MessageKind::Error, "main.cpp", 42, 10, "syntax error: expected semicolon");
		ASSERT(msg.kind == MessageKind::Error);
		ASSERT(msg.file == "main.cpp");
		ASSERT(msg.line == 42);
		ASSERT(msg.column == 10);
		ASSERT(msg.text == "syntax error: expected semicolon");

		CompilerMessage warn(MessageKind::Warning, "util.cpp", 15, 0, "unused variable 'x'");
		ASSERT(warn.kind == MessageKind::Warning);

		CompilerMessage info(MessageKind::Info, "", 0, 0, "Build completed in 1.2s");
		ASSERT(info.kind == MessageKind::Info);

		CompilerMessage fatal(MessageKind::Fatal, "App.upp", 1, 0, "package not found");
		ASSERT(fatal.kind == MessageKind::Fatal);

		Cout() << "PASS: CompilerMessage verified\n";
	}

	// 2. Test BuildEvents Dispatch
	Cout() << "\n[2] Testing BuildEvents dispatch and lifecycle...\n";
	{
		BuildEvents events;
		int progress_count = 0;
		int message_count = 0;
		bool finished_called = false;
		bool finish_ok = false;

		events.WhenProgress = [&](int done, int total, const String& pkg) {
			progress_count++;
			ASSERT(done <= total);
			ASSERT(!pkg.IsEmpty());
		};

		events.WhenMessage = [&](const CompilerMessage& msg) {
			message_count++;
			ASSERT(msg.kind == MessageKind::Warning);
		};

		events.WhenFinished = [&](bool success, int error_count) {
			finished_called = true;
			finish_ok = success;
			ASSERT(error_count == 0);
		};

		// Dispatch simulated events
		events.WhenProgress(1, 3, "Core");
		events.WhenProgress(2, 3, "DevEnv/Core");
		events.WhenMessage(CompilerMessage(MessageKind::Warning, "foo.cpp", 10, 1, "test warning"));
		events.WhenProgress(3, 3, "DevEnv/App");
		events.WhenFinished(true, 0);

		ASSERT(progress_count == 3);
		ASSERT(message_count == 1);
		ASSERT(finished_called);
		ASSERT(finish_ok);

		Cout() << "PASS: BuildEvents dispatch verified\n";
	}

	// 3. Test PortageManager Data Model & VDB Resolution
	Cout() << "\n[3] Testing PortageManager Model & Package Resolution...\n";
	{
		PortageManager pm;
		String vdb = PortageManager::GetVdbDir();
		ASSERT(!vdb.IsEmpty());

		String overlay = PortageManager::GetOverlayDir();
		ASSERT(!overlay.IsEmpty());

		// Test package lookup
		PortagePackage basic_html;
		bool found = pm.IsInstalled("DevCpp/BasicHtmlRenderer", &basic_html);
		ASSERT(found);
		ASSERT(basic_html.atom == "DevCpp/BasicHtmlRenderer");
		ASSERT(basic_html.category == "DevCpp");
		ASSERT(basic_html.name == "BasicHtmlRenderer");
		ASSERT(basic_html.installed);

		// Test package listing
		Vector<PortagePackage> pkgs = pm.ListPackages();
		ASSERT(pkgs.GetCount() > 0);

		bool saw_basichtml = false;
		for (const auto& p : pkgs) {
			if (p.atom == "DevCpp/BasicHtmlRenderer") {
				saw_basichtml = true;
				ASSERT(p.installed);
				break;
			}
		}
		ASSERT(saw_basichtml);

		Cout() << "PASS: PortageManager model & VDB resolution verified (" << pkgs.GetCount() << " packages indexed)\n";
	}

	// 4. Test AgentBridge IPC & Event Dispatch
	Cout() << "\n[4] Testing AgentBridge IPC & Background Event Dispatch...\n";
	{
		AgentMessage msg;
		msg.type = "test_result";
		msg.target = "autotest/Bits";
		msg.status = "pass";
		msg.duration_ms = 45;
		msg.message = "All assertions passed";

		String json = msg.ToJson();
		ASSERT(!json.IsEmpty());
		ASSERT(json.Find("autotest/Bits") >= 0);
		ASSERT(json.Find("pass") >= 0);

		AgentMessage parsed = AgentMessage::FromJson(json);
		ASSERT(parsed.type == "test_result");
		ASSERT(parsed.target == "autotest/Bits");
		ASSERT(parsed.status == "pass");
		ASSERT(parsed.duration_ms == 45);

		// Test Frame formatting and parsing
		ValueMap payload;
		payload.Add("compiler", "clang++");
		payload.Add("jobs", 8);
		String frame = AgentBridge::FormatFrame("agent_status", payload);
		ASSERT(frame.Find("clang++") >= 0);

		String out_type;
		ValueMap out_payload;
		bool ok = AgentBridge::ParseFrame(frame, out_type, out_payload);
		ASSERT(ok);
		ASSERT(out_type == "agent_status");
		ASSERT(out_payload["compiler"] == "clang++");

		// Test Bridge Event Callback and Background Test Runner
		AgentBridge bridge;
		bool test_cb_fired = false;
		String cb_target;
		bool cb_ok = false;

		bridge.WhenTestResult = [&](const String& target, bool pass, int ms) {
			test_cb_fired = true;
			cb_target = target;
			cb_ok = pass;
			ASSERT(ms >= 0);
		};

		bool test_run = bridge.TriggerBackgroundTest("autotest/Bits");
		ASSERT(test_run);
		ASSERT(test_cb_fired);
		ASSERT(cb_target == "autotest/Bits");
		ASSERT(cb_ok);

		Cout() << "PASS: AgentBridge IPC, JSON framing & background test dispatch verified\n";
	}

	Cout() << "\n========================================\n";
	Cout() << "ALL DevEnvCheck TESTS PASSED SUCCESSFULLY!\n";
	Cout() << "========================================\n";
}
