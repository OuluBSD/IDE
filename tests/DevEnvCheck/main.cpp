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

	Cout() << "\n========================================\n";
	Cout() << "ALL DevEnvCheck TESTS PASSED SUCCESSFULLY!\n";
	Cout() << "========================================\n";
}
