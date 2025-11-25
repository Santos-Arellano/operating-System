def metrics(schedule, processes, completion):
    mp = {p["pid"]: p for p in processes}
    wt, tt, rt = {}, {}, {}
    first = {}
    for seg in schedule:
        pid = seg["pid"]
        if pid not in first:
            first[pid] = seg["start"]
    for pid, ct in completion.items():
        burst = mp[pid]["burst"]
        arr = mp[pid]["arrival"]
        tt[pid] = ct - arr
        wt[pid] = tt[pid] - burst
        rt[pid] = (first.get(pid, arr) - arr)
    avg_wait = sum(wt.values()) / len(wt) if wt else 0.0
    avg_turn = sum(tt.values()) / len(tt) if tt else 0.0
    avg_resp = sum(rt.values()) / len(rt) if rt else 0.0
    return {
        "schedule": schedule,
        "waiting": wt,
        "turnaround": tt,
        "response": rt,
        "avg_wait": avg_wait,
        "avg_turn": avg_turn,
        "avg_response": avg_resp,
    }


def prio(processes):
    t = 0.0
    sch = []
    comp = {}
    ready = []
    procs = sorted(processes, key=lambda x: x["arrival"])
    i = 0
    n = len(procs)
    while len(comp) < n:
        while i < n and procs[i]["arrival"] <= t:
            ready.append(procs[i])
            i += 1
        if not ready:
            if i < n:
                t = procs[i]["arrival"]
                continue
            else:
                break
        p = min(ready, key=lambda x: (x["priority"], x["arrival"], x["pid"]))
        ready = [r for r in ready if r["pid"] != p["pid"]]
        start = t
        end = t + p["burst"]
        sch.append({"pid": p["pid"], "start": start, "end": end})
        comp[p["pid"]] = end
        t = end
    return metrics(sch, processes, comp)


def prio_preempt(processes):
    t = 0.0
    sch = []
    comp = {}
    rem = {p["pid"]: p["burst"] for p in processes}
    procs = sorted(processes, key=lambda x: x["arrival"])  # by arrival time
    i = 0
    ready = []
    cur = None
    seg_start = None
    n = len(processes)
    while len(comp) < n:
        while i < len(procs) and procs[i]["arrival"] <= t:
            ready.append(procs[i])
            i += 1
        # Drop finished from ready
        ready = [p for p in ready if rem[p["pid"]] > 0 and p["pid"] not in comp]
        # Close segment if current finished
        if cur is not None and rem[cur["pid"]] <= 0:
            comp[cur["pid"]] = t
            sch.append({"pid": cur["pid"], "start": seg_start, "end": t})
            cur = None
            seg_start = None
        # Pick next current
        if cur is None:
            if not ready:
                if i < len(procs):
                    t = procs[i]["arrival"]
                    continue
                else:
                    break
            cur = min(ready, key=lambda x: (x["priority"], x["arrival"], x["pid"]))
            seg_start = t
        else:
            best = min(ready + [cur], key=lambda x: (x["priority"], x["arrival"], x["pid"]))
            if best["pid"] != cur["pid"]:
                # preempt current
                sch.append({"pid": cur["pid"], "start": seg_start, "end": t})
                cur = best
                seg_start = t
        # Run until next arrival or completion
        next_arr = procs[i]["arrival"] if i < len(procs) else float("inf")
        delta = min(rem[cur["pid"]], max(0.0, next_arr - t))
        if delta == 0.0 and next_arr != float("inf"):
            # jump to next arrival to reconsider
            t = next_arr
            continue
        t += delta
        rem[cur["pid"]] -= delta
        if rem[cur["pid"]] == 0:
            comp[cur["pid"]] = t
            sch.append({"pid": cur["pid"], "start": seg_start, "end": t})
            cur = None
            seg_start = None
    return metrics(sch, processes, comp)


def run(processes, algo, quantum=1.0):
    plist = []
    for p in processes:
        plist.append({
            "pid": str(p["pid"]),
            "arrival": float(p["arrival"]),
            "burst": float(p["burst"]),
            "priority": int(p.get("priority", 0)),
        })
    if algo == "PRIO":
        return prio(plist)
    if algo == "PRIO_PREEMPT":
        return prio_preempt(plist)
    raise ValueError("algo")