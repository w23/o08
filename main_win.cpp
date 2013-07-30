#include <iostream>
#include <windows.h>
#undef min
#undef max
#include <kapusha/core.h>
#include <kapusha/sys/win/Window.h>

using namespace kapusha;

extern IViewport *createGame(int argc, const char *argv[]);

void log::sys_write(const char *message) {
  OutputDebugStringA(message);
  OutputDebugStringA("\n");
  std::cerr << message << std::endl;
}

class Args {
public:
	Args(LPTSTR cmdLine);
	~Args();

  inline int argc() const { return argc_; }
	const char** argv() const { return const_cast<const char**>(argv_); }

private:
	int argc_;
	char **argv_;
};

Args::Args(LPTSTR cmdLine) {
	LPWSTR *wargs = CommandLineToArgvW(cmdLine, &argc_);
	if (argc_ == 0)
		argv_ = nullptr;
	else {
		argv_ = new char*[argc_];
		for (int i = 0; i < argc_; ++i) {
			int len = WideCharToMultiByte(CP_UTF8, 0, wargs[i], -1, NULL, 0, NULL, NULL);
			argv_[i] = new char[len];
			WideCharToMultiByte(CP_UTF8, 0, wargs[i], -1, argv_[i], len, NULL, NULL);
		}
	}
	LocalFree(wargs);
}

Args::~Args() {
	for (int i = 0; i < argc_; ++i)
		delete[] argv_[i];
	delete[] argv_;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInst);
  UNREFERENCED_PARAMETER(cmdLine);
  UNREFERENCED_PARAMETER(cmdShow);

  Args args(GetCommandLine());
  IViewport *game = createGame(args.argc(), args.argv());

  if (game) return RunWindow(hInst, game, 800, 600, false);
  return -1;
}