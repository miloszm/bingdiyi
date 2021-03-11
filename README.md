btc utility based on libbitcoin-explorer

***
locked_tx_pusher - 100% offline tool
creates transaction transferring funds so that they remain 
locked until specified time
to be able to unlock it you need to keep:
- private key 
- the exact lock time (epoch in seconds)
- transaction id (hash) of a transaction created by this tool
- redeem script format (if you use locked_tx_spender, you don't need to worry about it)
if you want to be completely safe, you can use locked_tx_spender right
away and keep transaction created by it, in such case you won't need
anything else, you just need to broadcast the transaction when the
unlocking time comes (you can keep the transaction in a text file, 
note that transaction is safe, if it is stolen, it will still transfer
funds to your address, so if you have private key to the target address,
it does not matter if you broadcast the transaction or a thief, 
caveat is that if you keep the entire transaction only, you lose the
ability to change the fee should it turned out to be insufficient
after the hodling time, hence, it is safer to keep necessary
ingredients just in case you need to recreate transaction from scratch)

***
locked_tx_spender - 100% offline tool
creates transaction transferring funds that have been locked by
the locked_tx_pusher transaction (or any other compatible transaction)
you need to jave provate key, exect lock time and transaction
is (hash) of a transaction created by locked_tx_pusher
the transaction can be broadcast before the lock time, but it will
be rejected and not propagated, nevertheless it is safe to try to do so

***
locked_tx_autopusher - an online tool
similar to locked_tx_pusher but:
- accepts address amount and fee only, figures out the funding transaction(s)
  by itself
- is able to gather the amount from multiple funding transactions
- is able to transfer the remaining money back to the original address
transaction created by locked_tx_autopusher can be spent by
locked_tx_spender, transaction UTXO is the same as from locked_tx_pusher
  
