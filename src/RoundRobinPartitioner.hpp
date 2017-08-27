#ifndef ROUND_ROBIN_PARTITIONER_HPP
#define ROUND_ROBIN_PARTITIONER_HPP

#include <vector>

#include "Partitioner.hpp"

namespace warped {

class LogicalProcess;

class RoundRobinPartitioner : public Partitioner {
public:
    RoundRobinPartitioner(unsigned int block_size) :
        block_size_(block_size) {}

    std::vector<std::vector<LogicalProcess*>> interNodePartition(
                                const std::vector<LogicalProcess*>& lps,
                                const unsigned int num_nodes    ) const;

    std::vector<std::vector<LogicalProcess*>> intraNodePartition(
                                const std::vector<LogicalProcess*>& lps ) const;

private:
    mutable unsigned int block_size_;

};

} // namespace warped

#endif
