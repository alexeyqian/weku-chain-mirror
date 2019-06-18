#include <wk/chain_refactory/balance_processor.hpp>

using namespace steemit::chain;

namespace wk{namespace chain{

void balance_processor::adjust_reward_balance( const account_object& a, const asset& delta )
{
   _db.modify( a, [&]( account_object& acnt )
   {
      switch( delta.symbol )
      {
         case STEEM_SYMBOL:
            acnt.reward_steem_balance += delta;
            break;
         case SBD_SYMBOL:
            acnt.reward_sbd_balance += delta;
            break;
         default:
            FC_ASSERT( false, "invalid symbol" );
      }
   });
}

void balance_processor::adjust_savings_balance( const account_object& a, const asset& delta )
{
   _db.modify( a, [&]( account_object& acnt )
   {
      switch( delta.symbol )
      {
         case STEEM_SYMBOL:
            acnt.savings_balance += delta;
            break;
         case SBD_SYMBOL:
            if( a.savings_sbd_seconds_last_update != _db.head_block_time() )
            {
               acnt.savings_sbd_seconds += fc::uint128_t(a.savings_sbd_balance.amount.value) * (_db.head_block_time() - a.savings_sbd_seconds_last_update).to_seconds();
               acnt.savings_sbd_seconds_last_update = _db.head_block_time();

               if( acnt.savings_sbd_seconds > 0 &&
                   (acnt.savings_sbd_seconds_last_update - acnt.savings_sbd_last_interest_payment).to_seconds() > STEEMIT_SBD_INTEREST_COMPOUND_INTERVAL_SEC )
               {
                  auto interest = acnt.savings_sbd_seconds / STEEMIT_SECONDS_PER_YEAR;
                  interest *= _db.get_dynamic_global_properties().sbd_interest_rate;
                  interest /= STEEMIT_100_PERCENT;
                  asset interest_paid(interest.to_uint64(), SBD_SYMBOL);
                  acnt.savings_sbd_balance += interest_paid;
                  acnt.savings_sbd_seconds = 0;
                  acnt.savings_sbd_last_interest_payment = _db.head_block_time();

                  if(interest > 0)
                    _db.push_virtual_operation( interest_operation( a.name, interest_paid ) );

                  _db.modify( _db.get_dynamic_global_properties(), [&]( dynamic_global_property_object& props)
                  {
                     props.current_sbd_supply += interest_paid;
                     props.virtual_supply += interest_paid * _db.get_feed_history().current_median_history;
                  } );
               }
            }
            acnt.savings_sbd_balance += delta;
            break;
         default:
            FC_ASSERT( !"invalid symbol" );
      }
   } );
}

void balance_processor::adjust_balance( const account_object& a, const asset& delta )
{
   _db.modify( a, [&]( account_object& acnt )
   {
      switch( delta.symbol )
      {
         case STEEM_SYMBOL:
            acnt.balance += delta;
            break;
         case SBD_SYMBOL:
            if( a.sbd_seconds_last_update != _db.head_block_time() )
            {
               acnt.sbd_seconds += fc::uint128_t(a.sbd_balance.amount.value) * (_db.head_block_time() - a.sbd_seconds_last_update).to_seconds();
               acnt.sbd_seconds_last_update = _db.head_block_time();

               if( acnt.sbd_seconds > 0 &&
                   (acnt.sbd_seconds_last_update - acnt.sbd_last_interest_payment).to_seconds() > STEEMIT_SBD_INTEREST_COMPOUND_INTERVAL_SEC ) // 30 days
               {
                  auto interest = acnt.sbd_seconds / STEEMIT_SECONDS_PER_YEAR;
                  interest *= _db.get_dynamic_global_properties().sbd_interest_rate;
                  interest /= STEEMIT_100_PERCENT;
                  asset interest_paid(interest.to_uint64(), SBD_SYMBOL);
                  acnt.sbd_balance += interest_paid;
                  acnt.sbd_seconds = 0; // reset sbd_seconds to 0
                  acnt.sbd_last_interest_payment = _db.head_block_time();

                  if(interest > 0)
                     _db.push_virtual_operation( interest_operation( a.name, interest_paid ) );

                  _db.modify( _db.get_dynamic_global_properties(), [&]( dynamic_global_property_object& props)
                  {
                     props.current_sbd_supply += interest_paid;
                     props.virtual_supply += interest_paid * _db.get_feed_history().current_median_history;
                  } );
               }
            }
            acnt.sbd_balance += delta;
            break;
         default:
            FC_ASSERT( false, "invalid symbol" );
      }
   } );
}

}}