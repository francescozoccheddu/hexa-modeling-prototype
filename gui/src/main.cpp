
#include <HMP/Gui/App.hpp>
#include <iostream>

int main(int _argc, char* _argv[])
{
	const char* file{};
	if (_argc == 2)
	{
		file = _argv[1];
		std::cout << "loading file '" << file << "'" << std::endl;
	}
	else if (_argc > 2)
	{
		std::cerr << "expected 0 or 1 argument, got " << _argc - 1 << std::endl;
		return 1;
	}
	HMP::Gui::App app{};
	if (file)
	{
		app.loadTargetMeshOrProjectFile(file);
	}
	return app.launch();
}