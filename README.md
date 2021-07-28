# btc locking utilities

## summary:

```
crtx        - create p2pkh payment transaction from funding transaction (offline)
crtx2       - create p2pkh payment transaction from address (online-L)
crlocktx    - create lock transaction from funding transaction (offline)
crlocktx2   - create lock transaction from address (online-L)
crlocktx3   - create lock transaction from a wallet created from seed (online-LE)
crunlocktx  - create unlock (spend) transaction (offline)
history     - show history of a wallet created from seed (online-E)
balance     - show balance of a wallet created from seed or given address' balance (online-E)
eclient     - demos electrum server calls: banner, history, balance (online-E)
lclient     - demos programmatic bx call: fetch-height, broadcast (online-L)
aconv       - converts address to EPS/EPSMI/Electrum Server address history key (offline)

E - uses Electrum server
L - uses Libbitcoin server
LE - uses both Libbitcoin and Electrum servers
```

This little set of utilities uses library binglib which is again used for wallet operations in
a the UI wallet LabZM. This code was meant as manual testing utility for the binglib library.
The library is based on libbitcoin and the API provided by Electrum servers.
Some implementation ideas were inspired by Electrum Personal Server and
its Scala version, but really big thanks to excellent blogs of http://aaronjaramillo.org.
Scripthash subscription and header subscription mechanisms are used
by the wallet although they are not used here.
Utilities like crlocktx3 and clunlocktx can be used to lock and unlock bitcoin,
although doing it via the UI wallet is probably much easier and less error-prone.

Unfortunately, big step of supporting SegWit is still not done, not to mention Taproot.
Consolation is that locking is intended to be done very rarely, so that blockchain should not
be polluted by legacy transactions too much.

All utilities support mainnet and testnet, online commands require --t=false for mainnet
and --t=true for testnet, which is the default.

## servers

Bingdiyi commands connect to 2 servers - libbitcoin server and Electrum server.
Server urls are contained in config.json file. Feel free to change the server data there.
To look up current libbitcoin servers' status, see:
https://github.com/libbitcoin/libbitcoin-server/wiki/Community-Servers
If you'd like to change the default Electrum server chosen,
easies way is to click the green button in your Electrum wallet, and grab
server url from there, then change config.json accordingly.
Here is an example config.json:
```
{
  "testnet": {
    "libbitcoin_connection": {
      "url": "tcp://testnet2.libbitcoin.net:19091"
    },
    "electrum_connection": {
      "host": "testnet.electrumx.hodlwallet.com",
      "service": "51002",
      "cert_file_path": "cert.crt"
    }
  },
  "mainnet": {
    "libbitcoin_connection": {
      "url": "tcp://mainnet2.libbitcoin.net:9091"
    },
    "electrum_connection": {
      "host": "electrumx.ultracloud.tk",
      "service": "50002",
      "cert_file_path": "cert.crt"
    }
  }
}
```
These values worked fine for me at the time of writing, but chances are, especially
for the Electrum server, that you'll have to update these values.
UI wallet uses multiple Electrum servers and automatic peer discovery mechanism, this
utility only uses a single server, so if the server is down, you need to manually
find out another server's url. Please note that only one utility, crlocktx3 uses
both Libbitcoin and Electrum servers, the rest uses servers as indicated in 
the summary - either L for Libbitcoin or E for Electrum.

## help

Every command utility, when invoked with no parameters, or with --h or --help parameter, will show
help information which should be sufficient to use the command. For completeness, this help information
is also included in this README file.


***
## crtx

Creates transaction to transfer funds via p2pkh

Required options:
```
  -h [ --help ]             print usage message
  -p [ --priv-key ] arg     private key (in WIF format)
  -f [ --funding-txid ] arg funding transaction id
  -v [ --vout ] arg         funding transaction output index (vout)
  --amount arg              amount to transfer (satoshis)
  -d [ --dest ] arg         destination address
```
example:
```
--f=d001bd68fc87f05ae3760b4f9c4b64e1000d9194d9c95e0b5a7c7efd933f43d1 --v=0 --amount=890000 --p=<private-key> --dest=msWHhBL1vLycmZtQ5M1j7xWuUYvienydfq
```
You can find funding transaction by:
1) bx fetch-balance <funding_address>
2) if the balance is sufficient, do:
   bx fetch-utxo <desired-amount-in-satoshis> <funding-address>
3) choose one utxo and capture 'hash' as funding transaction id
   and 'index' as funding transaction output index (vout)

Note that the amount to transfer must be smaller than the available amount in utxo
so that the remainder can be used as a fee.
This program does not give change, you need to use up the entire amount
from the UTXO.
Private key can be found in your wallet, in Electrum, go to tab 'Addresses',
highlight the desired address, right click and choose `private key'.
Ignore script type part of the key, like 'p2pkh', copy only the key part.

This is an offline program, it produces transaction in a hex format that can be broadcast
using any means, for example via 'bx send-tx <tx>' or any online transaction
broadcast drop-off place. This program works for both mainnet and testnet.

***
## crtx2

Creates transaction to transfer funds via p2pkh. Finds funding transaction(s) based on funding address.

Required options:
```
  -h [ --help ]             print usage message
  --addr arg                funding address
  -p [ --priv-key ] arg     private key for the funding address (in WIF format)
  --amount arg              amount to transfer (satoshis)
  --fee arg                 fee (satoshis), note: amount+fee <= available funds
  -d [ --dest ] arg         destination address
  -t [ --testnet ] arg (=1) use testnet blockchain
```
example:
```
--amount=90000 --fee=5000 --p=<private-key> --addr=msWHhBL1vLycmZtQ5M1j7xWuUYvienydfq --d=morHRfjX3sQ4R2BRSyjij8yePAW2XKHWd3 --t=true
```
You can find funding address by inspecting your wallet.
Note that the amount to transfer plus fee must be smaller than or equal to the available amount for a given address.
This program does give change, if any, it will be transferred back into the source address.
Private key can be found in your wallet, in Electrum, go to tab 'Addresses',
highlight the desired address, right click and choose `private key'.
Ignore script type part of the key, like 'p2pkh', copy only the key part.

This program produces transaction in a hex format that can be broadcast
using any means, for example via 'bx send-tx <tx>' or any online transaction
broadcast drop-off place.


***
## crlocktx

Creates transaction to lock funds via p2sh

Required options:
```
  -h [ --help ]             print usage message
  --addr arg                funding address
  -p [ --priv-key ] arg     private key to unlock the funding transaction (in
                            WIF format)
  -f [ --funding-txid ] arg funding transaction id
  -v [ --vout ] arg         funding transaction output index (vout)
  --amount arg              amount to transfer (satoshis)
  -l [ --lock-until ] arg   lock until epoch time (seconds)
```
example:
```
--f=d001bd68fc87f05ae3760b4f9c4b64e1000d9194d9c95e0b5a7c7efd933f43d1 --v=0 --a=890000 --l=1616255893 --p=<private-key> --addr=msWHhBL1vLycmZtQ5M1j7xWuUYvienydfq
```

You can find funding transaction by:
1) bx fetch-balance <funding_address>
2) if the balance is sufficient, do:
   bx fetch-utxo <desired-amount-in-satoshis> <funding-address>
3) choose one utxo and capture 'hash' as funding transaction id
   and 'index' as funding transaction output index (vout)
   Note that the amount to transfer must be smaller than the available amount in utxo
   so that the remainder can be used as a fee.
   This program does not give change, you need to use up the entire amount
   from the UTXO (divided to funds being locked and to a fee).
   For 'lock until' time, use any available online epoch time converter,
   note that epoch must be in seconds, not milliseconds. Also note, that the actual
   unlocking time will be delayed by around 7 blocks.

Private key can be found in your wallet, in Electrum, go to tab 'Addresses',
highlight the desired address, right click and choose `private key'.
Ignore script type part of the key, like 'p2pkh', copy only the key part.

This is an offline program, it produces transaction in a hex format that can be broadcast
using any means, for example via 'bx send-tx <tx>' or any online transaction
broadcast drop-off place.

Remember that you need to store the unlocking data as printed out by this program,
otherwise your funds will be lost.

This program works for both mainnet and testnet.

Note that crlocktx is a 100% offline tool.
If you want to be completely safe, you can use locked_tx_spender right
  away and keep transaction created by it, in such case you won't need
  anything else, you just need to broadcast the transaction when the
  unlocking time comes (you can keep the transaction in a text file,
  note that transaction is safe, if it is stolen, it will still transfer
  funds to your address, so if you have private key to the target address,
  it does not matter if you broadcast the transaction or a thief,
  caveat is that if you keep the entire transaction only, you lose the
  ability to change the fee should it turned out to be insufficient
  after the hodling time, hence, it is safer to keep necessary
  ingredients just in case you need to recreate transaction from scratch).

***
## crlocktx2

Creates transaction to lock funds via p2sh. Unlike crlocktx, it does give the rest.
It does not require funding transaction so it is easier to use than crlocktx.

Required options:
```
  -h [ --help ]             print usage message
  --addr arg                funding address
  -p [ --priv-key ] arg     private key for the funding address (in WIF format)
  --amount arg              amount to transfer (satoshis)
  --fee arg                 fee (satoshis), note: amount+fee <= available funds
  -l [ --lock-until ] arg   lock until epoch time (seconds)
  -t [ --testnet ] arg (=1) use testnet blockchain
```
example:
```
--amount=890000 --fee=5000 --l=1616255893 --p=<private-key> --addr=msWHhBL1vLycmZtQ5M1j7xWuUYvienydfq --t=true
```

You can find funding address by inspecting your wallet.
Note that the amount to transfer plus fee must be smaller than or equal to the available amount for a given address.
This program does give change, if any, it will be transferred back into the source address.
For 'lock until' time, use any available online epoch time converter,
note that epoch must be in seconds, not milliseconds. Also note, that the actual
unlocking time will be delayed by around 7 blocks.
Private key can be found in your wallet, in Electrum, go to tab 'Addresses',
highlight the desired address, right click and choose `private key'.
Ignore script type part of the key, like 'p2pkh', copy only the key part.

This program produces transaction in a hex format that can be broadcast
using any means, for example via 'bx send-tx <tx>' or any online transaction
broadcast drop-off place.

Remember that you need to store the unlocking data as printed out by this program,
otherwise your funds will be lost.

Note that crlocktx2 is an online tool, it is similar to crlocktx but:
- accepts address amount and fee only, figures out the funding transaction(s)
  by itself
- is able to gather the amount from multiple funding transactions
- is able to transfer the remaining money back to the original address
  transaction created by crlocktx2 can be spent by
  crunlocktx, transaction UTXO is the same as from crlocktx or crlocktx3.

***
## crlocktx3

Creates transaction to lock funds via p2sh
Requires only the seed phrase, finds funding transaction(s) automatically.

Required options:
```
  -h [ --help ]                         print usage message
  -s [ --seed ] arg                     Electrum seed phrase
  --amount arg                          amount to transfer (satoshis)
  --fee arg                             fee (satoshis), note: amount+fee <=
                                        available funds
  -l [ --lock-until ] arg               lock until epoch time (seconds)
  -r [ --receiving addresses ] arg (=50)
                                        number of receiving addresses
  -c [ --change addresses ] arg (=50)   number of change addresses
  -t [ --testnet ] arg (=1)             use testnet blockchain
```
example:
```
--amount=90000 --fee=5000 --l=1616255000 --s="effort canal zoo ... few quick" --t=true
```
You only provide Electrum mnemonic seed phrase and the program will
find the funding transaction automatically.

Note that all funds must be under a single address, multiple addresses will not
be gathered to contribute their funds to the desired amount.

This program does give change, if any, it will be transferred back into the source address.
For 'lock until' time, use any available online epoch time converter,
note that epoch must be in seconds, not milliseconds. Also note, that the actual
unlocking time will be delayed by around 7 blocks.

This program produces transaction in a hex format that can be broadcast
using any means, for example via 'bx send-tx <tx>' or any online transaction
broadcast drop-off place.

Remember that you need to store the unlocking data as printed out by this program,
otherwise your funds will be lost.

***
## crunlocktx

Creates transaction to unlock funds locked via p2sh

Required options:
```
  -h [ --help ]             print usage message
  -p [ --priv-key ] arg     private key to unlock the funding transaction (in
                            WIF format)
  -f [ --funding-txid ] arg funding transaction id
  --amount arg              amount to transfer (satoshis)
  -l [ --lock-until ] arg   lock until epoch time (seconds)
  -d [ --dest ] arg         destination address
```
example:
```
--p=<private-key> --f=29e959ce847842ee86d22703b68c725c854328675b660f15d5272fa71ffc38ba --a=88000 --l=1616255893 --d=n4XMrFeDdEg2vtDYQzDcaK7jcthh5xG4MX
```
You need to have the following items of information available
(as advised and provided by the locking program):
1) lock time
2) private key of the source address (from where the funds originated before locking)
3) available amount (from which the fee needs to be subtracted)
4) funding (locking) transaction id to unlock
   You will also need the destination address where the funds should be transferred to.

An example unlocking information looks as follows:
```
===== data to unlock: ====
lock time: 1616418000
private key of address: mkP2QQqQYsReSpt3JBoRQ5zVdw3ra1jenh
available amount: 210000 satoshi
from ^^ please subtract fee
funding transaction id to unlock: 085f3e80771036a68ee4116fdb208eb44ffadce70fcd9d77cf935537535d0b27
desired destination address to which the unlocked funds will be transferred
==========================
```
This program produces transaction in a hex format that can be broadcast
using any means, for example via 'bx send-tx <tx>' or any online transaction
broadcast drop-off place.

This program works for both mainnet and testnet.

Note that crunlocktx is a 100% offline tool.
It creates transaction transferring funds that have been locked by
the crlocktx or crlocktx2 or crlocktx3 transaction (or any other compatible transaction).
The unlocking transaction can be broadcast before the lock time, but it will
be rejected and not propagated, nevertheless it is safe to try to do so

***

## history

Displays wallet history

Required options:
```
  -h [ --help ]                         print usage message
  -s [ --seed ] arg                     Electrum seed phrase
  -r [ --receiving-addresses ] arg (=50)
                                        number of receiving addresses
  -c [ --change-addresses ] arg (=50)   number of change addresses
  -t [ --testnet ] arg (=1)             use testnet blockchain
```
example:
```
--r=50 --c=50 --s="effort canal zoo ... few quick" --t=true
```

***

## balance

Displays wallet balance

Required options:
```
  -h [ --help ]                         print usage message
  -s [ --seed ] arg                     Electrum seed phrase
  --addr arg                            optional address
  -r [ --receiving-addresses ] arg (=50)
                                        number of receiving addresses
  -c [ --change-addresses ] arg (=50)   number of change addresses
  -t [ --testnet ] arg (=1)             use testnet blockchain
```
example:
```
--r=50 --c=50 --s="effort canal zoo ... few quick" --t=true
```

***

## eclient

Demonstrates examples of programmatic calls via Electrum client

Required options:
```
  -h [ --help ]             print usage message
  -t [ --testnet ] arg (=1) use testnet blockchain
```

***

## lclient

Demonstrates programmatic call via libbitcoin client

Required options:
```
  -h [ --help ]             print usage message
  -t [ --testnet ] arg (=1) use testnet blockchain
```

***

##aconv

Converts address to scripthash as used by EPS/EPSMI/Electrum Server address history key.

Required options:
```
  -h [ --help ]         print usage message
  --addr arg            source address
```
example:
```
--a=mpS14bFCZiHFRxfNNbnPT2FScJBrm96iLE
```
