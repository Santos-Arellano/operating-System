import threading
import time
import random
from collections import deque

def run_producer_consumer(buffer_size, producers, consumers, seconds):
    buf = deque()
    maxsize = buffer_size
    lock = threading.Lock()
    not_empty = threading.Condition(lock)
    not_full = threading.Condition(lock)
    stop = threading.Event()

    def prod(i):
        while True:
            if stop.is_set():
                return
            x = random.randint(1, 100)
            with not_full:
                while len(buf) >= maxsize and not stop.is_set():
                    not_full.wait()
                if stop.is_set():
                    return
                buf.append(x)
                print(f"P{i} produce {x} size={len(buf)}")
                not_empty.notify()
            time.sleep(0.05)

    def cons(i):
        while True:
            with not_empty:
                while not buf and not stop.is_set():
                    not_empty.wait()
                if stop.is_set() and not buf:
                    return
                x = buf.popleft()
                print(f"C{i} consume {x} size={len(buf)}")
                not_full.notify()
            time.sleep(0.05)

    threads = [threading.Thread(target=prod, args=(i,)) for i in range(producers)]
    threads += [threading.Thread(target=cons, args=(i,)) for i in range(consumers)]
    for t in threads:
        t.start()
    time.sleep(seconds)
    stop.set()
    with lock:
        not_empty.notify_all()
        not_full.notify_all()
    for t in threads:
        t.join()