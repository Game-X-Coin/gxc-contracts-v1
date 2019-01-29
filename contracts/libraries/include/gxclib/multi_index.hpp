/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosiolib/multi_index.hpp>

namespace eosio {
   template <typename T, eosio::name::raw IndexName = name(), typename Extractor = uint64_t>
   class multi_index_item {
   protected:
      T                          _tbl;
      typename T::const_iterator _this;

   public:
      multi_index_item(name receiver, name code, Extractor key)
      : _tbl(receiver, code.value)
      , _this(_tbl.end())
      {
         auto _idx = _tbl.template get_index<IndexName>();
         auto _it  = _idx.find(key);
         if (_it != _idx.end())
            _this = _tbl.find(_it->primary_key());
      }

      bool exists() { return _this != _tbl.end(); }

      inline name get_self()const  { return _tbl.get_code(); }
      inline name get_scope()const { return name(_tbl.get_scope()); }

      const typename T::const_iterator operator->()const { return _this; }
   };

   template <typename T>
   class multi_index_item<T, static_cast<eosio::name::raw>(0)> {
   protected:
      T                          _tbl;
      typename T::const_iterator _this;

   public:
      multi_index_item(name receiver, name code, uint64_t key)
      : _tbl(receiver, code.value)
      , _this(_tbl.find(key))
      {}

      bool exists() { return _this != _tbl.end(); }

      inline name get_self()const  { return _tbl.get_code(); }
      inline name get_scope()const { return name(_tbl.get_scope()); }

      const typename T::const_iterator operator->()const { return _this; }
   };
}
