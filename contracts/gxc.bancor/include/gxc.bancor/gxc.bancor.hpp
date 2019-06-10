#include <eosio/singleton.hpp>
#include "connector.hpp"

using namespace eosio;
using std::string;

namespace gxc {

class [[eosio::contract("gxc.bancor")]] bancor_contract : public contract {
public:
   using contract::contract;

   struct [[eosio::table]] connector : eosio::connector {
   };

   struct base_config {
      asset connected; // if amount != 0, it's considered as fixed amount of conversion fee
      uint16_t rate; // permyriad

      bool is_exempted()const { return connected.amount == 0 && rate == 0; }
      extended_asset get_fee(extended_asset value)const {
         int64_t fee = 0;
         if (rate != 0) {
            int64_t p = 10000 / rate;
            fee = (value.quantity.amount + p - 1) / p;
         }
         fee += connected.amount;
         return {fee, value.get_extended_symbol()};
      }

      EOSLIB_SERIALIZE(base_config, (connected)(rate))
   };

   struct [[eosio::table]] charge_policy : base_config {
      extended_symbol smart;

      uint64_t primary_key() const { return smart.get_symbol().code().raw(); }

      EOSLIB_SERIALIZE_DERIVED(charge_policy, base_config, (smart))
   };

   struct [[eosio::table]] config : base_config {
      name owner;
      name connected_contract;

      extended_asset get_connected()const { return { connected, connected_contract }; }
      extended_symbol get_connected_symbol()const { return { connected.symbol, connected_contract }; }

      EOSLIB_SERIALIZE_DERIVED(config, base_config, (owner)(connected_contract))
   };

   typedef multi_index<"connector"_n, connector> connectors;
   typedef multi_index<"charge"_n, charge_policy> charges;
   typedef singleton<"config"_n, config> configuration;

   [[eosio::action]]
   void convert(name sender, extended_asset from, extended_asset to);

   [[eosio::action]]
   void init(name owner, extended_symbol connected);

   [[eosio::action]]
   void connect(extended_symbol smart, extended_asset balance, double weight);

   [[eosio::action]]
   void setcharge(int16_t rate, std::optional<extended_asset> fixed, std::optional<extended_symbol> smart);

   [[eosio::action]]
   void setowner(name owner);
};

} /// namespace gxc
