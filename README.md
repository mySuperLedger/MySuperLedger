# MySuperLedger

## What is MySuperLedger
MySuperLedger is a database specifically designed for accounting purpose. You can use it to create accounts, create ledgers, 
do double-entry bookkeeping. Its goal is to equip SMEs with the capability to handle super large accounting volume 
which usually internet giants such as Alipay, WeChat Pay, PayPal have.

## Why SMEs will need MySuperLedger
The main reason is I believe the pay-as-you-go payment model will lead to a burst of bookkeeping activities which can easily
reach the same magnitude of traffic and high-availability requirement that only internet giants have.

The accounting capability will be crucial to the success of SMEs that rely on the pay-as-you-go model. However, they usually 
don't have sufficient resources to develop their own accounting software, and existing ones are feature-rich but don't meet
these non-functional requirements, e.g., high traffic volume, 7x24 available, zero data loss.

## Who am I and Why I can do this
I am a seasoned developer in the financial industry for over 13 years, dealing with critical finance data most of my career.
I designed and led the development the financial accounting system for eBay management payment. The system can now handle 1 million transactions
per second and can easily scale out to support even larger volume. It is also high available, 99.999% available in over two years, 
that's 7-minutes downtime, it could get more 9s if that mis-operation didn't happen. Thirdly, the system can guarantee no data loss, which is
also a must-have requirement on financial accounting system. Last but not least, it's 100% audit-able, it will not only tell you the 
account's latest balance, but also evey transaction that leads to it.

Below are two articles I wrote about the system.
1. [超越“双十一” | ebay支付核心账务系统架构演进之路](https://mp.weixin.qq.com/s/O5_Rde5uUXvmBS2B7w2hOQ)
2. [eBay支付账务系统架构解析之“读”一无二](https://mp.weixin.qq.com/s?__biz=MzA3MDMyNDUzOQ==&mid=2650512756&idx=1&sn=04fd97a4c4129bb26c5dd3b6ea7e75f9&chksm=8731a520b0462c369671c63d06721ee7817984bf041964fbdeb24d513c9b31451cbb544ed2c1&scene=178&cur_album_id=1590887959409590275#rd)

## What's incoming
I'm going to provide following features:
1. Create a Credit or Debit account
2. Create a ledger
3. Add a transaction to a ledger
4. Query account's balance
5. Query the transaction history of an account or ledger

Stay tuned.

## How to connect
Feel free to send a message from all below channels. I'd love to discuss your use scenarios and what I can help on that.
1. telegram: t.me/MySuperLedger
2. discord: #MySuperLedger

