#pragma once

#include <eosio/multi_index.hpp>

namespace eosio {
   template <typename T, eosio::name::raw IndexName = name(), typename Extractor = uint64_t>
   class multi_index_wrapper {
   protected:
      T                          _tbl;
      typename T::const_iterator _this;

   public:
      multi_index_wrapper(name code, name scope,
                       Extractor key = _multi_index_detail::secondary_key_traits<Extractor>::true_lowest())
      : _tbl(code, scope.value)
      , _this(_tbl.end())
      {
         auto _idx = _tbl.template get_index<IndexName>();
         auto _it  = _idx.find(key);
         if (_it != _idx.end())
            _this = _tbl.find(_it->primary_key());
      }

      const T& table()const { return _tbl; }
      auto index()const { return _tbl.template get_index<IndexName>(); }

      template <eosio::name::raw SecondaryIndex>
      auto get_index() { return _tbl.template get_index<SecondaryIndex>(); }

      bool exists()const { return _this != _tbl.end(); }
      operator bool()const { return exists(); }

      inline name code()const  { return _tbl.get_code(); }
      inline name scope()const { return name(_tbl.get_scope()); }

      const typename T::const_iterator operator->()const { return _this; }

      multi_index_wrapper operator++()    {
         auto _idx = index();
         auto _it = _idx.iterator_to(*_this);
         if (++_it != _idx.end())
            _this = _tbl.find(_it->primary_key());
         else
            _this = _tbl.end();
         return (*this);
      }
      multi_index_wrapper operator++(int) { return ++(*this); }
      multi_index_wrapper operator--()    {
         auto _idx = index();
         auto _it = _idx.iterator_to(*_this);
         if (--_it != _idx.end())
            _this = _tbl.find(_it->primary_key());
         return (*this);
      }
      multi_index_wrapper operator--(int) { return --(*this); }

      template<typename Lambda>
      void emplace(name payer, Lambda&& updater) {
         _this = _tbl.emplace(payer, std::forward<Lambda&&>(updater));
      }

      template<typename Lambda>
      void modify(name payer, Lambda&& updater) {
         _tbl.modify(_this, payer, std::forward<Lambda&&>(updater));
      }

      void erase() { _this = _tbl.erase(_this); }
   };

   template <typename T>
   class multi_index_wrapper<T> {
   protected:
      T                          _tbl;
      typename T::const_iterator _this;

   public:
      multi_index_wrapper(name code, name scope, uint64_t key = std::numeric_limits<uint64_t>::lowest())
      : _tbl(code, scope.value)
      , _this(_tbl.find(key))
      {}

      const T& table()const { return _tbl; }

      template <eosio::name::raw SecondaryIndex>
      auto get_index() { return _tbl.template get_index<SecondaryIndex>(); }

      bool exists()const { return _this != _tbl.end(); }
      operator bool()const { return exists(); }

      inline name code()const  { return _tbl.get_code(); }
      inline name scope()const { return name(_tbl.get_scope()); }

      const typename T::const_iterator operator->()const { return _this; }

      multi_index_wrapper operator++()    { ++_this; return (*this); }
      multi_index_wrapper operator++(int) { return ++(*this); }
      multi_index_wrapper operator--()    { --_this; return (*this); }
      multi_index_wrapper operator--(int) { return --(*this); }

      template<typename Lambda>
      void emplace(name payer, Lambda&& updater) {
         _this = _tbl.emplace(payer, std::forward<Lambda&&>(updater));
      }

      template<typename Lambda>
      void modify(name payer, Lambda&& updater) {
         _tbl.modify(_this, payer, std::forward<Lambda&&>(updater));
      }

      void erase() { _this = _tbl.erase(_this); }
   };
}
