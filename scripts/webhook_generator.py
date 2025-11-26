import asyncio
import aiohttp
import argparse
import json
import time
from datetime import datetime


async def send_one(session, url, payload):
    """Send a single POST request and optionally log the result."""
    try:
        async with session.post(url, data=payload) as resp:
            # We read the body just to fully consume the response
            text = await resp.text()
            print(f"[{datetime.now().strftime('%H:%M:%S')}] {resp.status} - {text}")
    except Exception as e:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Request failed: {e}")


async def main():
    parser = argparse.ArgumentParser(description="Async TradingView-style webhook load generator.")
    parser.add_argument("--url", default="http://localhost:8000/webhook",
                        help="Target webhook URL (default: http://localhost:8000/webhook)")
    parser.add_argument("--symbol", default="AAPL",
                        help="Symbol field in JSON payload (default: AAPL)")
    parser.add_argument("--side", default="BUY",
                        help="Side field in JSON payload (default: BUY)")
    parser.add_argument("--price", type=float, default=150.25,
                        help="Price field in JSON payload (default: 150.25)")
    parser.add_argument("--hz", type=float, default=1000.0,
                        help="Messages per second (default: 10.0)")
    parser.add_argument("--duration", type=float, default=10.0,
                        help="How long to send for, in seconds (default: 10)")
    parser.add_argument("--max_in_flight", type=int, default=1000,
                        help="Max concurrent requests in flight (default: 1000)")
    args = parser.parse_args()

    interval = 1.0 / args.hz
    total_msgs = int(args.hz * args.duration)

    payload_dict = {
        "symbol": args.symbol,
        "side": args.side,
        "price": args.price,
    }
    payload = json.dumps(payload_dict)

    print(f"URL:       {args.url}")
    print(f"Payload:   {payload}")
    print(f"Rate:      {args.hz} msg/s")
    print(f"Duration:  {args.duration} s")
    print(f"Messages:  {total_msgs}")
    print(f"Max in-flight: {args.max_in_flight}")
    print("-------------------------------------------------------")

    semaphore = asyncio.Semaphore(args.max_in_flight)
    tasks = []

    async with aiohttp.ClientSession(headers={"Content-Type": "application/json"}) as session:
        start = time.perf_counter()
        next_send = start

        for i in range(total_msgs):
            # Rate control: wait until the scheduled time for this message
            now = time.perf_counter()
            if now < next_send:
                await asyncio.sleep(next_send - now)

            next_send += interval

            # Limit number of in-flight requests
            await semaphore.acquire()

            async def wrapped_send():
                try:
                    await send_one(session, args.url, payload)
                finally:
                    semaphore.release()

            # Fire off the request without waiting for it to finish
            tasks.append(asyncio.create_task(wrapped_send()))

        # Wait for all outstanding requests to complete
        await asyncio.gather(*tasks)

        elapsed = time.perf_counter() - start
        print(f"Finished sending {total_msgs} messages in {elapsed:.3f} s")


if __name__ == "__main__":
    asyncio.run(main())
