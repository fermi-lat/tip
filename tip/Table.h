/** \file Table.h

    \brief High level encapsulation of tabular data.

    \author James Peachey, HEASARC
*/
#ifndef table_Table_h
#define table_Table_h

#include <string>

#include "table/Extension.h"
#include "table/Iterator.h"
#include "table/Ref.h"
#include "table/TableException.h"
#include "table/TableRecord.h"
#include "table/table_types.h"
#include "table/VectorAdaptor.h"

namespace table {

  class ITabularData;

  /** \class Table

      \brief High level encapsulation of tabular data, viewed as a container of Records. Access is mainly
      attained through STL-style iterators.
  */
  class Table : public Extension {
    public:
      /** \brief Helper type: encapsulation of a cell of data in this table.
      */
      typedef TableCell Cell;

      /** \brief Helper type: encapsulation of a record, containing cells of data in this table.
      */
      typedef TableRecord Record;

      /** \brief Helper type: table iterator.
      */
      typedef RandomAccessIterator<TableRecord, IndexDiff_t> Iterator;

      /** \brief Helper type: auxilliary Cell access through a type which behaves like a primitive,
          but is connected to the table cell.
      */
      template <typename T>
      class Ref : public ReferenceAdaptor<T, Cell> {
        public:
          Ref(Cell & cell): ReferenceAdaptor<T, Cell>(cell) {}
          Ref & operator =(const T & data) { ReferenceAdaptor<T, Cell>::operator =(data); return *this; }
      };

      /** \brief Helper type: auxilliary Cell access through a type which behaves like a primitive,
          but is connected to the table cell.
      */
      template <typename T>
      class Vector : public VectorAdaptor<T, Cell> {
        public:
          Vector(Cell & cell): VectorAdaptor<T, Cell>(cell) {}
      };

      Table(ITabularData * tab_data): Extension(tab_data), m_tab_data(tab_data) {}

      virtual ~Table() { delete m_tab_data; }

      /** \brief Return an iterator pointing to the first record in the table.
      */
      Iterator begin() { return Iterator(Record(m_tab_data, 0)); }

      /** \brief Return an iterator pointing past the last record in the table.
      */
      Iterator end() { return Iterator(Record(m_tab_data, m_tab_data->getNumRecords())); }

      /** \brief Return the number of records contained in this table.
      */
      Index_t getNumRecords() const { return m_tab_data->getNumRecords(); }

    private:
      ITabularData * m_tab_data;
  };

}

#endif
