#include "generate-stack-trace.h"

#include <DbgHelp.h>
#include <psapi.h>

#include <optional>
#include <ranges>
#include <sstream>

#include "logging/global-macros.h"

auto StackWalk64GMod(PCONTEXT context, LPSTACKFRAME64 frame) {
	const auto process = GetCurrentProcess();
	const auto thread = GetCurrentThread();

	return StackWalk64(
		IMAGE_FILE_MACHINE_AMD64,
		process,
		thread,
		frame,
		context,
		nullptr,
		SymFunctionTableAccess64,
		SymGetModuleBase64,
		nullptr
	);
}

constexpr const char *DEFAULT_MODULE_NAME = "<no module found>";

auto GetModuleNameFromAddress(DWORD64 address) -> std::string {
	HMODULE module;
	if (GetModuleHandleEx(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
			reinterpret_cast<LPCTSTR>(address),
			&module
		)) {
		char moduleName[MAX_PATH];
		GetModuleBaseNameA(GetCurrentProcess(), module, moduleName, MAX_PATH);
		return moduleName;
	}

	return DEFAULT_MODULE_NAME;
}

auto GetFormattedAddress(DWORD64 address) {
	std::stringstream addressStream;
	addressStream << "0x" << std::hex << address;

	auto formattedAddress = addressStream.str();
	return formattedAddress;
}

auto GetSymbolName(DWORD64 address) -> std::string {
	// you need to actually dynamically allocate for the name
	// variable--otherwise it just runs over the stack pointer and absolutely
	// destroys everything... 0 documentation on this!
	auto *symbol = static_cast<IMAGEHLP_SYMBOL64 *>(
		calloc(sizeof(IMAGEHLP_SYMBOL64) + 256 * sizeof(char), 1)
	);

	memset(symbol, 0, sizeof(IMAGEHLP_SYMBOL64));
	symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
	symbol->MaxNameLength = 255;

	DWORD64 displacement;

	if (!SymGetSymFromAddr64(
			GetCurrentProcess(), address, &displacement, symbol
		)) {
		return "<no symbol found>";
	}

	return symbol->Name;
}

auto GetSymbolLine(DWORD64 address) -> std::optional<std::string> {
	IMAGEHLP_LINE64 line = {};
	memset(&line, 0, sizeof(IMAGEHLP_LINE64));
	line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	DWORD displacement;
	if (!SymGetLineFromAddr64(
			GetCurrentProcess(), address, &displacement, &line
		)) {
		return std::nullopt;
	}

	std::stringstream lineStream;
	lineStream << line.FileName << "(" << line.LineNumber << ")";
	return lineStream.str();
}

auto GetFrameSummary(LPSTACKFRAME64 frame) {
	std::string frameSummary;

	frameSummary += GetSymbolName(frame->AddrPC.Offset);

	if (const auto line = GetSymbolLine(frame->AddrPC.Offset);
		line.has_value()) {
		frameSummary += " at " + line.value();
	}

	frameSummary += " - " + GetFormattedAddress(frame->AddrPC.Offset) + " in " +
					GetModuleNameFromAddress(frame->AddrPC.Offset);

	return frameSummary;
}

namespace logging::exceptions {
auto GetFormattedStackTrace(PCONTEXT context) -> std::string {
	CONTEXT copiedContext = *context;

	std::vector<std::string> stackTraces = {};
	std::string fullStackTrace = {};

	int frame = 0;
	STACKFRAME64 stackFrame = {};
	memset(&stackFrame, 0, sizeof(STACKFRAME64));

	// important so we can get the correct stack frame
	stackFrame.AddrPC.Offset = context->Rip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = context->Rbp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = context->Rsp;
	stackFrame.AddrStack.Mode = AddrModeFlat;

	do {
		if (!StackWalk64GMod(&copiedContext, &stackFrame) ||
			stackFrame.AddrPC.Offset == 0) {
			break;
		}

		stackTraces.push_back(GetFrameSummary(&stackFrame));
	} while (stackFrame.AddrPC.Offset != 0 && frame++ < 64);

	fullStackTrace +=
		"Stacktrace at " + GetFormattedAddress(copiedContext.Rip) + "\n";

	for (const auto &trace : std::ranges::reverse_view(stackTraces)) {
		fullStackTrace += "> ";
		fullStackTrace += trace;
		fullStackTrace += "\n";
	}

	fullStackTrace += "End of stack trace\n";

	return std::move(fullStackTrace);
}
}  // namespace logging::exceptions
