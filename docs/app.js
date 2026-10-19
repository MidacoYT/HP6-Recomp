(function () {
  "use strict";

  var STATUS_FROM_CODE = { i: "integrated", d: "deferred", b: "blocked", u: "untouched" };
  var BUILD_FROM_CODE = { p: "passed", c: "compiled_only", n: "not_run", f: "failed" };
  var STATUS_LABELS = {
    integrated: "Integree",
    deferred: "Differee",
    blocked: "Bloquee",
    untouched: "Non etudiee"
  };
  var BUILD_LABELS = { passed: "OK", compiled_only: "Compile", not_run: "Non lance", failed: "Echec" };
  var STATUS_FILL = { integrated: "#22c55e", deferred: "#f59e0b", blocked: "#ef4444", untouched: "#30343d" };
  var PAGE_SIZE = 150;

  function applyStoredTheme() {
    try {
      var stored = localStorage.getItem("hp6-recomp-theme");
      if (stored === "light" || stored === "dark") {
        document.documentElement.setAttribute("data-theme", stored);
      }
    } catch (e) { /* storage unavailable (private mode, etc.) */ }
  }

  function wireThemeToggle() {
    var btn = document.getElementById("theme-toggle");
    if (!btn) return;
    btn.addEventListener("click", function () {
      var current = document.documentElement.getAttribute("data-theme");
      var prefersDark = window.matchMedia && window.matchMedia("(prefers-color-scheme: dark)").matches;
      var effective = current || (prefersDark ? "dark" : "light");
      var next = effective === "dark" ? "light" : "dark";
      document.documentElement.setAttribute("data-theme", next);
      try { localStorage.setItem("hp6-recomp-theme", next); } catch (e) { /* ignore */ }
    });
  }

  function formatNumber(value) {
    return value.toLocaleString("fr-FR");
  }

  function renderOverall(global, reviewed) {
    document.getElementById("overall-percent").textContent = global.percent.toFixed(1);
    document.getElementById("overall-count").textContent =
      formatNumber(global.integrated) + " / " + formatNumber(global.total) + " fonctions (" + global.source + ")";

    document.getElementById("reviewed-percent").textContent = reviewed.percent.toFixed(1);
    document.getElementById("reviewed-count").textContent =
      formatNumber(reviewed.integrated) + " / " + formatNumber(reviewed.total) + " fonctions deja regardees";

    var bar = document.getElementById("overall-bar");
    var segments = [
      ["integrated", "bar-integrated"],
      ["deferred", "bar-deferred"],
      ["blocked", "bar-blocked"],
      ["untouched", "bar-untouched"]
    ];
    bar.innerHTML = segments.map(function (pair) {
      var count = global[pair[0]] || 0;
      var pct = global.total ? (100 * count / global.total) : 0;
      return '<span class="' + pair[1] + '" style="width:' + pct + '%"></span>';
    }).join("");

    var legend = document.getElementById("overall-legend");
    legend.innerHTML = [
      ["var(--ok)", "Integrees", global.integrated],
      ["var(--warn)", "Differees", global.deferred],
      ["var(--bad)", "Bloquees", global.blocked],
      ["var(--muted-dot)", "Non etudiees", global.untouched]
    ].map(function (item) {
      return '<span><span class="dot" style="background:' + item[0] + '"></span>' +
        item[1] + " : " + formatNumber(item[2]) + "</span>";
    }).join("");
  }

  function renderModules(modules) {
    var grid = document.getElementById("module-grid");
    grid.innerHTML = modules.map(function (m) {
      return (
        '<div class="module-card">' +
          '<div class="module-name">' + escapeHtml(m.name) + '</div>' +
          '<div class="module-bar"><span style="width:' + m.percent + '%"></span></div>' +
          '<div class="module-stats"><span>' + m.integrated + ' / ' + m.total + '</span>' +
          '<span>' + m.percent.toFixed(0) + '%</span></div>' +
        '</div>'
      );
    }).join("");
  }

  function escapeHtml(value) {
    return String(value)
      .replace(/&/g, "&amp;")
      .replace(/</g, "&lt;")
      .replace(/>/g, "&gt;");
  }

  function statusBadge(status) {
    var label = STATUS_LABELS[status] || status;
    return '<span class="badge badge-' + status + '">' + label + '</span>';
  }

  function buildBadge(build) {
    var label = BUILD_LABELS[build] || build;
    return '<span class="badge badge-' + build + '">' + label + '</span>';
  }

  function decodeRow(row) {
    return {
      address: row.a,
      name: row.n || "",
      auto: !!row.auto,
      status: STATUS_FROM_CODE[row.s] || "untouched",
      module: row.m || "",
      build: BUILD_FROM_CODE[row.b] || "not_run",
      note: row.note || "",
      source: row.src || "",
      size: row.sz || 1
    };
  }

  function matchesFilters(row, state) {
    if (state.status !== "all" && row.status !== state.status) return false;
    if (!state.query) return true;
    return (
      row.address.indexOf(state.query) !== -1 ||
      row.name.toLowerCase().indexOf(state.query) !== -1 ||
      row.source.toLowerCase().indexOf(state.query) !== -1 ||
      row.module.toLowerCase().indexOf(state.query) !== -1
    );
  }

  function renderRows(rows) {
    var body = document.getElementById("function-rows");
    body.innerHTML = "";

    if (rows.length === 0) {
      document.getElementById("empty-state").hidden = false;
      return;
    }
    document.getElementById("empty-state").hidden = true;

    var html = rows.map(function (row) {
      var title = row.note ? ' title="' + escapeHtml(row.note).replace(/"/g, "&quot;") + '"' : "";
      var nameClass = row.auto ? "cell-symbol auto-name" : "cell-symbol";
      var nameText = row.name ? escapeHtml(row.name) : '<span class="muted">—</span>';
      return (
        "<tr" + title + ' data-address="' + row.address + '">' +
          '<td class="cell-address" data-label="Adresse">0x' + row.address + '</td>' +
          '<td class="' + nameClass + '" data-label="Nom">' + nameText + '</td>' +
          '<td class="cell-module" data-label="Module">' + (row.module ? escapeHtml(row.module) : '<span class="muted">—</span>') + '</td>' +
          '<td data-label="Statut">' + statusBadge(row.status) + '</td>' +
          '<td data-label="Build">' + buildBadge(row.build) + '</td>' +
        "</tr>"
      );
    }).join("");
    body.insertAdjacentHTML("beforeend", html);
  }

  // ---- Squarified treemap (Bruls/Huizing/van Wijk) ---------------------
  // `values` must be positive numbers; returns parallel array of {x,y,w,h}
  // in the same units as x/y/w/h passed in (CSS pixels here).
  function worstRatio(row, shortSide) {
    var sum = 0, max = -Infinity, min = Infinity;
    for (var i = 0; i < row.length; i++) {
      var v = row[i];
      sum += v;
      if (v > max) max = v;
      if (v < min) min = v;
    }
    var s2 = shortSide * shortSide;
    var sum2 = sum * sum;
    return Math.max((s2 * max) / sum2, sum2 / (s2 * min));
  }

  function squarify(values, x, y, w, h) {
    var n = values.length;
    var rects = new Array(n);
    var i = 0;
    var row = [];
    while (i < n) {
      row.length = 0;
      row.push(values[i]);
      var shortSide = Math.min(w, h);
      var worst = worstRatio(row, shortSide);
      var j = i + 1;
      while (j < n) {
        row.push(values[j]);
        var testWorst = worstRatio(row, shortSide);
        if (testWorst <= worst) {
          worst = testWorst;
          j++;
        } else {
          row.pop();
          break;
        }
      }
      var rowSum = 0;
      for (var k = 0; k < row.length; k++) rowSum += row[k];

      if (w >= h) {
        var stripW = rowSum / h;
        var cy = y;
        for (var k2 = 0; k2 < row.length; k2++) {
          var itemH = row[k2] / stripW;
          rects[i + k2] = { x: x, y: cy, w: stripW, h: itemH };
          cy += itemH;
        }
        x += stripW;
        w -= stripW;
      } else {
        var stripH = rowSum / w;
        var cx = x;
        for (var k3 = 0; k3 < row.length; k3++) {
          var itemW = row[k3] / stripH;
          rects[i + k3] = { x: cx, y: y, w: itemW, h: stripH };
          cx += itemW;
        }
        y += stripH;
        h -= stripH;
      }
      i = j;
    }
    return rects;
  }

  function setupTreemap(allRows, getFilterState) {
    var canvas = document.getElementById("treemap-canvas");
    var container = document.getElementById("treemap-wrap");
    var tooltip = document.getElementById("treemap-tooltip");
    if (!canvas || !container) return function () {};

    var ctx = canvas.getContext("2d");
    var pointerFine = window.matchMedia && window.matchMedia("(pointer: fine)").matches;

    // Biggest functions first: conventional squarify input order, and it
    // also means the first rows placed are the large, easy-to-hit blocks.
    var items = allRows.slice().sort(function (a, b) { return b.size - a.size; });
    var weights = items.map(function (it) { return Math.sqrt(Math.max(it.size, 1)); });

    var layout = [];
    var cssWidth = 0, cssHeight = 0;

    function computeHeight(width) {
      return Math.max(220, Math.min(560, Math.round(width * 0.6)));
    }

    function layoutNow() {
      cssWidth = Math.max(1, Math.round(container.clientWidth));
      cssHeight = computeHeight(cssWidth);

      var dpr = window.devicePixelRatio || 1;
      canvas.width = Math.round(cssWidth * dpr);
      canvas.height = Math.round(cssHeight * dpr);
      canvas.style.width = cssWidth + "px";
      canvas.style.height = cssHeight + "px";
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0);

      var totalArea = cssWidth * cssHeight;
      var totalWeight = weights.reduce(function (a, b) { return a + b; }, 0) || 1;
      var scaled = weights.map(function (v) { return (v / totalWeight) * totalArea; });
      var rects = squarify(scaled, 0, 0, cssWidth, cssHeight);
      layout = rects.map(function (r, idx) { return { r: r, item: items[idx] }; });
    }

    function draw() {
      var state = getFilterState();
      var dimming = !!(state.query || state.status !== "all");

      ctx.clearRect(0, 0, cssWidth, cssHeight);
      for (var i = 0; i < layout.length; i++) {
        var entry = layout[i];
        var r = entry.r;
        var match = !dimming || matchesFilters(entry.item, state);
        ctx.globalAlpha = match ? 1 : 0.1;
        ctx.fillStyle = STATUS_FILL[entry.item.status] || STATUS_FILL.untouched;
        var rw = Math.max(0, r.w - 1);
        var rh = Math.max(0, r.h - 1);
        ctx.fillRect(r.x + 0.5, r.y + 0.5, rw, rh);
      }
      ctx.globalAlpha = 1;
    }

    function findAt(mx, my) {
      for (var i = 0; i < layout.length; i++) {
        var r = layout[i].r;
        if (mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h) {
          return layout[i].item;
        }
      }
      return null;
    }

    function eventToCanvasPoint(evt) {
      var rect = canvas.getBoundingClientRect();
      var clientX = evt.touches ? evt.touches[0].clientX : evt.clientX;
      var clientY = evt.touches ? evt.touches[0].clientY : evt.clientY;
      return { x: clientX - rect.left, y: clientY - rect.top, clientX: clientX, clientY: clientY };
    }

    function showTooltip(item, clientX, clientY) {
      if (!tooltip) return;
      tooltip.innerHTML =
        '<strong>0x' + item.address + '</strong><br>' +
        (item.name ? escapeHtml(item.name) : '<span class="muted">sans nom</span>') + '<br>' +
        '<span class="badge badge-' + item.status + '">' + (STATUS_LABELS[item.status] || item.status) + '</span>';
      tooltip.style.left = clientX + 14 + "px";
      tooltip.style.top = clientY + 14 + "px";
      tooltip.hidden = false;
    }

    function hideTooltip() {
      if (tooltip) tooltip.hidden = true;
    }

    var lastHoverAt = 0;
    canvas.addEventListener("mousemove", function (evt) {
      if (!pointerFine) return;
      var now = Date.now();
      if (now - lastHoverAt < 40) return;
      lastHoverAt = now;
      var pt = eventToCanvasPoint(evt);
      var item = findAt(pt.x, pt.y);
      if (item) {
        canvas.style.cursor = "pointer";
        showTooltip(item, pt.clientX, pt.clientY);
      } else {
        canvas.style.cursor = "default";
        hideTooltip();
      }
    });
    canvas.addEventListener("mouseleave", hideTooltip);

    var onPick = null;
    canvas.addEventListener("click", function (evt) {
      var pt = eventToCanvasPoint(evt);
      var item = findAt(pt.x, pt.y);
      if (item && onPick) onPick(item);
    });

    var resizeTimer = null;
    window.addEventListener("resize", function () {
      window.clearTimeout(resizeTimer);
      resizeTimer = window.setTimeout(function () {
        layoutNow();
        draw();
      }, 150);
    });

    layoutNow();
    draw();

    return {
      redraw: draw,
      onPick: function (fn) { onPick = fn; }
    };
  }

  function setup(data) {
    var global = data.global || data.totals;
    var reviewed = data.reviewed || data.totals;
    renderOverall(global, reviewed);
    renderModules(data.modules);

    var allRows = data.functions.map(decodeRow);

    document.getElementById("function-count").textContent =
      "(" + formatNumber(allRows.length) + ")";
    document.getElementById("generated-at").textContent =
      new Date(data.generated_at).toLocaleString("fr-FR", { dateStyle: "medium", timeStyle: "short" });

    var state = { query: "", status: "all", visible: PAGE_SIZE, filtered: allRows };

    function computeFiltered() {
      return allRows.filter(function (row) { return matchesFilters(row, state); });
    }

    function renderPage(reset) {
      if (reset) state.visible = PAGE_SIZE;
      var slice = state.filtered.slice(0, state.visible);
      renderRows(slice);

      var loadMore = document.getElementById("load-more");
      var remaining = state.filtered.length - slice.length;
      loadMore.hidden = remaining <= 0;
      document.getElementById("shown-count").textContent =
        formatNumber(slice.length) + " / " + formatNumber(state.filtered.length) + " affichees";
    }

    var treemap = setupTreemap(allRows, function () { return state; });

    function applyFilters() {
      state.filtered = computeFiltered();
      renderPage(true);
      treemap.redraw();
    }

    var searchInput = document.getElementById("search");
    var searchTimer = null;
    searchInput.addEventListener("input", function (e) {
      var value = e.target.value;
      window.clearTimeout(searchTimer);
      searchTimer = window.setTimeout(function () {
        state.query = value.trim().toLowerCase();
        applyFilters();
      }, 120);
    });

    var filterButtons = document.querySelectorAll("#status-filters button");
    filterButtons.forEach(function (btn) {
      btn.addEventListener("click", function () {
        filterButtons.forEach(function (b) { b.classList.remove("active"); });
        btn.classList.add("active");
        state.status = btn.getAttribute("data-status");
        applyFilters();
      });
    });

    document.getElementById("load-more").addEventListener("click", function () {
      state.visible += PAGE_SIZE;
      renderPage(false);
    });

    treemap.onPick(function (item) {
      searchInput.value = item.address;
      state.query = item.address;
      filterButtons.forEach(function (b) { b.classList.remove("active"); });
      document.querySelector('#status-filters button[data-status="all"]').classList.add("active");
      state.status = "all";
      applyFilters();

      document.getElementById("function-table").scrollIntoView({ behavior: "smooth", block: "center" });
      window.setTimeout(function () {
        var row = document.querySelector('#function-rows tr[data-address="' + item.address + '"]');
        if (row) {
          row.classList.add("row-flash");
          window.setTimeout(function () { row.classList.remove("row-flash"); }, 1600);
        }
      }, 300);
    });

    applyFilters();
  }

  applyStoredTheme();
  wireThemeToggle();

  fetch("data/progress.json")
    .then(function (res) { return res.json(); })
    .then(setup)
    .catch(function (err) {
      document.getElementById("overall-percent").textContent = "err";
      console.error("Impossible de charger data/progress.json", err);
    });
})();
