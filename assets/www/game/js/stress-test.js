(() => {
    const endpoint = "/cgi-bin/stress-test.siege";
    const storageKey = "cp_last_siege_run";
    const result = document.getElementById("stress-result");
    const lastRun = document.getElementById("last-stress-run");
    const validToken = /^[0-9a-f]{32}$/;

    try {
        if (result) sessionStorage.setItem(storageKey, result.dataset.job);
        const token = sessionStorage.getItem(storageKey);
        if (lastRun && token && validToken.test(token)) {
            lastRun.href = `${endpoint}?job=${token}`;
            lastRun.hidden = false;
        }
    } catch (_) { /* The run's URL still works when storage is unavailable. */ }

    if (!result) return;
    const token = result.dataset.job;
    const status = document.getElementById("stress-status");
    const output = document.getElementById("stress-output");
    const stop = document.getElementById("stress-stop");
    let offset = Number(result.dataset.offset) || 0;
    let timer;
    let active = true;
    let controller;
    const labels = { queued: "Starting…", running: "Running", stopping: "Stopping…", completed: "Completed", stopped: "Stopped", failed: "Failed" };

    async function refresh() {
        controller = new AbortController();
        const timeout = setTimeout(() => controller.abort(), 15000);
        let delay = 1000;
        try {
            const response = await fetch(`${endpoint}?job=${token}&format=json&offset=${offset}`, {
                cache: "no-store", signal: controller.signal,
            });
            const data = await response.json();
            if (!response.ok) {
                if (response.status === 400 || response.status === 404) active = false;
                throw new Error(data.error || `HTTP ${response.status}`);
            }
            const atBottom = output.scrollTop + output.clientHeight >= output.scrollHeight - 24;
            output.append(document.createTextNode(data.output));
            offset = data.offset;
            if (atBottom) output.scrollTop = output.scrollHeight;
            status.textContent = labels[data.state] || data.state;
            const running = ["queued", "running", "stopping"].includes(data.state);
            stop.hidden = !running;
            active = active && (running || data.has_more);
            if (data.has_more) delay = 0;
        } catch (error) {
            status.textContent = active ? "Connection interrupted. Retrying…" : error.message;
        } finally {
            clearTimeout(timeout);
            if (active) timer = setTimeout(refresh, delay);
        }
    }

    window.addEventListener("pagehide", () => {
        active = false;
        clearTimeout(timer);
        if (controller) controller.abort();
    });
    window.addEventListener("pageshow", (event) => {
        if (event.persisted) { active = true; refresh(); }
    });
    refresh();
})();
