#include <bitcoin/system.hpp>
#include "FundsFinder.hpp"

using namespace std;
using namespace bc;
using namespace bc::system::chain;
using namespace bc::system::wallet;
using namespace bc::system::machine;

pair<vector<output_point>, uint64_t> FundsFinder::find_funds(uint64_t satoshisNeeded, points_value points){
    vector<output_point> utxos;
    uint64_t gatheredFunds {0};
    for (auto p = begin(points.points); p != end(points.points) && gatheredFunds < satoshisNeeded; ++p){
        if (points.value() > 0){
            output_point utxo(p->hash(), p->index());
            utxos.push_back(utxo);
            gatheredFunds += p->value();
        }
    }
    if (gatheredFunds < satoshisNeeded)
        return {vector<output_point>(), gatheredFunds};
    else
        return {utxos, gatheredFunds};
}
