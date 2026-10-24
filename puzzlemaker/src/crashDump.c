#ifdef _WIN32

#include <windows.h>
#include <dbghelp.h>

long createCrashDump(EXCEPTION_POINTERS* exceptionInfo)
{
	HANDLE hFile = CreateFileA("crash.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
		dumpInfo.ThreadId = GetCurrentThreadId();
		dumpInfo.ExceptionPointers = exceptionInfo;
		dumpInfo.ClientPointers = TRUE;

		char success = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
										 MiniDumpWithProcessThreadData, &dumpInfo, NULL, NULL);

		CloseHandle(hFile);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

void initCrashHandler()
{
	SetUnhandledExceptionFilter(createCrashDump);
}

#else

void initCrashHandler()
{
}

#endif
