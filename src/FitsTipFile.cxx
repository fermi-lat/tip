#include "FitsTipFile.h"

#include "fitsio.h"

#include "tip/Extension.h"
#include "tip/IFileSvc.h"
#include "tip/Image.h"
#include "tip/Table.h"
#include "tip/TipException.h"

namespace tip {

  FitsTipFile::FitsTipFile(const std::string & file_name): ITipFile(file_name), m_fp(0) {
    openFile();
  }

  FitsTipFile::FitsTipFile(const std::string & file_name, const std::string & template_name, bool clobber): ITipFile(file_name),
    m_fp(0) {
    std::string full_name;

    // Handle clobber by prepending a bang or not.
    if (clobber) full_name = "!" + file_name;
    else full_name = file_name;

    // Concatenate the file name with the template name so that cfitsio can use it.
    if (!template_name.empty()) full_name += "(" + template_name + ")";

    // Fitsio stuff.
    int status = 0;

    // Create the file.
    fits_create_file(&m_fp, const_cast<char *>(full_name.c_str()), &status);
    if (0 != status) {
      closeFile(status);
      throw TipException(status, "Unable to create file named \"" + full_name + '"');
    }

    if (template_name.empty()) {
      long dims[1] = { 0 };

      // No template: need to create primary image explicitly.
      fits_create_img(m_fp, FLOAT_IMG, 0, dims, &status);
      if (0 != status) {
        closeFile(status);
        throw TipException(status, "Unable to create primary image in file named \"" + full_name + '"');
      }
    }
  }

  FitsTipFile::FitsTipFile(const FitsTipFile & file): ITipFile(file.getName()), m_fp(0) {
    openFile();
  }

  FitsTipFile::~FitsTipFile() {
    closeFile(0);
  }

  // TODO: Turn this around: IFileSvc should call FitsTipFile not vice versa.
  Extension * FitsTipFile::editExtension(const std::string & ext_name) {
    return IFileSvc::instance().editExtension(getName(), ext_name);
  }

  Image * FitsTipFile::editImage(const std::string & image_name) {
    return IFileSvc::instance().editImage(getName(), image_name);
  }

  Table * FitsTipFile::editTable(const std::string & table_name) {
    return IFileSvc::instance().editTable(getName(), table_name);
  }

  ITipFile * FitsTipFile::clone() const { return new FitsTipFile(*this); }

  void FitsTipFile::openFile() {
    int status = 0;
    fits_open_file(&m_fp, const_cast<char *>(getName().c_str()), READWRITE, &status);
    if (0 != status) {
      status = 0;
      fits_open_file(&m_fp, const_cast<char *>(getName().c_str()), READONLY, &status);

      if (0 != status)
        throw TipException(status, "FitsTipFile::openFile could not open " + getName() + " either read/write or read-only");
    }
  }

  void FitsTipFile::closeFile(int status) {
    fits_close_file(m_fp, &status);
    m_fp = 0;
  }

}
