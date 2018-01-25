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

#include <xrit/fileaccess.h>
#include <glob.h>
#include <stdexcept>
#include <sstream>

using namespace std;

#define PATH_SEPARATOR "/"
// For windows use #define PATH_SEPARATOR "\\"

//namespace msat {
namespace xrit {

// Pad the string 'base' with trailing underscores to ensure it's at least
// final_len characters long
static std::string underscoreit(const std::string& base, int final_len)
{
	string res = base;
	res.resize(final_len, '_');
	return res;
}

static std::string deunderscore(const std::string& str)
{
    size_t pos = str.size() - 1;
    while (pos > 0 && str[pos] == '_')
        --pos;
    return str.substr(0, pos + 1);
}

// dir/res:prodid1:prodid2:time
bool isValid(const std::string& filename)
{
    if (filename.size() < 3) return false;

    // Segments
    if (filename.substr(filename.size() - 3) == "-C_")
        return true;

	// check that it contains at least 3 ':' signs
	size_t pos = 0;
	for (int i = 0; i < 3; ++i, ++pos)
		if ((pos = filename.find(':', pos)) == string::npos)
			return false;
	return true;
}

// dir/res:prodid1:prodid2:time
FileAccess::FileAccess(const std::string& filename)
{
	parse(filename);
}

FileAccess::FileAccess(const FileAccess& fa, const std::string& chan)
{
    parse(fa, chan);
}

void FileAccess::parse(const FileAccess& fa, const std::string& chan)
{
    directory = fa.directory;
    resolution = fa.resolution;
    productid1 = fa.productid1;
    productid2 = chan;
    timing = fa.timing;
}

void FileAccess::parse(const std::string& filename)
{
    // Split directory name
	size_t beg;
	size_t end = filename.rfind('/');
	if (end == string::npos)
	{
		directory = ".";
		beg = 0;
	}
	else
	{
		directory = filename.substr(0, end);
		if (directory.size() == 0) directory = "/";
		beg = end + 1;
	}

    if (filename.size() < 3)
        throw std::runtime_error(filename + " is not a valid xRIT file name");

    if (filename.substr(filename.size() - 3) == "-C_")
    {
        // Parse segment filename [directory/]H-000-MSG1__-MSG1________-HRV______-000018___-200611141200-C_
        if ((end = filename.find('-', beg)) == string::npos)
            throw std::runtime_error("XRIT segment " + filename + " is not in the form [directory/]resolution-nnn-xxxxxx-productid1-productid2-datetime-C_");
        resolution = filename.substr(beg, end-beg);

        // Skip first number
        beg = end + 1;
        if ((end = filename.find('-', beg)) == string::npos)
            throw std::runtime_error("XRIT segment " + filename + " is not in the form [directory/]resolution-nnn-xxxxxx-productid1-productid2-datetime-C_");

        // Skip string (satellite name?)
        beg = end + 1;
        if ((end = filename.find('-', beg)) == string::npos)
            throw std::runtime_error("XRIT segment " + filename + " is not in the form [directory/]resolution-nnn-xxxxxx-productid1-productid2-datetime-C_");

        beg = end + 1;
        if ((end = filename.find('-', beg)) == string::npos)
            throw std::runtime_error("XRIT segment " + filename + " is not in the form [directory/]resolution-nnn-xxxxxx-productid1-productid2-datetime-C_");
        productid1 = deunderscore(filename.substr(beg, end-beg));

        beg = end + 1;
        if ((end = filename.find('-', beg)) == string::npos)
            throw std::runtime_error("XRIT segment " + filename + " is not in the form [directory/]resolution-nnn-xxxxxx-productid1-productid2-datetime-C_");
        productid2 = deunderscore(filename.substr(beg, end-beg));

        // Skip segment number
        beg = end + 1;
        if ((end = filename.find('-', beg)) == string::npos)
            throw std::runtime_error("XRIT segment " + filename + " is not in the form [directory/]resolution-nnn-xxxxxx-productid1-productid2-datetime-C_");

        beg = end + 1;
        if ((end = filename.find('-', beg)) == string::npos)
            throw std::runtime_error("XRIT segment " + filename + " is not in the form [directory/]resolution-nnn-xxxxxx-productid1-productid2-datetime-C_");
        timing = deunderscore(filename.substr(beg, end-beg));
    } else {
        // Parse shortened form [directory/]resolution:productid1:productid2:datetime
        if ((end = filename.find(':', beg)) == string::npos)
            throw std::runtime_error("XRIT name " + filename + " is not in the form [directory/]resolution:productid1:productid2:datetime");
        resolution = filename.substr(beg, end-beg);

        beg = end + 1;
        if ((end = filename.find(':', beg)) == string::npos)
            throw std::runtime_error("XRIT name " + filename + " is not in the form [directory/]resolution:productid1:productid2:datetime");
        productid1 = filename.substr(beg, end-beg);

        beg = end + 1;
        if ((end = filename.find(':', beg)) == string::npos)
            throw std::runtime_error("XRIT name " + filename + " is not in the form [directory/]resolution:productid1:productid2:datetime");
        productid2 = filename.substr(beg, end-beg);

        beg = end + 1;
        timing = filename.substr(beg);
    }
}

void FileAccess::ensureComplete() const
{
	if (directory.empty())
		throw std::runtime_error("source directory is missing");
	if (resolution.empty())
		throw std::runtime_error("resolution is missing");
	if (productid1.empty())
		throw std::runtime_error("first product ID is missing");
	if (productid2.empty())
		throw std::runtime_error("second product ID is missing");
	if (timing.empty())
		throw std::runtime_error("timing is missing");
}

std::string FileAccess::prologueFile() const
{
  std::string filename = directory
		       + PATH_SEPARATOR
					 + resolution
		       + "-" "???" "-" "??????" "-" // Split to avoid warnings on trigraphs
					 + underscoreit(productid1, 12) + "-"
					 + underscoreit("_", 9) + "-"
					 + "PRO______-"
					 + timing
					 + "-__";

  glob_t globbuf;
  globbuf.gl_offs = 1;

  if ((glob(filename.c_str(), GLOB_DOOFFS, NULL, &globbuf)) != 0)
    throw std::runtime_error("No such file(s)");

  if (globbuf.gl_pathc > 1)
    throw std::runtime_error("Non univoque prologue file.... Do not trust calibration.");

	string res(globbuf.gl_pathv[1]);
  globfree(&globbuf);
  return res;
}

std::string FileAccess::epilogueFile() const
{
  std::string filename = directory
		       + PATH_SEPARATOR
					 + resolution
		       + "-???" "-??????" "-"  // Split to avoid warnings on trigraphs
					 + underscoreit(productid1, 12) + "-"
					 + underscoreit("_", 9) + "-"
					 + "EPI______-"
					 + timing
					 + "-__";

  glob_t globbuf;
  globbuf.gl_offs = 1;

  if ((glob(filename.c_str(), GLOB_DOOFFS, NULL, &globbuf)) != 0)
    throw std::runtime_error("No such file(s)");

  if (globbuf.gl_pathc > 1)
    throw std::runtime_error("Non univoque prologue file.... Do not trust calibration.");

	string res(globbuf.gl_pathv[1]);
  globfree(&globbuf);
  return res;
}

std::vector<std::string> FileAccess::segmentFiles() const
{
  string filename = directory
					 + PATH_SEPARATOR
					 + resolution
           + "-???" "-??????" "-"	// Split to avoid warnings on trigraphs
           + underscoreit(productid1, 12) + "-"
           + underscoreit(productid2, 9) + "-"
           + "0?????___" + "-"
           + timing + "-" + "C_";

  glob_t globbuf;
  globbuf.gl_offs = 1;

  if ((glob(filename.c_str( ), GLOB_DOOFFS, NULL, &globbuf)) != 0)
    throw std::runtime_error("No such file(s)");

	std::vector<std::string> res;
	for (size_t i = 0; i < globbuf.gl_pathc; ++i)
		res.push_back(globbuf.gl_pathv[i+1]);
  globfree(&globbuf);
	return res;
}

std::string FileAccess::toString() const
{
	std::stringstream str;
	str <<        "dir: " << directory
		  <<       " res: " << resolution
			<<     " prod1: " << productid1
			<<     " prod2: " << productid2
			<<      " time: " << timing;
	return str.str();
}

}
//}
