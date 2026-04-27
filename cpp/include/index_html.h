#pragma once

static const char kIndexHtml[] = R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Robot Control</title>
    <style>
        :root {
            --bg-1: #0f172a;
            --bg-2: #0b1220;
            --panel: rgba(15, 23, 42, 0.75);
            --text: #e2e8f0;
            --muted: #94a3b8;
            --accent-1: #38bdf8;
            --accent-2: #f59e0b;
            --ring: rgba(56, 189, 248, 0.25);
            --danger: #ef4444;
            --danger-dark: #b91c1c;
            --control-size: 320px;
            --rotation-size: 260px;
            --offset: 24px;
            --radius: 18px;
            --stop-scale: 1;
        }

        * {
            box-sizing: border-box;
            font-family: "Space Grotesk", "Fira Sans", system-ui, sans-serif;
        }

        body {
            margin: 0;
            min-height: 100vh;
            color: var(--text);
            background: radial-gradient(circle at 20% 20%, #1e293b, transparent 45%),
                        radial-gradient(circle at 80% 0%, #0ea5e9, transparent 40%),
                        linear-gradient(160deg, var(--bg-1), var(--bg-2));
            display: flex;
            justify-content: space-between;
            align-items: flex-end;
            overflow: hidden;
        }

        body::after {
            content: "";
            position: fixed;
            inset: 0;
            background-image: radial-gradient(rgba(148, 163, 184, 0.08) 1px, transparent 0);
            background-size: 28px 28px;
            opacity: 0.35;
            pointer-events: none;
        }

        .control-container {
            position: fixed;
            bottom: var(--offset);
            transition: transform 0.4s ease, opacity 0.4s ease;
        }

        #movement-circle {
            left: var(--offset);
            width: var(--control-size);
            height: var(--control-size);
            background: radial-gradient(circle, rgba(56, 189, 248, 0.35), transparent 60%);
            border: 2px solid rgba(56, 189, 248, 0.4);
            border-radius: 50%;
            box-shadow: 0 0 40px rgba(56, 189, 248, 0.2);
        }

        #rotation-doughnut {
            right: var(--offset);
            width: var(--rotation-size);
            height: var(--rotation-size);
            background: radial-gradient(circle, transparent 45%, rgba(245, 158, 11, 0.3) 46%);
            border-radius: 50%;
            border: 2px solid rgba(245, 158, 11, 0.45);
            box-shadow: 0 0 35px rgba(245, 158, 11, 0.2);
        }

        body[data-layout="swap"] #movement-circle {
            left: auto;
            right: var(--offset);
        }

        body[data-layout="swap"] #rotation-doughnut {
            right: auto;
            left: var(--offset);
        }

        #stick {
            width: 26px;
            height: 26px;
            background: #f8fafc;
            border-radius: 50%;
            position: absolute;
            transform: translate(-50%, -50%);
            box-shadow: 0 10px 25px rgba(15, 23, 42, 0.35);
        }

        #stop-button {
            position: fixed;
            top: 24px;
            left: 24px;
            width: 120px;
            height: 52px;
            background: var(--danger);
            color: white;
            font-size: 18px;
            font-weight: 700;
            border: none;
            border-radius: 999px;
            cursor: pointer;
            box-shadow: 0 10px 25px rgba(239, 68, 68, 0.35);
            transform: scale(var(--stop-scale));
            transform-origin: top left;
            transition: transform 0.2s ease, box-shadow 0.2s ease;
            z-index: 3;
        }

        #stop-button:hover {
            background: var(--danger-dark);
            transform: translateY(-2px) scale(var(--stop-scale));
            box-shadow: 0 12px 30px rgba(239, 68, 68, 0.4);
        }

        #control-panel {
            position: fixed;
            top: 24px;
            right: 24px;
            width: min(320px, 90vw);
            background: var(--panel);
            border: 1px solid rgba(148, 163, 184, 0.2);
            border-radius: var(--radius);
            padding: 16px 18px;
            backdrop-filter: blur(14px);
            box-shadow: 0 20px 40px rgba(15, 23, 42, 0.4);
            z-index: 3;
        }

        #control-panel h2 {
            margin: 0 0 8px;
            font-size: 18px;
            letter-spacing: 0.3px;
        }

        .panel-row {
            display: flex;
            align-items: center;
            justify-content: space-between;
            gap: 12px;
            margin: 10px 0;
            font-size: 14px;
            color: var(--muted);
        }

        select,
        input[type="range"] {
            width: 150px;
            accent-color: var(--accent-1);
        }

        #panel-toggle {
            margin-top: 10px;
            width: 100%;
            border: none;
            border-radius: 999px;
            padding: 8px 12px;
            background: rgba(56, 189, 248, 0.2);
            color: var(--text);
            cursor: pointer;
            transition: background 0.2s ease;
        }

        #panel-toggle:hover {
            background: rgba(56, 189, 248, 0.35);
        }

        .hidden {
            opacity: 0;
            transform: translateY(-10px);
            pointer-events: none;
        }

        @media (max-width: 700px) {
            #movement-circle {
                width: calc(var(--control-size) * 0.8);
                height: calc(var(--control-size) * 0.8);
            }

            #rotation-doughnut {
                width: calc(var(--rotation-size) * 0.8);
                height: calc(var(--rotation-size) * 0.8);
            }

            #control-panel {
                left: 50%;
                right: auto;
                transform: translateX(-50%);
                top: auto;
                bottom: calc(var(--control-size) * 0.8 + 32px);
            }
        }
    </style>
</head>
<body>

    <button id="stop-button">STOP</button>

    <div id="control-panel" class="panel">
        <h2>Console</h2>
        <div class="panel-row">
            <label for="theme-select">Theme</label>
            <select id="theme-select">
                <option value="nebula">Nebula</option>
                <option value="ember">Ember</option>
                <option value="tide">Tide</option>
                <option value="dune">Dune</option>
            </select>
        </div>
        <div class="panel-row">
            <label for="size-slider">Control Size</label>
            <input id="size-slider" type="range" min="240" max="420" value="320">
        </div>
        <div class="panel-row">
            <label for="rotation-slider">Rotation Size</label>
            <input id="rotation-slider" type="range" min="200" max="360" value="260">
        </div>
        <div class="panel-row">
            <label for="offset-slider">Spread</label>
            <input id="offset-slider" type="range" min="12" max="80" value="24">
        </div>
        <div class="panel-row">
            <label for="deadzone-slider">Deadzone %</label>
            <input id="deadzone-slider" type="range" min="0" max="40" value="0">
        </div>
        <div class="panel-row">
            <label for="stop-slider">Stop Size</label>
            <input id="stop-slider" type="range" min="80" max="140" value="100">
        </div>
        <div class="panel-row">
            <label for="swap-toggle">Swap Layout</label>
            <input id="swap-toggle" type="checkbox">
        </div>
        <button id="panel-toggle">Hide panel</button>
    </div>

    <div id="movement-circle" class="control-container">
        <div id="stick"></div>
    </div>

    <div id="rotation-doughnut" class="control-container"></div>

    <script>
        const themes = {
            nebula: {
                bg1: "#0f172a",
                bg2: "#0b1220",
                panel: "rgba(15, 23, 42, 0.75)",
                text: "#e2e8f0",
                muted: "#94a3b8",
                accent1: "#38bdf8",
                accent2: "#f59e0b",
                ring: "rgba(56, 189, 248, 0.25)",
            },
            ember: {
                bg1: "#220901",
                bg2: "#3b0b02",
                panel: "rgba(59, 11, 2, 0.8)",
                text: "#fde68a",
                muted: "#fca5a5",
                accent1: "#fb7185",
                accent2: "#f97316",
                ring: "rgba(251, 113, 133, 0.25)",
            },
            tide: {
                bg1: "#0f172a",
                bg2: "#082f49",
                panel: "rgba(8, 47, 73, 0.82)",
                text: "#e0f2fe",
                muted: "#7dd3fc",
                accent1: "#22d3ee",
                accent2: "#38bdf8",
                ring: "rgba(34, 211, 238, 0.25)",
            },
            dune: {
                bg1: "#2a1f14",
                bg2: "#3f2a1d",
                panel: "rgba(63, 42, 29, 0.8)",
                text: "#fef3c7",
                muted: "#fcd34d",
                accent1: "#f59e0b",
                accent2: "#fcd34d",
                ring: "rgba(245, 158, 11, 0.25)",
            }
        };

        const root = document.documentElement;
        const themeSelect = document.getElementById("theme-select");
        const sizeSlider = document.getElementById("size-slider");
        const rotationSlider = document.getElementById("rotation-slider");
        const offsetSlider = document.getElementById("offset-slider");
        const deadzoneSlider = document.getElementById("deadzone-slider");
        const stopSlider = document.getElementById("stop-slider");
        const swapToggle = document.getElementById("swap-toggle");
        const panel = document.getElementById("control-panel");
        const panelToggle = document.getElementById("panel-toggle");

        const saved = JSON.parse(localStorage.getItem("ui-settings") || "{}");
        if (saved.theme && themes[saved.theme]) themeSelect.value = saved.theme;
        if (saved.controlSize) sizeSlider.value = saved.controlSize;
        if (saved.rotationSize) rotationSlider.value = saved.rotationSize;
        if (saved.offset) offsetSlider.value = saved.offset;
        if (saved.deadzone) deadzoneSlider.value = saved.deadzone;
        if (saved.stopScale) stopSlider.value = saved.stopScale;
        if (saved.swapLayout) swapToggle.checked = saved.swapLayout;
        if (saved.panelHidden) panel.classList.add("hidden");

        function applyTheme(name) {
            const theme = themes[name];
            root.style.setProperty("--bg-1", theme.bg1);
            root.style.setProperty("--bg-2", theme.bg2);
            root.style.setProperty("--panel", theme.panel);
            root.style.setProperty("--text", theme.text);
            root.style.setProperty("--muted", theme.muted);
            root.style.setProperty("--accent-1", theme.accent1);
            root.style.setProperty("--accent-2", theme.accent2);
            root.style.setProperty("--ring", theme.ring);
        }

        function applyLayout() {
            root.style.setProperty("--control-size", `${sizeSlider.value}px`);
            root.style.setProperty("--rotation-size", `${rotationSlider.value}px`);
            root.style.setProperty("--offset", `${offsetSlider.value}px`);
            root.style.setProperty("--stop-scale", `${stopSlider.value / 100}`);
            document.body.dataset.layout = swapToggle.checked ? "swap" : "default";
        }

        function saveSettings() {
            localStorage.setItem("ui-settings", JSON.stringify({
                theme: themeSelect.value,
                controlSize: sizeSlider.value,
                rotationSize: rotationSlider.value,
                offset: offsetSlider.value,
                deadzone: deadzoneSlider.value,
                stopScale: stopSlider.value,
                swapLayout: swapToggle.checked,
                panelHidden: panel.classList.contains("hidden")
            }));
        }

        themeSelect.addEventListener("change", () => {
            applyTheme(themeSelect.value);
            saveSettings();
        });

        [sizeSlider, rotationSlider, offsetSlider, deadzoneSlider, stopSlider].forEach((slider) => {
            slider.addEventListener("input", () => {
                applyLayout();
                saveSettings();
            });
        });

        swapToggle.addEventListener("change", () => {
            applyLayout();
            saveSettings();
        });

        panelToggle.addEventListener("click", () => {
            panel.classList.toggle("hidden");
            panelToggle.textContent = panel.classList.contains("hidden") ? "Show panel" : "Hide panel";
            saveSettings();
        });

        applyTheme(themeSelect.value);
        applyLayout();

        let quaternion = { w: 1, x: 0, y: 0, z: 0 };
        let lastSendTime = 0;
        const sendInterval = 40;
        let requestId = null;

        function sendQuaternion() {
            fetch('/api/motors', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(quaternion)
            });
        }

        function centerStick() {
            const stick = document.getElementById('stick');
            const rect = document.getElementById('movement-circle').getBoundingClientRect();
            const cx = rect.width / 2, cy = rect.height / 2;
            stick.style.left = `${cx}px`;
            stick.style.top = `${cy}px`;
        }

        function updatePosition(event) {
            const rect = document.getElementById('movement-circle').getBoundingClientRect();
            const cx = rect.width / 2, cy = rect.height / 2;
            const x = (event.clientX || event.touches[0].clientX) - rect.left - cx;
            const z = (event.clientY || event.touches[0].clientY) - rect.top - cy;
            const distance = Math.sqrt(x * x + z * z);

            if (distance <= cx) {
                const deadzoneRadius = (parseFloat(deadzoneSlider.value) / 100) * cx;
                if (distance < deadzoneRadius) {
                    quaternion.x = 0;
                    quaternion.z = 0;
                    centerStick();
                    return;
                }

                const newX = x / cx;
                const newZ = -z / cy;
                const logScale = (value) => Math.sign(value) * Math.log1p(Math.abs(value));

                quaternion.x = logScale(newX);
                quaternion.z = logScale(newZ);

                const stick = document.getElementById('stick');
                stick.style.left = `${cx + x}px`;
                stick.style.top = `${cy + z}px`;
            }
        }

        function stopRobot() {
            if (requestId) {
                cancelAnimationFrame(requestId);
            }
            quaternion = { w: 1, x: 0, y: 0, z: 0 };
            sendQuaternion();
            centerStick();
        }

        function loop() {
            const now = performance.now();
            if (now - lastSendTime >= sendInterval) {
                lastSendTime = now;
                sendQuaternion();
            }
            requestId = requestAnimationFrame(loop);
        }

        document.getElementById("movement-circle").addEventListener("mousemove", updatePosition);
        document.getElementById("movement-circle").addEventListener("touchmove", updatePosition);
        document.getElementById("stop-button").addEventListener("click", stopRobot);

        requestId = requestAnimationFrame(loop);
    </script>

</body>
</html>
)HTML";
