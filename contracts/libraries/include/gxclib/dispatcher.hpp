/**
 * @file
 * @copyright defined in gxc/LICENSE
 */

#define GXC_DISPATCH_INTERNAL( r, OP, elem ) \
   case eosio::name( BOOST_PP_STRINGIZE(elem) ).value: \
      eosio::execute_action( eosio::name(receiver), eosio::name(code), &OP::elem ); \
      break;

#define GXC_DISPATCH_HELPER( TYPE,  MEMBERS ) \
   BOOST_PP_SEQ_FOR_EACH( GXC_DISPATCH_INTERNAL, TYPE, MEMBERS )

#define GXC_DISPATCH( TYPE, MEMBERS ) \
extern "C" { \
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
      if( code == receiver ) { \
         switch( action ) { \
            GXC_DISPATCH_HELPER( TYPE, MEMBERS ) \
         } \
         /* does not allow destructor of thiscontract to run: eosio_exit(0); */ \
      } \
   } \
} \

#define GXC_DISPATCH_SAFE( TYPE, MEMBERS ) \
extern "C" { \
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
      if( code == receiver ) { \
         switch( action ) { \
            GXC_DISPATCH_HELPER( TYPE, MEMBERS ) \
            default: \
               auto msg = "action '" + ::eosio::name(action).to_string() + "' not found"; \
               eosio_assert(false, msg.data()); \
               break; \
         } \
         /* does not allow destructor of thiscontract to run: eosio_exit(0); */ \
      } \
   } \
} \

