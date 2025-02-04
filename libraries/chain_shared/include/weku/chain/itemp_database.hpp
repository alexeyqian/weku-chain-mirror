#pragma once

#include <fc/uint128.hpp>
#include <fc/signals.hpp>
#include <chainbase/chainbase.hpp>

#include <weku/protocol/block.hpp>
#include <weku/protocol/weku_operations.hpp>
#include <weku/protocol/config.hpp>

#include <weku/chain/asset.hpp>
#include <weku/chain/reward.hpp>
#include <weku/chain/uint256.hpp>
#include <weku/chain/weku_object_types.hpp>
#include <weku/chain/account_objects.hpp>
#include <weku/chain/global_property_object.hpp>
#include <weku/chain/hardfork_property_object.hpp>
#include <weku/chain/common_objects.hpp>
#include <weku/chain/comment_objects.hpp>
#include <weku/chain/node_property_object.hpp>
#include <weku/chain/block_summary_object.hpp>
#include <weku/chain/database_exceptions.hpp>
#include <weku/chain/fork_database.hpp>
#include <weku/chain/transaction_object.hpp>
#include <weku/chain/witness_objects.hpp>
#include <weku/chain/compound.hpp>
#include <weku/chain/i_hardforker.hpp>
#include <weku/chain/block_log.hpp>
#include <weku/chain/fork_database.hpp>
#include <weku/chain/hardfork_constants.hpp>
#include <weku/chain/custom_operation_interpreter.hpp>
#include <weku/chain/validation_steps.hpp>

using namespace weku::protocol;

namespace weku{ namespace chain{

// this class is to tempararily used to refactory database
class itemp_database: public chainbase::database 
{
    public:
    virtual ~itemp_database() = default;

    virtual uint32_t head_block_num()const = 0;
    virtual block_id_type head_block_id()const = 0;
    virtual fc::time_point_sec head_block_time()const = 0;

    virtual bool has_hardfork( uint32_t hardfork )const = 0;
    
    virtual bool is_producing() const = 0;
    virtual block_log& get_block_log() = 0; 
    virtual fork_database& fork_db() = 0;

    virtual node_property_object& get_node_properties()  = 0;

    virtual const dynamic_global_property_object&  get_dynamic_global_properties()const = 0;
    virtual const witness_schedule_object&         get_witness_schedule_object()const = 0;
    virtual const hardfork_property_object&        get_hardfork_property_object()const = 0;    
    
    virtual const feed_history_object&             get_feed_history()const = 0; 
    virtual const reward_fund_object& get_reward_fund( const comment_object& c )const = 0; // move    
    
    virtual const account_object&  get_account(  const account_name_type& name )const = 0;
    virtual const account_object*  find_account( const account_name_type& name )const = 0; 
    virtual const witness_object&  get_witness(  const account_name_type& name )const = 0;
    virtual const witness_object*  find_witness( const account_name_type& name )const = 0;
    virtual const comment_object&  get_comment(  const account_name_type& author, const shared_string& permlink )const = 0;
    virtual const comment_object*  find_comment( const account_name_type& author, const shared_string& permlink )const = 0;
    virtual const comment_object&  get_comment(  const account_name_type& author, const string& permlink )const = 0;
    virtual const comment_object*  find_comment( const account_name_type& author, const string& permlink )const = 0;
    virtual const escrow_object&   get_escrow(  const account_name_type& name, uint32_t escrow_id )const = 0;
    
    virtual void apply_block( const signed_block& next_block, uint32_t skip ) = 0;
    virtual const void push_virtual_operation( const operation& op, bool force = false ) = 0; 
    
    virtual void open( const fc::path& data_dir, const fc::path& shared_mem_dir, uint64_t initial_supply = STEEMIT_INIT_SUPPLY, uint64_t shared_file_size = 0, uint32_t chainbase_flags = 0 );
    virtual void close(bool rewind = true);
    virtual void wipe(const fc::path& data_dir, const fc::path& shared_mem_dir, bool include_blocks);  
    virtual void reindex( const fc::path& data_dir, const fc::path& shared_mem_dir, uint64_t shared_file_size = (1024l*1024l*1024l*8l) );

    virtual void adjust_balance( const account_object& a, const asset& delta ) = 0;
    virtual void adjust_proxied_witness_votes( const account_object& a,
                                            const std::array< share_type, STEEMIT_MAX_PROXY_RECURSION_DEPTH+1 >& delta,
                                            int depth = 0) = 0;
    virtual void adjust_proxied_witness_votes( const account_object& a, share_type delta, int depth = 0) = 0;
   
    virtual std::shared_ptr< custom_operation_interpreter > get_custom_json_evaluator( const std::string& id ) = 0;
    
};

}}