(function () {
  "use strict";

  var STATUS_LABELS = { integrated: "Integree", deferred: "Differee", blocked: "Bloquee", pending: "En attente" };
  var BUILD_LABELS = { passed: "OK", compiled_only: "Compile", not_run: "Non lance", failed: "Echec" };

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

  function renderOverall(totals) {
    document.getElementById("overall-percent").textContent = totals.percent.toFixed(1);

    var bar = document.getElementById("overall-bar");
    var segments = [
      ["integrated", "bar-integrated"],
      ["deferred", "bar-deferred"],
      ["blocked", "bar-blocked"]
    ];
    bar.innerHTML = segments.map(function (pair) {
      var count = totals[pair[0]] || 0;
      var pct = totals.total ? (100 * count / totals.total) : 0;
      return '<span class="' + pair[1] + '" style="width:' + pct + '%"></span>';
    }).join("");

    var legend = document.getElementById("overall-legend");
    legend.innerHTML = [
      ["var(--ok)", "Integrees", totals.integrated],
      ["var(--warn)", "Differees", totals.deferred],
      ["var(--bad)", "Bloquees", totals.blocked]
    ].map(function (item) {
      return '<span><span class="dot" style="background:' + item[0] + '"></span>' +
        item[1] + " : " + item[2] + " / " + totals.total + "</span>";
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

  function renderRows(rows) {
    var body = document.getElementById("function-rows");
    if (rows.length === 0) {
      body.innerHTML = "";
      document.getElementById("empty-state").hidden = false;
      return;
    }
    document.getElementById("empty-state").hidden = true;

    var html = rows.map(function (row) {
      var title = row.note ? ' title="' + escapeHtml(row.note).replace(/"/g, "&quot;") + '"' : "";
      return (
        "<tr" + title + ">" +
          '<td class="cell-address">0x' + row.address + '</td>' +
          '<td class="cell-symbol">' + (row.symbol ? escapeHtml(row.symbol) : '<span class="muted">—</span>') + '</td>' +
          '<td class="cell-module">' + escapeHtml(row.module) + '</td>' +
          '<td>' + statusBadge(row.status) + '</td>' +
          '<td>' + buildBadge(row.build) + '</td>' +
        "</tr>"
      );
    }).join("");
    body.innerHTML = html;
  }

  function setup(data) {
    renderOverall(data.totals);
    renderModules(data.modules);

    document.getElementById("function-count").textContent =
      "(" + data.totals.total + ")";
    document.getElementById("generated-at").textContent =
      new Date(data.generated_at).toLocaleString("fr-FR", { dateStyle: "medium", timeStyle: "short" });

    var state = { query: "", status: "all" };

    function applyFilters() {
      var query = state.query.trim().toLowerCase();
      var filtered = data.functions.filter(function (row) {
        if (state.status !== "all" && row.status !== state.status) return false;
        if (!query) return true;
        return (
          row.address.indexOf(query) !== -1 ||
          row.symbol.toLowerCase().indexOf(query) !== -1 ||
          row.source.toLowerCase().indexOf(query) !== -1 ||
          row.module.toLowerCase().indexOf(query) !== -1
        );
      });
      renderRows(filtered);
    }

    document.getElementById("search").addEventListener("input", function (e) {
      state.query = e.target.value;
      applyFilters();
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
