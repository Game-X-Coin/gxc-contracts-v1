/**
 *  @file
 *  @copyright defined in gxc/LICENSE
 */ 
#include <gxc.stdtoken/gxc.stdtoken.hpp>

namespace gxc {

void stdtoken::create(name issuer, asset maximum_supply) {
   token::validate(token::account);

   eosio_assert(maximum_supply.symbol.code() != system::core_symbol.code(), "not possible use core symbol");
   eosio_assert(game::is_game(issuer), "issuer should be a game account");

   _check_args(maximum_supply);

   statstable stats(_self, issuer.value);
   auto existing = stats.find(maximum_supply.symbol.code().raw());
   eosio_assert(existing == stats.end(), "token with symbol already exists");

   stats.emplace(_self, [&](auto& s) {
      s.supply.symbol = maximum_supply.symbol;
      s.max_supply    = maximum_supply;
      s.issuer        = issuer;
   });
}


void stdtoken::issue(name to, asset quantity, string memo, name issuer) {
   _check_args(quantity, memo);

   statstable stats(_self, issuer.value);
   const auto& st = stats.get(quantity.symbol.code().raw(), "token with symbol does not exist, create token before issue");

   token::validate(st.issuer);
   eosio_assert(game::is_game(st.issuer), "issuer should be a game account");

   eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
   eosio_assert(quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

   stats.modify(st, same_payer, [&](auto& s) {
      s.supply += quantity;
   });

   add_balance(st.issuer, quantity, st.issuer);

   if (to != st.issuer) {
      action(permission_level(_self, system::active_permission),
         token::account, "transfer"_n, std::make_tuple(st.issuer, to, quantity, memo, st.issuer)
      ).send();
   }
}

void stdtoken::retire(asset quantity, string memo, name issuer) {
   _check_args(quantity, memo);

   statstable stats(_self, issuer.value);
   const auto& st = stats.get(quantity.symbol.code().raw());

   token::validate(st.issuer);
   eosio_assert(game::is_game(st.issuer), "issuer should be a game account");
   eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");

   stats.modify(st, same_payer, [&](auto& s) {
      s.supply -= quantity;
   });

   sub_balance(st.issuer, quantity, issuer);
}

void stdtoken::transfer(name from, name to, asset quantity, string memo, name issuer) {
   _check_args(quantity, memo);

   eosio_assert(from != to, "cannot transfer to self");
   eosio_assert(is_account( to ), "to account does not exist");

   statstable stats(_self, issuer.value);
   const auto& st = stats.get(quantity.symbol.code().raw());

   token::validate();
   bool has_game_auth = (to == st.issuer) && has_auth(st.issuer) && game::is_game(st.issuer);

   eosio_assert(has_game_auth || has_auth(from), "missing required authority");
   eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");

   if (from == st.issuer) {
      sub_balance(from, quantity, issuer);
      add_deposit(to, quantity, issuer);
   } else if (to == st.issuer) {
      sub_deposit(from, quantity, issuer);
      add_balance(to, quantity, issuer);
   } else {
      sub_balance(from, quantity, issuer);
      add_balance(to, quantity, issuer);
   }

   //require_recipient(from);
   //require_recipient(to);
}

void stdtoken::sub_balance(name owner, asset value, name issuer) {
   accountstable from_acnts(_self, owner.value);
   auto extsym = from_acnts.get_index<"extsymbol"_n>();

   const auto& from = extsym.get(token::extended_symbol_code(value.symbol.code(), issuer), "no balance object found");
   eosio_assert(from.balance.amount >= value.amount, "overdrawn balance");

   if (from.balance.amount == value.amount && from.deposit.amount == 0) {
      from_acnts.erase(from);
   } else {
      from_acnts.modify(from, owner, [&](auto& a) {
         a.balance -= value;
      });
   }
}

void stdtoken::add_balance(name owner, asset value, name issuer) {
   accountstable to_acnts(_self, owner.value);
   auto extsym = to_acnts.get_index<"extsymbol"_n>();

   auto to = extsym.find(token::extended_symbol_code(value.symbol.code(), issuer));
   if (to == extsym.end()) {
      to_acnts.emplace(_self, [&](auto& a) {
         a.id = to_acnts.available_primary_key();
         a.balance = value;
         a.deposit.symbol = value.symbol;
         a.issuer = issuer;
      });
   } else {
      extsym.modify(to, same_payer, [&](auto& a) {
         a.balance += value;
      });
   }
}

void stdtoken::sub_deposit(name owner, asset value, name issuer) {
   accountstable from_acnts(_self, owner.value);
   auto extsym = from_acnts.get_index<"extsymbol"_n>();

   const auto& from = extsym.get(token::extended_symbol_code(value.symbol.code(), issuer), "no deposit object found");
   eosio_assert(from.deposit.amount >= value.amount, "overdrawn deposit");

   if (from.deposit.amount == value.amount && from.balance.amount == 0) {
      from_acnts.erase(from);
   } else {
      from_acnts.modify(from, owner, [&](auto& a) {
         a.deposit -= value;
      });
   }
}

void stdtoken::add_deposit(name owner, asset value, name issuer) {
   accountstable to_acnts(_self, owner.value);
   auto extsym = to_acnts.get_index<"extsymbol"_n>();

   auto to = extsym.find(token::extended_symbol_code(value.symbol.code(), issuer));
   if (to == extsym.end()) {
      to_acnts.emplace(_self, [&](auto& a) {
         a.id = to_acnts.available_primary_key();
         a.balance.symbol = value.symbol;
         a.deposit = value;
         a.issuer = issuer;
      });
   } else {
      extsym.modify(to, same_payer, [&](auto& a) {
         a.deposit += value;
      });
   }
}

void stdtoken::refresh_drawout(withdrawtable& witd, name owner, time_point_sec ctp) {
   auto reqtime = witd.get_index<"reqtime"_n>();
   auto itr = reqtime.begin();

   if (itr != reqtime.end()) {
      auto& next = *itr;

      transaction out;
      out.actions.emplace_back(action{{_self, system::active_permission}, _self, "drawout"_n, owner});
      if (next.request_time == ctp) {
         out.delay_sec = withdraw_delay_sec;
      } else {
         auto timeleft = seconds(withdraw_delay_sec) - (ctp - next.request_time);
         out.delay_sec = static_cast<uint32_t>(timeleft.to_seconds());
      }
      cancel_deferred(owner.value);
      out.send(owner.value, owner, true);
   } else {
      cancel_deferred(owner.value);
   }
}

void stdtoken::deposit(name owner, asset quantity, name issuer) {
   _check_args(quantity);

   withdrawtable witd(_self, owner.value);
   auto extsym = witd.get_index<"extsymbol"_n>();
   auto req = extsym.find(token::extended_symbol_code(quantity.symbol.code(), issuer));

   if (req == extsym.end()) {
      require_auth(owner);
      sub_balance(owner, quantity, issuer);
   } else {
      auto leftover = req->quantity - quantity;
      eosio_assert((leftover.amount >= 0 && has_auth(issuer)) || has_auth(owner), "missing required authority");
      if (leftover.amount > 0) {
         extsym.modify(req, same_payer, [&](auto& w) {
            w.quantity -= quantity;
         });
         sub_balance(_self, quantity, issuer);
      } else {
         sub_balance(_self, req->quantity, issuer);
         if (leftover.amount) {
            sub_balance(owner, -leftover, issuer);
         }
         extsym.erase(req);
         refresh_drawout(witd, owner);
      }
   }

   add_deposit(owner, quantity, issuer);
}

void stdtoken::withdraw(name owner, asset quantity, name issuer) {
   _check_args(quantity);

   require_auth(owner);

   withdrawtable witd(_self, owner.value);
   auto extsym = witd.get_index<"extsymbol"_n>();
   auto req = extsym.find(token::extended_symbol_code(quantity.symbol.code(), issuer));
   auto ctp = current_time_point();

   if (req != extsym.end()) {
      extsym.modify(req, same_payer, [&](auto& w) {
         w.request_time = ctp;
         w.quantity += quantity;
      });
   } else {
      witd.emplace(owner, [&](auto& w) {
         w.id = witd.available_primary_key();
         w.request_time = ctp;
         w.quantity = quantity;
         w.issuer = issuer;
      });
   }

   sub_deposit(owner, quantity, issuer);
   add_balance(_self, quantity, issuer);

   refresh_drawout(witd, owner, ctp);
}

void stdtoken::drawout(name owner) {
   require_auth(_self);

   withdrawtable witd(_self, owner.value);
   auto reqtime = witd.get_index<"reqtime"_n>();
   auto req = reqtime.begin();

   eosio_assert(req != reqtime.end(), "withdraw request not found");

   for ( ; req != reqtime.end(); req = reqtime.begin()) {
      if (req->request_time + seconds(withdraw_delay_sec) > current_time_point()) break;

      sub_balance(_self, req->quantity, req->issuer);
      add_balance(owner, req->quantity, req->issuer);

      reqtime.erase(req);
   }

   refresh_drawout(witd, owner);
}

}

GXC_DISPATCH_SAFE(gxc::stdtoken, (create)(issue)(transfer)(retire)(deposit)(withdraw)(drawout))
