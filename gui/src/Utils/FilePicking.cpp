#include <HMP/Gui/Utils/FilePicking.hpp>

#include <cinolib/gl/file_dialog_save.h>
#include <cinolib/gl/file_dialog_open.h>
#include <filesystem>

namespace HMP::Gui::Utils::FilePicking
{

	std::string withExt(const std::string& _filename, const std::string& _ext)
	{
		if (!_filename.empty())
		{
			const std::string dotExt{ "." + _ext };
			std::string oldExt{ std::filesystem::path{ _filename }.extension().string() };
			for (char& c : oldExt) c = static_cast<char>(std::tolower(c));
			if (oldExt != dotExt)
			{
				return _filename + dotExt;
			}
		}
		return _filename;
	}

	std::optional<std::string> toOptional(const std::string& _filenameOrEmpty)
	{
		if (_filenameOrEmpty.empty())
		{
			return std::nullopt;
		}
		return _filenameOrEmpty;
	}

	std::optional<std::string> open()
	{
		return toOptional(cinolib::file_dialog_open());
	}

	std::optional<std::string> save(const std::string& _extension)
	{
		std::optional<std::string> filename{ save() };
		if (filename)
		{
			const std::string dotExt{ "." + _extension };
			std::string oldExt{ std::filesystem::path{ *filename }.extension().string() };
			for (char& c : oldExt) c = static_cast<char>(std::tolower(c));
			if (oldExt != dotExt)
			{
				return *filename + dotExt;
			}
		}
		return filename;
	}

	std::optional<std::string> save()
	{
		return toOptional(cinolib::file_dialog_save());
	}

}