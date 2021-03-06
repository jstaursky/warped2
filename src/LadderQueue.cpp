#include "LadderQueue.hpp"
#include <cassert>

namespace warped {

LadderQueue::LadderQueue() {

    /* Reserve the size of Top */
    top_.buffer_.reserve(2*THRESHOLD);

    /* Create and Initialize Rungs */
    for (unsigned int i = 0; i < MAX_RUNG_CNT; i++) {

        /* NOTE: First rung's bucket count equals 2*THRESHOLD,
                 Other rungs' bucket count equals THRESHOLD
         */
        unsigned int bucket_cnt = i ? THRESHOLD : 2*THRESHOLD;
        for (unsigned int j = 0; j < bucket_cnt; j++) {
            bucket temp;
            temp.reserve(THRESHOLD);
            rung_[i].buffer_.push_back(temp);
        }
    }

#ifdef PARTIALLY_SORTED_LADDER_QUEUE
    /* Initialize Bottom capacity */
    bottom_.reserve(THRESHOLD);
#endif
}

std::shared_ptr<Event> LadderQueue::dequeue() {

    // Remove from bottom if not empty
    if ( !bottom_.empty() ) {

#ifdef PARTIALLY_SORTED_LADDER_QUEUE
        auto e = bottom_.back();
        bottom_.pop_back();
        return e;
#else
        auto e = *bottom_.begin();
        bottom_.erase( bottom_.begin() );
        return e;
#endif
    }

    // If there are no rungs, pull from Top
    if (!rung_cnt_) {
        createNewRung();

        // Transfer events from Top to 1st rung of Ladder
        rung_[0].first_nonempty_bucket_ts_ = (unsigned int)-1;
        rung_[0].last_nonempty_bucket_ts_ = 0;
        for (auto event : top_.buffer_) {
            unsigned int index = (event->timestamp()-rung_[0].start_ts_) / rung_[0].bucket_width_;
            rung_[0].buffer_[index].push_back(event);

            rung_[0].first_nonempty_bucket_ts_ =  std::min( rung_[0].first_nonempty_bucket_ts_ ,
                                                            rung_[0].start_ts_ +
                                                                index*rung_[0].bucket_width_    );
            rung_[0].last_nonempty_bucket_ts_ =
                                    std::max(   rung_[0].last_nonempty_bucket_ts_ ,
                                                rung_[0].start_ts_ +
                                                    index*rung_[0].bucket_width_    );
        }
        top_.buffer_.clear();
        top_.start_ts_ = top_.max_ts_ + 1;

        recurseRung();
    }

    // Move the first non-empty bucket of last rung to bottom
    unsigned int bucket_index =
        (rung_[rung_cnt_-1].first_nonempty_bucket_ts_ - rung_[rung_cnt_-1].start_ts_) /
                                                rung_[rung_cnt_-1].bucket_width_;

#ifdef PARTIALLY_SORTED_LADDER_QUEUE
    bottom_start_ = rung_[rung_cnt_-1].first_nonempty_bucket_ts_;
    for (auto event : rung_[rung_cnt_-1].buffer_[bucket_index]) {
        bottom_.push_back(event);
    }
    rung_[rung_cnt_-1].buffer_[bucket_index].clear();
#else
    for (auto event : rung_[rung_cnt_-1].buffer_[bucket_index]) {
        bottom_.insert(event);
    }
    rung_[rung_cnt_-1].buffer_[bucket_index].clear();
#endif

    // If last rung is empty now
    if (rung_[rung_cnt_-1].first_nonempty_bucket_ts_ ==
                                rung_[rung_cnt_-1].last_nonempty_bucket_ts_) {
        rung_cnt_--;

    } else { // Another non-empty bucket is present on the last rung
        do {
            rung_[rung_cnt_-1].first_nonempty_bucket_ts_ += rung_[rung_cnt_-1].bucket_width_;
        } while( rung_[rung_cnt_-1].buffer_[++bucket_index].empty() );
    }
    recurseRung();

    // Check and erase from bottom, if present
    if (bottom_.empty()) return nullptr;

#ifdef PARTIALLY_SORTED_LADDER_QUEUE
    auto e = bottom_.back();
    bottom_.pop_back();
    return e;
#else
    auto e = *bottom_.begin();
    bottom_.erase( bottom_.begin() );
    return e;
#endif
}

void LadderQueue::insert(std::shared_ptr<Event> event) {

    assert(event);
    auto ts = event->timestamp();

    // Insert into top, if valid
    if (ts >= top_.start_ts_) {
        if(top_.buffer_.empty()) {
            top_.max_ts_ = ts;
        }
        top_.max_ts_ = std::max( ts, top_.max_ts_ );
        top_.buffer_.push_back(event);
        return;
    }

    // Step through rungs
    unsigned int rung_index = 0;
    while ( (rung_index < rung_cnt_) && (ts < rung_[rung_index].first_nonempty_bucket_ts_) ) {
        rung_index++;
    }

    // If a valid rung was found, insert the event
    if (rung_index < rung_cnt_) {
        unsigned int bucket_index = (ts - rung_[rung_index].start_ts_) /
                                                            rung_[rung_index].bucket_width_;

        rung_[rung_index].last_nonempty_bucket_ts_ =
                    std::max (  rung_[rung_index].last_nonempty_bucket_ts_,
                                rung_[rung_index].start_ts_ +
                                    bucket_index*rung_[rung_index].bucket_width_    );

        rung_[rung_index].buffer_[bucket_index].push_back(event);
        recurseRung();
        return;
    }

    // Insert into Bottom
#ifdef PARTIALLY_SORTED_LADDER_QUEUE
    bottom_.push_back(event);
    bottom_start_ = std::min(bottom_start_, ts);
#else
    bottom_.insert(event);
#endif
}

bool LadderQueue::erase(std::shared_ptr<Event> event) {

    assert(event);
    auto ts = event->timestamp();

    // Erase from top, if found there
    if (ts >= top_.start_ts_) {
        if(top_.buffer_.empty()) return false;

        top_.max_ts_ = 0;
        bool status = false;
        for (auto it = top_.buffer_.begin(); it != top_.buffer_.end();) {
            if (*it == event) {
                status = true;
                it = top_.buffer_.erase(it);
            } else {
                top_.max_ts_ = std::max( (*it)->timestamp(), top_.max_ts_ );
                it++;
            }
        }
        return status;
    }

    // Step through rungs
    unsigned int rung_index = 0;
    while ( (rung_index < rung_cnt_) && (ts < rung_[rung_index].first_nonempty_bucket_ts_) ) {
        rung_index++;
    }

    // Delete the element if found inside a valid rung
    if (rung_index < rung_cnt_) {
        unsigned int bucket_index = (ts - rung_[rung_index].start_ts_) /
                                                    rung_[rung_index].bucket_width_;
        unsigned int current_index =
                (rung_[rung_index].first_nonempty_bucket_ts_ - rung_[rung_index].start_ts_) /
                                                    rung_[rung_index].bucket_width_;
        unsigned int last_valid_index =
                (rung_[rung_index].last_nonempty_bucket_ts_ - rung_[rung_index].start_ts_) /
                                                    rung_[rung_index].bucket_width_;

        // NOTE: Don't delete event if it is the only event in the
        //       first or last non-empty bucket of a inner rung
        if (rung_[rung_index].buffer_[bucket_index].size() <= 1) {

            // If chosen rung is an inner one
            if (rung_index < rung_cnt_-1) {

                // If the chosen bucket is the first or the last non-empty bucket
                if ( bucket_index == current_index || bucket_index == last_valid_index ) {
                    return false;
                }
            }
        }

        // Delete the event, if found
        bool status = false;
        for (auto it = rung_[rung_index].buffer_[bucket_index].begin();
                                it != rung_[rung_index].buffer_[bucket_index].end(); it++) {
            if (*it == event) {
                (void) rung_[rung_index].buffer_[bucket_index].erase(it);
                status = true;
                break;
            }
        }

        // If erase successful and chosen bucket in the last rung is empty after erase
        if (status && rung_index == rung_cnt_-1 &&
                            !rung_[rung_index].buffer_[bucket_index].size()) {

            // If that bucket was the only one in the last rung
            if (current_index == last_valid_index) {
                rung_cnt_--;
                recurseRung();

            } else if (bucket_index == current_index) { // first non-empty bucket
                // Update the first nonempty ts for last rung
                do {
                    rung_[rung_index].first_nonempty_bucket_ts_ += rung_[rung_index].bucket_width_;
                } while( rung_[rung_index].buffer_[++bucket_index].empty() );
                recurseRung();

            } else if (bucket_index == last_valid_index) { // last non-empty bucket
                // Update the last nonempty ts for last rung
                do {
                    rung_[rung_index].last_nonempty_bucket_ts_ -= rung_[rung_index].bucket_width_;
                } while( rung_[rung_index].buffer_[--bucket_index].empty() );
            }
        }
        return status;
    }

    // Check and erase from bottom, if present
    if (bottom_.empty()) return false;

    bool status = false;

#ifdef PARTIALLY_SORTED_LADDER_QUEUE
    for (auto it = bottom_.begin(); it != bottom_.end(); it++) {
        if (*it == event) {
            (void) bottom_.erase(it);
            status = true;
            break;
        }
    }
#else
    status = (bottom_.erase(event) >= 1) ? true : false;
#endif

    return status;
}

void LadderQueue::createNewRung() {

    /* Check if this is the first rung creation */
    if (!rung_cnt_) {
        rung_cnt_++;
        rung_[0].bucket_width_ = (top_.max_ts_ - top_.start_ts_ + rung_[0].buffer_.size()) /
                                                                            rung_[0].buffer_.size();
        rung_[0].start_ts_  = top_.start_ts_;

    } else { // When rungs already exist
        rung_cnt_++;
        rung_[rung_cnt_-1].bucket_width_ =
            (rung_[rung_cnt_-2].bucket_width_ + rung_[rung_cnt_-1].buffer_.size()) /
                                                        rung_[rung_cnt_-1].buffer_.size();
        rung_[rung_cnt_-1].start_ts_  = rung_[rung_cnt_-2].first_nonempty_bucket_ts_;
    }
}

void LadderQueue::recurseRung() {

    // If there are no rungs
    if (!rung_cnt_) return;
    
    // If rung count has reached upper threshold
    if (rung_cnt_ >= MAX_RUNG_CNT) return;

    // If the last rung has only one non-empty bucket
    if (rung_[rung_cnt_-1].last_nonempty_bucket_ts_ ==
                    rung_[rung_cnt_-1].first_nonempty_bucket_ts_) return;

    // If the last rung has minimum bucket width
    if (rung_[rung_cnt_-1].bucket_width_ == MIN_BUCKET_WIDTH) return;

    unsigned int bucket_index =
        (rung_[rung_cnt_-1].first_nonempty_bucket_ts_ - rung_[rung_cnt_-1].start_ts_) /
                                                rung_[rung_cnt_-1].bucket_width_;

    // If event count inside the first non-empty bucket of last rung is within threshold
    if (rung_[rung_cnt_-1].buffer_[bucket_index].size() <= THRESHOLD) return;

    // Create a new rung if event count exceeds threshold
    createNewRung();

    // Move events from the bucket to the new rung
    rung_[rung_cnt_-1].first_nonempty_bucket_ts_ = (unsigned int)-1;
    rung_[rung_cnt_-1].last_nonempty_bucket_ts_ = 0;
    for (auto event : rung_[rung_cnt_-2].buffer_[bucket_index]) {
        unsigned int index = 
            (event->timestamp() - rung_[rung_cnt_-1].start_ts_) / rung_[rung_cnt_-1].bucket_width_;
        rung_[rung_cnt_-1].buffer_[index].push_back(event);
        rung_[rung_cnt_-1].first_nonempty_bucket_ts_ =
                        std::min(   rung_[rung_cnt_-1].first_nonempty_bucket_ts_ ,
                                    rung_[rung_cnt_-1].start_ts_ +
                                            index*rung_[rung_cnt_-1].bucket_width_  );
        rung_[rung_cnt_-1].last_nonempty_bucket_ts_ =
                        std::max(   rung_[rung_cnt_-1].last_nonempty_bucket_ts_ ,
                                    rung_[rung_cnt_-1].start_ts_ +
                                            index*rung_[rung_cnt_-1].bucket_width_  );
    }
    rung_[rung_cnt_-2].buffer_[bucket_index].clear();

    // Update the current ts for penultimate rung
    do {
        rung_[rung_cnt_-2].first_nonempty_bucket_ts_ += rung_[rung_cnt_-2].bucket_width_;
    } while( rung_[rung_cnt_-2].buffer_[++bucket_index].empty() );

    // Repeat the recursive process
    recurseRung();
}

} // namespace warped
