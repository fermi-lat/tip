/** \file Extension.h

    \brief High level encapsulation of a Fits-like file extension.

    \author James Peachey, HEASARC
*/
#ifndef table_Extension_h
#define table_Extension_h

#include <string>

#include "table/Header.h"
#include "table/table_types.h"

namespace table {

  class IHeaderData;

  /** \class Extension

      \brief High level encapsulation of a Fits-like file extension.
  */
  class Extension {
    public:

      Extension(IHeaderData * header_data): m_header(header_data) {}

      virtual ~Extension() {}

      Header getHeader() { return m_header; }

    private:
      Header m_header;
  };

}

#endif
