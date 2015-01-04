/* id3ted: mp3file.h
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

#ifndef MP3FILE_H
#define MP3FILE_H

#include <vector>

#include <taglib/mpegfile.h>
#include <taglib/id3v2frame.h>

#include "id3ted.h"
#include "frameinfo.h"
#include "genericinfo.h"
#include "lametag.h"
#include "pattern.h"

class MP3File {
	public:
		explicit MP3File(const char*, int, bool);
		~MP3File();

		bool isValid() const { return file.isValid(); }
		bool isReadOnly() const { return file.readOnly(); }
		const char* filename() const { return file.name(); }

		bool hasLameTag() const;
		bool hasID3v1Tag() const;
		bool hasID3v2Tag() const;

		void apply(GenericInfo*);
		void apply(FrameInfo*);
		void apply(const MatchInfo&);
		void fill(MatchInfo&);
		void removeFrames(const char*);
		bool save();
		bool strip(int);

		void showInfo() const;
		void printLameTag(bool) const;
		void listID3v1Tag() const;
		void listID3v2Tag(bool) const;

		void extractAPICs(bool) const;

	private:
		MPEG::File file;
		Tag *id3Tag;
		ID3v1::Tag *id3v1Tag;
		ID3v2::Tag *id3v2Tag;
		LameTag *lameTag;
		int tags;

		vector<ID3v2::Frame*> find(FrameInfo*);
};

#endif /* MP3FILE_H */
