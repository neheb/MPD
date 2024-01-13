// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "ArchiveInputPlugin.hxx"
#include "archive/ArchiveList.hxx"
#include "archive/ArchivePlugin.hxx"
#include "archive/ArchiveFile.hxx"
#include "../InputStream.hxx"
#include "fs/LookupFile.hxx"
#include "fs/NarrowPath.hxx"
#include "fs/Path.hxx"
#include "lib/fmt/ExceptionFormatter.hxx"
#include "lib/fmt/PathFormatter.hxx"
#include "util/Domain.hxx"
#include "Log.hxx"

static constexpr Domain input_domain("input");

InputStreamPtr
OpenArchiveInputStream(Path path, Mutex &mutex)
{
	const ArchivePlugin *arplug;

	ArchiveLookupResult l;
	try {
		l = LookupFile(path);
		if (l.archive.IsNull()) {
			return nullptr;
		}
	} catch (...) {
		FmtDebug(input_domain,
			 "not an archive, lookup '{}' failed: {}",
			 path, std::current_exception());
		return nullptr;
	}

	const auto suffix = l.archive.GetExtension();
	if (suffix == nullptr)
		return nullptr;

	const auto suffix_utf8 = Path::FromFS(suffix).ToUTF8();

	//check which archive plugin to use (by ext)
	arplug = archive_plugin_from_suffix(suffix_utf8.c_str());
	if (!arplug) {
		return nullptr;
	}

	auto np = NarrowPath(l.inside);
	return archive_file_open(arplug, l.archive)->OpenStream(np, mutex);
}
