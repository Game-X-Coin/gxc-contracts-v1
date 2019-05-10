#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace eosio;
using std::string;

namespace gxc {

class [[eosio::contract("gxc.htlc")]] htlc_contract : public contract {
public:
   using contract::contract;

   struct [[eosio::table]] htlc {
      name contract_name;
      name recipient;
      extended_asset value;
      checksum256 hashlock;
      time_point_sec timelock;
      bool activated = false;

      uint64_t primary_key()const { return contract_name.value; }

      EOSLIB_SERIALIZE(htlc, (contract_name)(recipient)(value)(hashlock)(timelock)(activated))
   };

   typedef multi_index<"htlc"_n, htlc> htlcs;

   // dummy action to resolve cdt v1.6.1 issue on notify handler
   [[eosio::on_notify("eosio.token::transfer")]]
   void on_eos_transfer(name from, name to, asset quantity, string memo) {
      on_transfer(from, to, quantity, memo);
   }

   [[eosio::on_notify("*::transfer")]]
   void on_transfer(name from, name to, asset quantity, string memo);
   typedef action_wrapper<"transfer"_n, &htlc_contract::on_transfer> transfer_action;

   [[eosio::action]]
   void newcontract(name owner, name contract_name, name recipient, extended_asset value, checksum256 hashlock, time_point_sec timelock);

   [[eosio::action]]
   void withdraw(name owner, name contract_name, checksum256 preimage);

   [[eosio::action]]
   void cancel(name owner, name contract_name);
};

}
