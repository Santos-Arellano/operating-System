def run(refs, frames, algo):
    if algo == "FIFO":
        res = fifo(refs, frames)
    elif algo == "LRU":
        res = lru(refs, frames)
    elif algo == "OPT":
        res = opt(refs, frames)
    elif algo == "LFU":
        res = lfu(refs, frames)
    else:
        raise ValueError("algo")
    total = len(refs)
    faults = res.get("faults", 0)
    hits = total - faults
    res["hits"] = hits
    res["hit_ratio"] = (hits / total if total > 0 else 0.0)
    return res