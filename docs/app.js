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
      source: row.src || ""
    };
  }

  function renderRows(rows, append) {
    var body = document.getElementById("function-rows");
    if (!append) body.innerHTML = "";

    if (rows.length === 0 && !append && body.children.length === 0) {
      document.getElementById("empty-state").hidden = false;
      return;
    }
    document.getElementById("empty-state").hidden = true;

    var html = rows.map(function (row) {
      var title = row.note ? ' title="' + escapeHtml(row.note).replace(/"/g, "&quot;") + '"' : "";
      var nameClass = row.auto ? "cell-symbol auto-name" : "cell-symbol";
      var nameText = row.name ? escapeHtml(row.name) : '<span class="muted">—</span>';
      return (
        "<tr" + title + ">" +
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
      var query = state.query.trim().toLowerCase();
      return allRows.filter(function (row) {
        if (state.status !== "all" && row.status !== state.status) return false;
        if (!query) return true;
        return (
          row.address.indexOf(query) !== -1 ||
          row.name.toLowerCase().indexOf(query) !== -1 ||
          row.source.toLowerCase().indexOf(query) !== -1 ||
          row.module.toLowerCase().indexOf(query) !== -1
        );
      });
    }

    function renderPage(reset) {
      if (reset) state.visible = PAGE_SIZE;
      var slice = state.filtered.slice(0, state.visible);
      renderRows(slice, false);

      var loadMore = document.getElementById("load-more");
      var remaining = state.filtered.length - slice.length;
      loadMore.hidden = remaining <= 0;
      document.getElementById("shown-count").textContent =
        formatNumber(slice.length) + " / " + formatNumber(state.filtered.length) + " affichees";
    }

    function applyFilters() {
      state.filtered = computeFiltered();
      renderPage(true);
    }

    var searchTimer = null;
    document.getElementById("search").addEventListener("input", function (e) {
      var value = e.target.value;
      window.clearTimeout(searchTimer);
      searchTimer = window.setTimeout(function () {
        state.query = value;
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
