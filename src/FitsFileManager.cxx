/** \file FitsFileManager.cxx

    \brief Utilities to help manage FITS files. These classes are not part of the API.

    \author James Peachey, HEASARC
*/

#include <cstdio>
#include <cstring>

#include "FitsFileManager.h"
#include "fitsio.h"
#include "tip/FileSummary.h"
#include "tip/TipException.h"

namespace tip {

  // Create a FITS file optionally using a template. File is closed afterwards.
  void FitsFileManager::createFile(const std::string & file_name, const std::string & template_name, bool clobber) {
    std::string full_name;

    // Handle clobber by prepending a bang or not.
    if (clobber) full_name = "!" + file_name;
    else full_name = file_name;

    // Concatenate the file name with the template name so that cfitsio can use it.
    if (!template_name.empty()) full_name += "(" + template_name + ")";

    // Fitsio stuff.
    fitsfile * fp = 0;
    int status = 0;

    // Create the file.
    fits_create_file(&fp, const_cast<char *>(full_name.c_str()), &status);
    if (0 != status) {
      closeFile(fp, status);
      throw TipException(status, "Unable to create file named \"" + full_name + '"');
    }

    if (template_name.empty()) {
      long dims[1] = { 0 };

      // No template: need to create primary image explicitly.
      fits_create_img(fp, FLOAT_IMG, 0, dims, &status);
      if (0 != status) {
        closeFile(fp, status);
        throw TipException(status, "Unable to create primary image in file named \"" + full_name + '"');
      }
    }

    // Close the file.
    closeFile(fp, status);
    if (0 != status)
      throw TipException(status, "Unable to close newly created file named \"" + file_name + "\"");
  }

  void FitsFileManager::appendImage(const std::string & file_name, const std::string & image_name, const std::vector<long> & dims) {
    fitsfile * fp = 0;
    int status = 0;

    // Open or create the file.
    fits_open_file(&fp, const_cast<char *>(file_name.c_str()), READWRITE, &status);
    if (0 != status) {
      fp = 0;
      status = 0;
      fits_create_file(&fp, const_cast<char *>(file_name.c_str()), &status);
      if (0 != status) {
        closeFile(fp, status);
        throw TipException(status, "Unable to open or create file named \"" + file_name + "\"");
      }
    }

    // Create new image extension at end of file.
    fits_create_img(fp, FLOAT_IMG, dims.size(), const_cast<long *>(&*dims.begin()), &status);
    if (0 != status) {
      closeFile(fp, status);
      throw TipException(status, std::string("Unable to create image named \"") + image_name + "\" in file \"" + file_name + "\"");
    }

    // Get the current extension number, because the primary extension is handled slightly differently.
    int hdu_num = 0;
    fits_get_hdu_num(fp, &hdu_num);
    if (0 != status) {
      closeFile(fp, status);
      throw TipException(status, std::string("Unable to determine the extension number of image \"") + image_name + "\" in file \"" +
        file_name + "\"");
    }

    // For primary array, write name in HDUNAME keyword, otherwise in the EXTNAME keyword.
    char key_name[16];
    if (1 == hdu_num) strcpy(key_name, "HDUNAME");
    else strcpy(key_name, "EXTNAME");
    fits_update_key(fp, TSTRING, key_name, const_cast<char *>(image_name.c_str()), 0, &status);
    if (0 != status) {
      closeFile(fp, status);
      throw TipException(status, std::string("Unable to name image in file \"") + file_name + "\"");
    }

    // Close the file; not interested in it anymore.
    closeFile(fp, status);
    if (0 != status)
      throw TipException(status, "Unable to close appended image named \"" + image_name + "\" in file \"" + file_name + "\"");
  }

  void FitsFileManager::appendTable(const std::string & file_name, const std::string & table_name) {
    fitsfile * fp = 0;
    int status = 0;

    // Open or create the file.
    fits_open_file(&fp, const_cast<char *>(file_name.c_str()), READWRITE, &status);
    if (0 != status) {
      fp = 0;
      status = 0;
      fits_create_file(&fp, const_cast<char *>(file_name.c_str()), &status);
      if (0 != status) {
        closeFile(fp, status);
        throw TipException(status, "Unable to open or create file named \"" + file_name + "\"");
      }
    }

    // Create new table extension at end of file.
    fits_create_tbl(fp, BINARY_TBL, 0, 0, 0, 0, 0, const_cast<char *>(table_name.c_str()), &status);

    // Close the file; not interested in it anymore.
    closeFile(fp, status);

    if (0 != status)
      throw TipException(status, "Unable to create table named \"" + table_name + "\" in file \"" + file_name + "\"");
  }

  void FitsFileManager::getFileSummary(const std::string & file_name, FileSummary & summary) {
    fitsfile * fp = 0;
    int status = 0;

    // Clear out summary.
    summary.clear();

    // Open the file, and complain if it doesn't work:
    fits_open_file(&fp, const_cast<char *>(file_name.c_str()), READONLY, &status);
    if (0 != status)
      throw TipException(status, std::string("Unable to open file named \"") + file_name + "\" with read only access");
    
    // Make sure we scan starting from the first extension, regardless of the full file name used.
    fits_movabs_hdu(fp, 1, 0, &status);
    if (0 != status) {
      closeFile(fp, status);
      throw TipException(status, std::string("Unable to move to primary HDU in file named \"") + file_name);
    }

    // String to hold each extension's identity.
    std::string ext_id;

    while (0 == status) {
      // Get identifier of current extension, and add that info to the summary.
      getExtId(fp, ext_id);
      summary.push_back(ExtSummary(ext_id));

      // Go on to next extension.
      fits_movrel_hdu(fp, 1, 0, &status);
    }
    // Clean up.
    closeFile(fp, status);

    // Flag any condition other than 0 and EOF.
    if (0 != status && END_OF_FILE != status)
      throw TipException(status, std::string("FitsFileManager::getFileSummary had trouble making summary of file ") + file_name);
  }

  bool FitsFileManager::isValid(const std::string & file_name) {
    fitsfile * fp = 0;
    int status = 0;
    fits_open_file(&fp, const_cast<char *>(file_name.c_str()), READONLY, &status);
    if (0 != status) return false;
    closeFile(fp, status);
    return true;
  }

  void FitsFileManager::getExtId(fitsfile * fp, std::string & ext_id) {
    int status = 0;

    // Make a C array to hold the id string.
    char tmp_id[FLEN_CARD] = "";

    // Try EXTNAME keyword first.
    fits_read_key(fp, TSTRING, "EXTNAME", tmp_id, 0, &status);
    if (0 != status) {
      status = 0;
      // If that didn't work, try HDUNAME.
      fits_read_key(fp, TSTRING, "HDUNAME", tmp_id, 0, &status);
      if (0 != status) {
        // Finally, if that didn't work, just use the number, offset by 1.
        int hdu_num = 0;
        fits_get_hdu_num(fp, &hdu_num);
        sprintf(tmp_id, "%d", hdu_num - 1);
      }
    }
    ext_id = tmp_id;
  }

  void FitsFileManager::closeFile(fitsfile *fp, int status) {
    fits_close_file(fp, &status);
  }
}
