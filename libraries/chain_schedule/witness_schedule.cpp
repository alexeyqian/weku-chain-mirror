#include <weku/chain/witness_schedule.hpp>
#include <weku/protocol/config.hpp>
#include <weku/chain/witness_objects.hpp>

namespace weku { namespace chain {

        void reset_virtual_schedule_time(itemp_database &db) {
            const witness_schedule_object &wso = db.get_witness_schedule_object();
            db.modify(wso, [&](witness_schedule_object &o) {
                o.current_virtual_time = fc::uint128(); // reset it 0
            });

            const auto &idx = db.get_index<witness_index>().indices();
            for (const auto &witness : idx) {
                db.modify(witness, [&](witness_object &wobj) {
                    wobj.virtual_position = fc::uint128();
                    wobj.virtual_last_update = wso.current_virtual_time;
                    wobj.virtual_scheduled_time = VIRTUAL_SCHEDULE_LAP_LENGTH2 / (wobj.votes.value + 1);
                });
            }
        }

        void update_median_witness_props(itemp_database &db) {
            const witness_schedule_object &wso = db.get_witness_schedule_object();

            /// fetch all witness objects
            vector<const witness_object *> active;
            active.reserve(wso.num_scheduled_witnesses);
            for (int i = 0; i < wso.num_scheduled_witnesses; i++) {
                active.push_back(&db.get_witness(wso.current_shuffled_witnesses[i]));
            }

            /// sort them by account_creation_fee
            std::sort(active.begin(), active.end(), [&](const witness_object *a, const witness_object *b) {
                return a->props.account_creation_fee.amount < b->props.account_creation_fee.amount;
            });
            asset median_account_creation_fee = active[active.size() / 2]->props.account_creation_fee;

            /// sort them by maximum_block_size
            std::sort(active.begin(), active.end(), [&](const witness_object *a, const witness_object *b) {
                return a->props.maximum_block_size < b->props.maximum_block_size;
            });
            uint32_t median_maximum_block_size = active[active.size() / 2]->props.maximum_block_size;

            /// sort them by sbd_interest_rate
            std::sort(active.begin(), active.end(), [&](const witness_object *a, const witness_object *b) {
                return a->props.sbd_interest_rate < b->props.sbd_interest_rate;
            });
            uint16_t median_sbd_interest_rate = active[active.size() / 2]->props.sbd_interest_rate;

            db.modify(wso, [&](witness_schedule_object &_wso) {
                _wso.median_props.account_creation_fee = median_account_creation_fee;
                _wso.median_props.maximum_block_size = median_maximum_block_size;
                _wso.median_props.sbd_interest_rate = median_sbd_interest_rate;
            });

            db.modify(db.get_dynamic_global_properties(), [&](dynamic_global_property_object &_dgpo) {
                _dgpo.maximum_block_size = median_maximum_block_size;
                _dgpo.sbd_interest_rate = median_sbd_interest_rate;
            });
        }

        void update_witness_schedule(itemp_database &db) {
            if ((db.head_block_num() % STEEMIT_MAX_WITNESSES) != 0) return;

            const witness_schedule_object &wso = db.get_witness_schedule_object();
            vector<account_name_type> active_witnesses;
            active_witnesses.reserve(STEEMIT_MAX_WITNESSES);

            /// Add the highest voted witnesses
            flat_set<witness_id_type> selected_voted;
            selected_voted.reserve(wso.max_voted_witnesses);

            const auto &widx = db.get_index<witness_index>().indices().get<by_vote_name>();
            for (auto itr = widx.begin();
                 itr != widx.end() && selected_voted.size() < wso.max_voted_witnesses;
                 ++itr) {
                if (db.has_hardfork(STEEMIT_HARDFORK_0_14) && (itr->signing_key == public_key_type()))
                    continue;
                selected_voted.insert(itr->id);
                active_witnesses.push_back(itr->owner);
                db.modify(*itr, [&](witness_object &wo) { wo.schedule = witness_object::top19; });
            }

            auto num_elected = active_witnesses.size();

            // TODO: looks like mining is disabled after hardfork 17, we might need to remove below logic
            /// Add miners from the top of the mining queue
            flat_set<witness_id_type> selected_miners;
            selected_miners.reserve(wso.max_miner_witnesses);
            const auto &gprops = db.get_dynamic_global_properties();
            const auto &pow_idx = db.get_index<witness_index>().indices().get<by_pow>();
            auto mitr = pow_idx.upper_bound(0);
            while (mitr != pow_idx.end() && selected_miners.size() < wso.max_miner_witnesses) {
                // Only consider a miner who is not a top voted witness
                if (selected_voted.find(mitr->id) == selected_voted.end()) {
                    // Only consider a miner who has a valid block signing key
                    if (!(db.has_hardfork(STEEMIT_HARDFORK_0_14) &&
                          db.get_witness(mitr->owner).signing_key == public_key_type())) {
                        selected_miners.insert(mitr->id);
                        active_witnesses.push_back(mitr->owner);
                        db.modify(*mitr, [&](witness_object &wo) { wo.schedule = witness_object::miner; });
                    }
                }
                // Remove processed miner from the queue
                auto itr = mitr;
                ++mitr;
                db.modify(*itr, [&](witness_object &wit) {
                    wit.pow_worker = 0;
                });
                db.modify(gprops, [&](dynamic_global_property_object &obj) {
                    obj.num_pow_witnesses--;
                });
            }

            auto num_miners = selected_miners.size();

            /// Add the running witnesses in the lead
            fc::uint128 new_virtual_time = wso.current_virtual_time;
            const auto &schedule_idx = db.get_index<witness_index>().indices().get<by_schedule_time>();
            auto sitr = schedule_idx.begin();
            vector<decltype(sitr)> processed_witnesses;
            for (auto witness_count = selected_voted.size() + selected_miners.size();
                 sitr != schedule_idx.end() && witness_count < STEEMIT_MAX_WITNESSES;
                 ++sitr) {
                new_virtual_time = sitr->virtual_scheduled_time; /// everyone advances to at least this time
                processed_witnesses.push_back(sitr);

                if (db.has_hardfork(STEEMIT_HARDFORK_0_14) && sitr->signing_key == public_key_type())
                    continue; /// skip witnesses without a valid block signing key

                if (selected_miners.find(sitr->id) == selected_miners.end()
                    && selected_voted.find(sitr->id) == selected_voted.end()) {
                    active_witnesses.push_back(sitr->owner);
                    db.modify(*sitr, [&](witness_object &wo) { wo.schedule = witness_object::timeshare; });
                    ++witness_count;
                }
            }

            auto num_timeshare = active_witnesses.size() - num_miners - num_elected;

            /// Update virtual schedule of processed witnesses
            bool reset_virtual_time = false;
            for (auto itr = processed_witnesses.begin(); itr != processed_witnesses.end(); ++itr) {
                auto new_virtual_scheduled_time =
                        new_virtual_time + VIRTUAL_SCHEDULE_LAP_LENGTH2 / ((*itr)->votes.value + 1);
                if (new_virtual_scheduled_time < new_virtual_time) {
                    reset_virtual_time = true; /// overflow
                    break;
                }
                db.modify(*(*itr), [&](witness_object &wo) {
                    wo.virtual_position = fc::uint128();
                    wo.virtual_last_update = new_virtual_time;
                    wo.virtual_scheduled_time = new_virtual_scheduled_time;
                });
            }
            if (reset_virtual_time) {
                new_virtual_time = fc::uint128();
                reset_virtual_schedule_time(db);
            }

            size_t expected_active_witnesses = std::min(size_t(STEEMIT_MAX_WITNESSES), widx.size());

            // Remove this check before hf22, which cause chain stuck at block: 3090927 during replay
            if(db.has_hardfork(STEEMIT_HARDFORK_0_22))
                FC_ASSERT( active_witnesses.size() == expected_active_witnesses, 
                    "number of active witnesses does not equal expected_active_witnesses=${expected_active_witnesses}",
                    ("active_witnesses.size()",active_witnesses.size()) 
                    ("STEEMIT_MAX_WITNESSES",STEEMIT_MAX_WITNESSES) 
                    ("expected_active_witnesses", expected_active_witnesses) );
            
            auto majority_version = wso.majority_version;

            if (db.has_hardfork(STEEMIT_HARDFORK_0_5)) {
                flat_map<version, uint32_t, std::greater<version> > witness_versions;
                flat_map<std::tuple<hardfork_version, time_point_sec>, uint32_t> hardfork_version_votes;

                for (uint32_t i = 0; i < wso.num_scheduled_witnesses; i++) {
                    auto witness = db.get_witness(wso.current_shuffled_witnesses[i]);
                    if (witness_versions.find(witness.running_version) == witness_versions.end())
                        witness_versions[witness.running_version] = 1;
                    else
                        witness_versions[witness.running_version] += 1;

                    auto version_vote = std::make_tuple(witness.hardfork_version_vote, witness.hardfork_time_vote);
                    if (hardfork_version_votes.find(version_vote) == hardfork_version_votes.end())
                        hardfork_version_votes[version_vote] = 1;
                    else
                        hardfork_version_votes[version_vote] += 1;
                }

                // major_version is initialized with wso.majority_version above.
                // for example:
                // witness_versions is: {"1.5": 15, "1.4":1, "1.2": 2, "1.1": 3}
                // wso.hardfork_required_witnesses = 17 (set in config)
                // so after the while loop, the majority_version will be: "1.2"
                // QUESTION: majority_version is only used to display info, doesn't involve in any concensus algorithm?
                // ANSWER: yes majority_version is not involve in the hardfork process logic, it's just for info displaying.
                // only hardfork_version_votes involves the hardfork process logic.
                int witnesses_on_version = 0;
                auto ver_itr = witness_versions.begin();

                // The map should be sorted highest version to smallest,
                // so we iterate until we hit the majority of witnesses on at least this version
                while (ver_itr != witness_versions.end()) {
                    witnesses_on_version += ver_itr->second;

                    if (witnesses_on_version >= wso.hardfork_required_witnesses) {
                        majority_version = ver_itr->first;
                        break;
                    }

                    ++ver_itr;
                }

                // "majority" feature for hardfork process logic starts here.
                auto hf_itr = hardfork_version_votes.begin();

                while (hf_itr != hardfork_version_votes.end()) {
                    if (hf_itr->second >= wso.hardfork_required_witnesses) {
                        const auto &hfp = db.get_hardfork_property_object();
                        if (hfp.next_hardfork != std::get<0>(hf_itr->first) ||
                            hfp.next_hardfork_time != std::get<1>(hf_itr->first)) {

                            db.modify(hfp, [&](hardfork_property_object &hpo) {
                                hpo.next_hardfork = std::get<0>(hf_itr->first);
                                hpo.next_hardfork_time = std::get<1>(hf_itr->first);
                            });
                        }
                        break;
                    }

                    ++hf_itr;
                }

                // We no longer have a majority
                // QUESTION: what is the consequences of not having a majority hardfork version?
                // ANSWER: if we don't have a najority, then new hardfork will not be applied.
                // since, hardfork is triggered by two factors: time and next_hardfork.
                if (hf_itr == hardfork_version_votes.end()) {
                    db.modify(db.get_hardfork_property_object(), [&](hardfork_property_object &hpo) {
                        hpo.next_hardfork = hpo.current_hardfork_version;
                    });
                }
            }

            assert(num_elected + num_miners + num_timeshare == active_witnesses.size());

            db.modify(wso, [&](witness_schedule_object &_wso) {
                for (size_t i = 0; i < active_witnesses.size(); i++) {
                    _wso.current_shuffled_witnesses[i] = active_witnesses[i];
                }

                for (size_t i = active_witnesses.size(); i < STEEMIT_MAX_WITNESSES; i++) {
                    _wso.current_shuffled_witnesses[i] = account_name_type();
                }

                // for example:
                // if you have 21 witness, then the witness_pay_normalization_factor will be equal to
                // 1 * 19 + 1 * 1 + 5 * 1 = 25
                _wso.num_scheduled_witnesses = std::max<uint8_t>(active_witnesses.size(), 1);
                _wso.witness_pay_normalization_factor =
                        _wso.top19_weight * num_elected
                        + _wso.miner_weight * num_miners
                        + _wso.timeshare_weight * num_timeshare;

                /// shuffle current shuffled witnesses
                auto now_hi = uint64_t(db.head_block_time().sec_since_epoch()) << 32;
                for (uint32_t i = 0; i < _wso.num_scheduled_witnesses; ++i) {
                    /// High performance random generator
                    /// http://xorshift.di.unimi.it/
                    uint64_t k = now_hi + uint64_t(i) * 2685821657736338717ULL;
                    k ^= (k >> 12);
                    k ^= (k << 25);
                    k ^= (k >> 27);
                    k *= 2685821657736338717ULL;

                    uint32_t jmax = _wso.num_scheduled_witnesses - i;
                    uint32_t j = i + k % jmax;
                    std::swap(_wso.current_shuffled_witnesses[i],
                              _wso.current_shuffled_witnesses[j]);
                }

                _wso.current_virtual_time = new_virtual_time;
                // looks like this next_shuffle_block_num is not used after STEEMIT_HARDFORK_0_4
                _wso.next_shuffle_block_num = db.head_block_num() + _wso.num_scheduled_witnesses;
                _wso.majority_version = majority_version;
            });

            update_median_witness_props(db);
        }
    }
}
