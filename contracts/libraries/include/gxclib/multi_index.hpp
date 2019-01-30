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
      multi_index_item(name receiver, name code,
                       Extractor key = _multi_index_detail::secondary_key_traits<Extractor>::true_lowest())
      : _tbl(receiver, code.value)
      , _this(_tbl.end())
      {
         auto _idx = _tbl.template get_index<IndexName>();
         auto _it  = _idx.find(key);
         if (_it != _idx.end())
            _this = _tbl.find(_it->primary_key());
      }

      const T& table()const { return _tbl; }
      auto index() { return _tbl.template get_index<IndexName>(); }

      bool exists() { return _this != _tbl.end(); }

      inline name self()const  { return _tbl.get_code(); }
      inline name scope()const { return name(_tbl.get_scope()); }

      const typename T::const_iterator operator->()const { return _this; }

      multi_index_item operator++()    {
         auto _idx = index();
         auto _it = _idx.iterator_to(*_this);
         if (++_it != _idx.end())
            _this = _tbl.find(_it->primary_key());
         return (*this);
      }
      multi_index_item operator++(int) { return ++(*this); }
      multi_index_item operator--()    {
         auto _idx = index();
         auto _it = _idx.iterator_to(*_this);
         if (--_it != _idx.end())
            _this = _tbl.find(_it->primary_key());
         return (*this);
      }
      multi_index_item operator--(int) { return --(*this); }

      template<typename Lambda>
      void emplace(name payer, Lambda&& updater) {
         _this = _tbl.emplace(payer, std::forward<Lambda&&>(updater));
      }

      template<typename Lambda>
      void modify(name payer, Lambda&& updater) {
         _tbl.modify(_this, payer, std::forward<Lambda&&>(updater));
      }

      void erase() { _tbl.erase(_this); }
   };

   template <typename T>
   class multi_index_item<T, static_cast<eosio::name::raw>(0)> {
   protected:
      T                          _tbl;
      typename T::const_iterator _this;

   public:
      multi_index_item(name receiver, name code, uint64_t key = std::numeric_limits<uint64_t>::lowest())
      : _tbl(receiver, code.value)
      , _this(_tbl.find(key))
      {}

      const T& table()const { return _tbl; }

      bool exists() { return _this != _tbl.end(); }

      inline name self()const  { return _tbl.get_code(); }
      inline name scope()const { return name(_tbl.get_scope()); }

      const typename T::const_iterator operator->()const { return _this; }

      multi_index_item operator++()    { ++_this; return (*this); }
      multi_index_item operator++(int) { return ++(*this); }
      multi_index_item operator--()    { --_this; return (*this); }
      multi_index_item operator--(int) { return --(*this); }

      template<typename Lambda>
      void emplace(name payer, Lambda&& updater) {
         _this = _tbl.emplace(payer, std::forward<Lambda&&>(updater));
      }

      template<typename Lambda>
      void modify(Lambda&& updater) {
         _tbl.modify(_this, same_payer, std::forward<Lambda&&>(updater));
      }

      void erase() { _tbl.erase(_this); }
   };
}
