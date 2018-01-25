#ifndef MSAT_XRIT_FILEACCESS_H
#define MSAT_XRIT_FILEACCESS_H

/*
 * xrit/fileaccess - Access the various components of a XRIT image
 *
 * Copyright (C) 2007  ARPA-SIM <urpsim@smr.arpa.emr.it>                                           
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 * Author: Enrico Zini <enrico@enricozini.org>                                                     
 */

#include <string>
#include <vector>

//namespace msat {
namespace xrit {

/// Return true if the file name looks like a valid XRIT 'pathname'
bool isValid(const std::string& filename);

/**
 * Compute the names of the various file parts of the XRIT data
 */
struct FileAccess
{
	std::string directory;
	std::string resolution;
	std::string productid1;
	std::string productid2;
	std::string timing;

    FileAccess() : directory(".") {}
    FileAccess(const std::string& filename);
    FileAccess(const FileAccess& fa, const std::string& chan);

    /// Initialize parsing a file name
    void parse(const std::string& filename);

    /// Initialize as a different channel of an existing FileAccess
    void parse(const FileAccess& fa, const std::string& chan);

	void ensureComplete() const;

	std::string prologueFile() const;
	std::string epilogueFile() const;
	std::vector<std::string> segmentFiles() const;

	std::string toString() const;
};

}
//}

#endif
