
#include <bitcoin/explorer/config/ec_private.hpp>


using namespace std;
using namespace bc;

int main() {
    // private key for source_addr
    const string privKeyWIF {"cN9XS1bFNhMmmvXTNudnwZd7zyuRwCk4HmEVy4xbSxtPArC4KcoE"};
    const wallet::ec_private privKeyEC(privKeyWIF);
    const wallet::ec_public pubKey = privKeyEC.to_public();
    const libbitcoin::config::base16 privKey = libbitcoin::config::base16(privKeyEC.secret());
    // amount to transfer in Satoshis
    const uint64_t satoshisToTransfer {80000};
    // fee in Satoshis - as calculated with help of bx fetch-utxo, where we can see
    // if srcAddr has sufficient funds for the main transfer, the change, and the fee
    const uint64_t fee {10000};
    // source address
    const string srcAddr {""};
    // source transaction id (as found out via bx fetch-tx)
    const string srcTxId {""};
    // source transaction's output index (as found out via bx fetch-tx)
    const int srcTxOutputIndex {0};
    // main target address
    const string targetAddr {""};
    // target address for a change
    const string targetRemainderAddr {""};


    cout << "priv WIF: " << privKeyEC << endl;
    cout << "public hex: " << pubKey << endl;
    cout << "private hex: " << privKey << endl;
}
