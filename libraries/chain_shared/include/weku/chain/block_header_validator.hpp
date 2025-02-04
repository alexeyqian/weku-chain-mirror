#pragma once

#include <weku/chain/itemp_database.hpp>

namespace weku{namespace chain{

class block_header_validator{
    public:
        block_header_validator(itemp_database& db):_db(db){}
        const witness_object& validate_block_header( uint32_t skip, const signed_block& next_block )const;
    private:
        itemp_database& _db;
};

}}