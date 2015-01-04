/* id3ted: fileio.h
 * Copyright (c) 2011 Bert Muennich <be.muennich at googlemail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef FILEIO_H
#define FILEIO_H

#include <cstdio>
#include <sys/time.h>

#include <taglib/tbytevector.h>

#include "id3ted.h"

typedef struct {
	struct timeval access;
	struct timeval modification;
} FileTimes;

class FileIO {
	public:
		enum Status {
			Success = 0,
			Abort,
			Error
		};

		static bool exists(const char*);
		static bool isRegular(const char*);
		static bool isReadable(const char*);
		static bool isWritable(const char*);
		static string sizeHumanReadable(unsigned long);
		static const char* mimetype(const char*);
		static Status saveTimes(const char*, FileTimes&);
		static Status resetTimes(const char*, const FileTimes&);
		static Status createDir(const char*);
		static bool confirmOverwrite(const char*);
		static Status copy(const char*, const char*);
		static Status remove(const char*);

		FileIO(const char*, const char*);
		virtual ~FileIO() = 0;

		bool isOpen() { return stream != NULL; }
		Status close();
		bool eof() { return stream != NULL && feof(stream); }
		bool error() { return stream == NULL || ferror(stream); }
		long tell();
		Status seek(long);

	protected:
		FILE *stream;
		const char *path;
		const char *mode;
};

class IFile : public FileIO {
	public:
		IFile(const char *_path) : FileIO(_path, "r") {}
		~IFile() { if (stream) close(); }

		size_t read(char*, size_t);
		size_t read(ByteVector&);
};

class OFile : public FileIO {
	public:
		OFile(const char *_path) : FileIO(_path, "w+") {}
		~OFile() { if (stream) close(); }

		size_t write(const char*, size_t);
		size_t write(const ByteVector&);
};

#endif /* FILEIO_H */
